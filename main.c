/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#include <err.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

#include "tldr.h"

#define SUPPORT_URL "https://github.com/kovmir/tinytldr/issues"
#ifndef GIT_VERSION
#define GIT_VERSION "dev"
#endif /* GIT_VERSION */

/* Parse command line options - and --. */
void parse_cli_opts(int argc, char *argv[]);
/* Print usage manual. */
void print_help(FILE *out);
/* Download and extract newest pages. */
void run_update(Config *cfg);
/* Print the requested page to the terminal. */
void run_display(Config *cfg, const char *name, const char *platform);

#include "config.h"

static int list_flag = 0;
static char *page_platform = NULL;
static int target_flag = 0;
static int update_flag = 0;

void
parse_cli_opts(int argc, char *argv[])
{
	int opt;
	static struct option long_options[] = {
		{"help",     no_argument,       0, 'h'},
		{"list",     no_argument,       0, 'l'},
		{"platform", required_argument, 0, 'p'},
		{"target",   no_argument,       0, 't'},
		{"update",   no_argument,       0, 'u'},
		{"version",  no_argument,       0, 'v'},
		{0, 0, 0, 0} /* Must be last. */
	};

	while ((opt = getopt_long(argc, argv, "hlp:tuv", long_options, NULL)) != -1) {
		switch (opt) {
		case 'h':
			print_help(stdout);
			exit(0);
		case 'l':
			list_flag = 1;
			break;
		case 'p':
			page_platform = optarg;
			break;
		case 't':
			target_flag = 1;
			break;
		case 'u':
			update_flag = 1;
			break;
		case 'v':
			puts(GIT_VERSION);
			exit(0);
		default:
			print_help(stderr);
			exit(-1);
		}
	}
}

void
print_help(FILE *out)
{
	fprintf(out, "usage: tldr [-h] [-l] [-p PLATFORM] [-t] [-u] [-v] PAGE...\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -h, --help        show this help message\n");
	fprintf(out, "  -l, --list        list all available pages\n");
	fprintf(out, "  -p, --platform    specify page platform (e.g. linux, osx, common)\n");
	fprintf(out, "  -t, --target      show page path instead of the page\n");
	fprintf(out, "  -u, --update      download tldr pages\n");
	fprintf(out, "  -v, --version     show version\n");
	fprintf(out, "\n");
	fprintf(out, "Arguments:\n");
	fprintf(out, "  PAGE              page name (e.g. tar, git commit)\n");
	fprintf(out, "\n");
	fprintf(out, "Examples:\n");
	fprintf(out, "  tldr tar\n");
	fprintf(out, "  tldr git commit\n");
	fprintf(out, "  tldr -p osx tar\n");
	fprintf(out, "  tldr -u\n");
	fprintf(out, "\n");
	fprintf(out, "Support: "SUPPORT_URL"\n");
}

void
run_update(Config *cfg)
{
	FILE *temp_file;

	temp_file = tmpfile();
	if (temp_file == NULL)
		err(1, "unable to open create a temporary file");

	/* Download. */
	if (fetch_pages(cfg, temp_file) == -1)
		errx(1, "unable to fetch pages");
	rewind(temp_file);
	/* Extract. */
	if (extract_pages(cfg, temp_file) == -1)
		errx(1, "unable to extract pages");

	fclose(temp_file);
}

void
run_display(Config *cfg, const char *name, const char *platform)
{
	char *page_path;
	FILE *page_file;

	/* Find page. */
	page_path = find_page(cfg, name, platform);
	if (page_path == NULL)
		errx(1, "not found");
	/* Open page. */
	page_file = fopen(page_path, "rb");
	if (page_file == NULL)
		err(1, "unable to open %s", page_path);
	/* Display page. */
	if (print_page(cfg, page_file) == -1)
		errx(1, "unable to display %s", page_path);

	fclose(page_file);
	free(page_path);
}

int
main(int argc, char *argv[])
{
	Config *cfg;
	char page_name[NAME_MAX] = {0};
	char expanded_home[PATH_MAX] = {0};
	wordexp_t w;
	int first;

	/* CLI options. */
	parse_cli_opts(argc, argv);
	argc -= optind;
	argv += optind;

	/* Expand ~ */
	if (wordexp(PAGES_HOME, &w, 0) != 0)
		errx(1, "shell unable to expand %s", PAGES_HOME);
	if (w.we_wordc < 1)
		errx(1, "invalid %s", PAGES_HOME);
	snprintf(expanded_home, PATH_MAX, "%s", w.we_wordv[0]);
	wordfree(&w);

	cfg = create_cfg(&(ConfigOpts){
		.pages_url     = PAGES_URL,
		.pages_home    = expanded_home,
		.user_agent    = "tinytldr/"GIT_VERSION,
		.heading_style = HEADING_STYLE,
		.summary_style = SUMMARY_STYLE,
		.comment_style = COMMENT_STYLE,
		.command_style = COMMAND_STYLE,
		.reset_style   = RESET_STYLE,
		.skip_empty    = SKIP_EMPTY,
		.apply_styles  = APPLY_STYLES,
		.out           = stdout,
	});
	if (cfg == NULL)
		err(1, "unable to allocate config");

	/* List pages. */
	if (list_flag == 1) {
		list_pages(cfg);
		return 0;
	}

	/* Update pages. */
	if (update_flag == 1) {
		run_update(cfg);
		return 0;
	}

	if (argc < 1) {
		/* No options and no arguments. */
		print_help(stderr);
		exit(-1);
	}

	if (access(expanded_home, F_OK) == -1) {
		/* Page requested, but there are no pages on disk. */
		errx(1, "no pages; try to --update");
	}

	/* Combine CLI args into a single page file name. */
	first = 1;
	while (argc-- > 0) {
		/* Buffer overflow is possible, but not by accident. */
		if (!first) {
			strcat(page_name, "-");
		}
		first = 0;
		strcat(page_name, *argv++);
	}
	strcat(page_name, ".md");

	/* Show page path only. */
	if (target_flag == 1) {
		char *path = find_page(cfg, page_name, page_platform);
		if (path == NULL)
			errx(1, "not found");
		puts(path);
		return 0;
	}

	/* Show page. */
	run_display(cfg, page_name, page_platform);

	return 0;
}
