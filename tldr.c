/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */

/* Includes */
#include <assert.h>
#include <dirent.h>
#include <err.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <fts.h>
#include <ftw.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>

#include "tldr.h"

/* Constants and Macros */
#define MAX_LINE_LEN 1024 /* Maximal tldr page line length. */
#define HEADING_TOKEN '#'
#define SUMMARY_TOKEN '>'
#define COMMENT_TOKEN '-'
#define COMMAND_TOKEN '`'

/* Typedefs */
struct Config {
	char *pages_url;
	char *user_agent;
	char *pages_home;
	char *heading_style;
	char *summary_style;
	char *comment_style;
	char *command_style;
	char *reset_style;
	int skip_empty;
	int apply_styles;
	FILE *out;
};

/* Function prototypes */
static int entcmp(const FTSENT **a, const FTSENT **b);

Config *
create_cfg(const ConfigOpts *opts)
{
	assert(opts != NULL);

	Config *cfg = malloc(sizeof(Config));
	if (cfg == NULL)
		return NULL;

	cfg->pages_url     = opts->pages_url     ? strdup(opts->pages_url)     : NULL;
	cfg->user_agent    = opts->user_agent    ? strdup(opts->user_agent)    : NULL;
	cfg->pages_home    = opts->pages_home    ? strdup(opts->pages_home)    : NULL;
	cfg->heading_style = opts->heading_style ? strdup(opts->heading_style) : NULL;
	cfg->summary_style = opts->summary_style ? strdup(opts->summary_style) : NULL;
	cfg->command_style = opts->command_style ? strdup(opts->command_style) : NULL;
	cfg->comment_style = opts->comment_style ? strdup(opts->comment_style) : NULL;
	cfg->reset_style   = opts->reset_style   ? strdup(opts->reset_style)   : NULL;

	cfg->skip_empty   = opts->skip_empty;
	cfg->apply_styles = opts->apply_styles;
	cfg->out          = opts->out;

	if ((cfg->pages_url     == NULL) ||
	    (cfg->pages_home    == NULL) ||
	    (cfg->heading_style == NULL) ||
	    (cfg->summary_style == NULL) ||
	    (cfg->command_style == NULL) ||
	    (cfg->comment_style == NULL) ||
	    (cfg->reset_style   == NULL)) {
		return NULL;
	}
	return cfg;
}

void
destroy_cfg(Config *cfg)
{
	if (cfg == NULL)
		return;
	free(cfg->pages_url);
	free(cfg->pages_home);
	free(cfg->heading_style);
	free(cfg->summary_style);
	free(cfg->command_style);
	free(cfg->comment_style);
	free(cfg->reset_style);
	free(cfg);
}

int
fetch_pages(const Config *cfg, FILE *dest)
{
	CURL    *curl_handle;
	CURLcode curl_res;                  /* Curl operation result. */
	char     curl_err[CURL_ERROR_SIZE]; /* Curl error message buffer. */

	assert(dest != NULL);
	assert(cfg != NULL);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, curl_err);
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 5L);
	curl_easy_setopt(curl_handle, CURLOPT_URL, cfg->pages_url);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, cfg->user_agent);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, dest);

	curl_res = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();

	if (curl_res != CURLE_OK)
		warnx("unable to fetch pages: %s", curl_err);
	return (curl_res == CURLE_OK) ? 0 : -1;
}

int
extract_pages(const Config *cfg, FILE *archive)
{
	struct archive *a = NULL;
	struct archive *ext = NULL;
	struct archive_entry *entry;
	int r;
	char *path;
	size_t len;
	const char *entry_path;

	assert(cfg != NULL);
	assert(archive != NULL);

	a = archive_read_new();
	if (a == NULL) {
		warnx("archive_read_new failed");
		return -1;
	}

	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);

	r = archive_read_open_FILE(a, archive);
	if (r != ARCHIVE_OK) {
		warnx("archive_read_open_FILE: %s", archive_error_string(a));
		goto out;
	}

	ext = archive_write_disk_new();
	if (ext == NULL) {
		warnx("archive_write_disk_new failed");
		r = ARCHIVE_FATAL;
		goto out;
	}

	archive_write_disk_set_options(ext, 0);

	while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
		entry_path = archive_entry_pathname(entry);
		if (entry_path == NULL)
			entry_path = "";

		/* +2 for / and \0 */
		len = strlen(cfg->pages_home) + strlen(entry_path) + 2;
		if ((path = malloc(len)) == NULL) {
			warn("malloc");
			r = ARCHIVE_FATAL;
			goto out;
		}
		snprintf(path, len, "%s/%s", cfg->pages_home, entry_path);

		archive_entry_set_pathname(entry, path);
		free(path);

		r = archive_read_extract2(a, entry, ext);
		if (r != ARCHIVE_OK) {
			warnx("archive_read_extract2: %s", archive_error_string(a));
			goto out;
		}
	}

	if (r != ARCHIVE_EOF)
		warnx("archive_read_next_header: %s", archive_error_string(a));

out:
	archive_write_free(ext);
	archive_read_close(a);
	archive_read_free(a);

	return (r == ARCHIVE_EOF) ? 0 : -1;
}

int
entcmp(const FTSENT **a, const FTSENT **b)
{
	return strcmp((*a)->fts_name, (*b)->fts_name);
}

char *
find_page(const Config *cfg, const char *name, const char *platform)
{
	char *path_argv[] = {cfg->pages_home, NULL};
	char *found = NULL;
	FTS *tree;
	FTSENT *f;

	assert(cfg != NULL);
	assert(name != NULL);

	tree = fts_open(path_argv, FTS_LOGICAL|FTS_NOSTAT, entcmp);
	if (tree == NULL) {
		warn("fts_open");
		return NULL;
	}

	while ((f = fts_read(tree))) {
		if (f->fts_info != FTS_F)
			continue; /* Not a file. */

		if (strcmp(f->fts_name, name) == 0) {
			/* Only name match. */
			if (platform == NULL) {
				found = f->fts_path;
				break;
			}
			/* Both name and platform match. */
			if (strcmp(f->fts_parent->fts_name, platform) == 0) {
				found = f->fts_path;
				break;
			}
		}
	}

	if (found != NULL)
		found = strdup(found);

	fts_close(tree);
	return found;
}

int
print_page(const Config *cfg, FILE *page)
{
	char line[MAX_LINE_LEN];
	char *n_ptr;
	char *style = NULL;
	int n;

	assert(cfg != NULL);
	assert(page != NULL);

	while (fgets(line, MAX_LINE_LEN, page) != NULL) {
		/* Skip empty lines if needed. */
		if (cfg->skip_empty && line[0] == '\n')
			continue;

		/* Discard end-of-line character. */
		n_ptr = strstr(line, "\n");
		if (n_ptr != NULL)
			*n_ptr = 0;

		/* Choose styling if needed. */
		if (cfg->apply_styles == 1) {
			switch (line[0]) {
			case HEADING_TOKEN:
				style = cfg->heading_style;
				break;
			case SUMMARY_TOKEN:
				style = cfg->summary_style;
				break;
			case COMMENT_TOKEN:
				style = cfg->comment_style;
				break;
			case COMMAND_TOKEN:
				style = cfg->command_style;
				break;
			default:
				style = NULL;
			}
		}

		/* Print page. */
		if (style != NULL) {
			n = fprintf(cfg->out, "%s%s%s\n", style, line, cfg->reset_style);
		} else {
			n = fprintf(cfg->out, "%s\n", line);
		}

		if (n < 0) {
			warn("unable to print page");
			return -1;
		}
	}
	return 0;
}

int
list_pages(const Config *cfg)
{
	const char *pattern = "*.md";
	char *path_argv[] = {cfg->pages_home, NULL};
	FTS *tree;
	FTSENT *f;

	assert(cfg != NULL);

	tree = fts_open(path_argv, FTS_LOGICAL|FTS_NOSTAT, entcmp);
	if (tree == NULL) {
		warn("fts_open");
		return -1;
	}

	while ((f = fts_read(tree))) {
		if (f->fts_info != FTS_F)
			continue; /* Not a file. */

		if (fnmatch(pattern, f->fts_name, FNM_PERIOD) == 0) {
			fprintf(cfg->out, "%s/%s\n",
				f->fts_parent->fts_name, f->fts_name);
		}
	}
	fts_close(tree);
	return 0;
}
