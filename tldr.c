/*
 * Copyright 2020 Ivan Kovmir
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Includes */
#include <curl/curl.h>

#include <archive.h>
#include <archive_entry.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Constants and Macros */
#define D_NAME entry->d_name
#define WRITE_MODE "w"
#define READ_MODE "r"
#define MAX_PATH 1024
#define MAX_INDEX_ENTRY 64
#define OPTION *(argv + 1)

#define BOOL int
#define TRUE 1
#define FALSE 0

#define EXPAND(s) #s
#define STRINGIZE(s) EXPAND(s)

/* Prototypes */
static void panic(const char *msg, const char *details);
static void cat_page(const char *page_path, const char *platform);

static void print_help(void);
static void list_pages(FILE *indexp);
static void fetch_pages(void);
static void extract_pages(void);
static void create_index(const char *pages_path, FILE *indexp);
static void display_page(char *pages_path, const char *page_name, FILE *indexp,
			 const char *platform);

/* Global variables */
static const char *RESET_STYLING = "\033[0m";

static const char *TMP_MASTER_ZIP = "/tmp/master.zip";
static const char *MASTER_ZIP_REMOTE_ADDR = "https://codeload.github.com"
					    "/tldr-pages/tldr/zip/master";

#include "config.h"

/*
 * Terminate the execution with an error message.
 * This function is used if something goes wrong.
 */
void
panic(const char *msg, const char *details)
{
	fprintf(stderr, "%s%s. Abort.\n", msg, details? details : "");
	exit(EXIT_FAILURE);
}

/* Supplies a page with colors/styling and prints it. */
void
cat_page(const char *page_path, const char *platform)
{
	char raw_page[MAX_PATH];
	char colored_page[MAX_PATH];
	char *chp;
	FILE *filep;
	BOOL do_we_skip_empty_line = FALSE;

	filep = fopen(page_path, READ_MODE);
	if (filep == NULL)
		panic("Failed to open the page", NULL);

	while (fgets(raw_page, MAX_PATH, filep) != NULL) {
		if (raw_page[0] == '#') {
			strcpy(colored_page, HEADING_STYLE);
			strcat(colored_page, platform);
			strcat(colored_page, "/");
			strcat(colored_page, &raw_page[2]);
			strcat(colored_page, RESET_STYLING);
			fprintf(stdout, "%s", colored_page);
			continue;
		}
		if (raw_page[0] == '>') {
			strcpy(colored_page, SUBHEADING_STYLE);
			strcat(colored_page, &raw_page[2]);
			strcat(colored_page, RESET_STYLING);
			fprintf(stdout, "%s", colored_page);
			continue;
		}
		if (raw_page[0] == '-') {
			do_we_skip_empty_line = TRUE;
			fprintf(stdout, "%s", raw_page);
			continue;
		}
		if (raw_page[0] == '`') {
			chp = strchr(&raw_page[1], '`');
			*chp = ' ';
			strcpy(colored_page, COMMAND_STYLE);
			strcat(colored_page, &raw_page[1]);
			strcat(colored_page, RESET_STYLING);
			fprintf(stdout, "%s", colored_page);
			continue;

		}
		if (do_we_skip_empty_line) {
			do_we_skip_empty_line = FALSE;
			continue;
		}
		printf(raw_page);
	}
}

/* Print instructions on how to use the program. */
void
print_help(void)
{
	printf("USAGE: tldr [options] <command>\n\n"
		"[options]\n"
		"  -l, --list:       show all available pages\n"
		"  -p, --platform    Display a platform specific page\n"
		"  -u, --update:     fetch lastest copies of cached pages\n"
		"  -h, --help:       this help overview\n\n"
		"<command>\n"
		"  Show examples for this command\n");
}

/* Print the list of all available pages. */
void
list_pages(FILE * indexp)
{
	char buf[MAX_INDEX_ENTRY];
	char *chp;

	while (fgets(buf, MAX_INDEX_ENTRY, indexp) != NULL) {
		chp = strchr(buf, '.');
		*chp = '\0';
		puts(buf);
	}
}

/*
 * Download the archive with pages from GitHub 
 * and put it in /tmp.
 */
void
fetch_pages(void)
{
	CURL *ce;
	CURLcode res;
	FILE *master_zip;

	master_zip = fopen(TMP_MASTER_ZIP, WRITE_MODE);
	if (master_zip == NULL)
		panic("Failed to create a temporary file", NULL);

	curl_global_init(CURL_GLOBAL_ALL);
	ce = curl_easy_init();
	curl_easy_setopt(ce, CURLOPT_WRITEDATA, master_zip);
	curl_easy_setopt(ce, CURLOPT_URL, MASTER_ZIP_REMOTE_ADDR);

	res = curl_easy_perform(ce);
	curl_easy_cleanup(ce);
	curl_global_cleanup();
	fclose(master_zip);
	if (res != CURLE_OK)
		panic("Failed to fetch pages; "
		      "cURL error code is ", STRINGIZE(res));
}

/*
 * Extract the downloaded archive with pages in /tmp
 * and put it into PAGES_PATH (defined in config.h).
 */
void
extract_pages(void)
{
	char page_path[MAX_PATH];
	char archive_path[MAX_PATH];

	FILE *master_zip;
	struct archive *ap;
	struct archive_entry *aep;
	int status;

	master_zip = fopen(TMP_MASTER_ZIP, READ_MODE);
	if (master_zip == NULL)
		panic("Failed to open the archive", NULL);

	ap = archive_read_new();
	if (ap == NULL)
		panic("Failed to allocate memory "
		      "for the archive reading", NULL);
	archive_read_support_format_zip(ap);
	status = archive_read_open_FILE(ap, master_zip);
	if (status != ARCHIVE_OK)
		panic("Failed to open the archive for reading; "
		      "archive error code is ", STRINGIZE(status));

	/* A place inside the archive to extract pages from. */
	strcpy(archive_path, "tldr-master");
	strcat(archive_path, PAGES_LANG);
	strcat(archive_path, "/");
	while (archive_read_next_header(ap, &aep) != ARCHIVE_EOF)
		if (!strcmp(archive_entry_pathname(aep), archive_path))
			break;
	while (archive_read_next_header(ap, &aep) != ARCHIVE_EOF) {
		if (strncmp(archive_entry_pathname(aep),
		    archive_path, strlen(archive_path)))
			break;
		/* A place to put the extracted pages to. */
		strcpy(page_path, getenv("HOME"));
		strcat(page_path, PAGES_PATH);
		strcat(page_path, strchr(strchr(archive_entry_pathname(aep),'/')
		       + 1, '/'));
		archive_entry_set_pathname(aep,page_path);
		status = archive_read_extract(ap, aep, 0);
		if (status != ARCHIVE_OK)
			panic("Failed to extract the archive; "
			      "archive error code is ", STRINGIZE(status));
	}
	archive_read_free(ap);
	fclose(master_zip);
}

/*
 * Create a plain text file with that serves as a database
 * with pages and their location.
 */
void
create_index(const char *pages_path, FILE *indexp)
{

	DIR *dir;
	struct dirent *entry;
	
	dir = opendir(pages_path);
	if (dir == NULL)
		panic("Something wrong with the path to be indexed; ",
		      pages_path);
	
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR) {
			char path[MAX_PATH];
			if (strcmp(D_NAME, ".") == 0 ||
			    strcmp(D_NAME, "..") == 0)
				continue;
			snprintf(path, sizeof(path), "%s/%s",
				 pages_path, D_NAME);
			create_index(path, indexp);
			continue;
		}
		/* Skip the index file */
		if (!strcmp(D_NAME, "index"))
			continue;
		/*
		 * Write an entry per line in the following form...
		 * '<page_name>.md/<platform>'
		 *
		 * Example: 7z.md/common
		 */
		fprintf(indexp, "%s%s\n", D_NAME, strrchr(pages_path, '/'));
	}
	closedir(dir);
}

void
display_page(char *pages_path, const char *page_name, FILE *indexp,
	     const char *platform)
{
	char buf[MAX_INDEX_ENTRY];
	char *chp;

	while (fgets(buf, MAX_INDEX_ENTRY, indexp) != NULL) {
		chp = strchr(buf, '\n');
		*chp = '\0';
		chp = strchr(buf, '/');
		*chp = '\0';
		/* -3 stands for minus the file extension '.md'. */
		if ((!strncmp(page_name, buf, strlen(buf) - 3)) &&
		    (strlen(page_name) == (strlen(buf) - 3))) {
			/*
			 * If the platform is specified and does not match
			 * then keep searching.
			 */
			if (platform != NULL &&
			    strcmp(platform, strchr(buf, '\0') + 1))
				continue;

			/* A path to the page. */
			strcat(pages_path, "/");
			strcat(pages_path, strchr(buf, '\0') + 1);
			strcat(pages_path, "/");
			strcat(pages_path, buf);

			cat_page(pages_path, strchr(buf, '\0') + 1);
			return;
		}
	}
	panic("The page has not been found.", NULL);
}


int
main(int argc, char **argv)
{
	FILE *indexp;
	char path[MAX_PATH];

	/* Path to the index file */
	strcpy(path, getenv("HOME"));
	strcat(path, PAGES_PATH);
	strcat(path, "/index");

	if (argc < 2 || !strcmp(OPTION, "-h") || !strcmp(OPTION, "--help")) {
		/*
		 * Show help if there are no arguments specified
		 * of if '-h'/'--help' is passed.
		 */
		print_help();
	} else if (!strcmp(OPTION, "-u") || !strcmp(OPTION, "--update")) {
		/*
		 * Fetch pages from GitHub repository,
		 * then extract pages into PAGES_PATH,
		 * and create the index file with information about them.
		 */
		printf("Downloading the archive with pages... ");
		fflush(stdout);
		fetch_pages();
		puts("Done.");

		printf("Extracting the pages... ");
		fflush(stdout);
		extract_pages();
		puts("Done.");

		printf("Creating the index file... ");
		fflush(stdout);
		indexp = fopen(path, WRITE_MODE);
		if (indexp == NULL)
			panic("Failed to create the index file", NULL);
		/* Path to the pages. */
		strcpy(path, getenv("HOME"));
		strcat(path, PAGES_PATH);
		create_index(path, indexp);
		fclose(indexp);
		puts("Done.");
	} else if (!strcmp(OPTION, "-l") || !strcmp(OPTION, "--list")) {
		/* List available pages. */
		indexp = fopen(path, READ_MODE);
		if (indexp == NULL)
			panic("Failed to read the index file; ", "Run tldr --update");
		list_pages(indexp);
		fclose(indexp);
	} else if (!strcmp(OPTION, "-p") || !strcmp(OPTION, "--platform")) {
		/* Display a platform specific page. */
		if (*(argv + 3) == NULL)
			panic("No platform/page specified", NULL);
		indexp = fopen(path, READ_MODE);
		if (indexp == NULL)
			panic("Failed to read the index file; ", "Run tldr --update");
		strcpy(path, getenv("HOME"));
		strcat(path, PAGES_PATH);
		display_page(path, *(argv + 3), indexp, *(argv + 2));
	} else {
		/* Display a page. */
		indexp = fopen(path, READ_MODE);
		if (indexp == NULL)
			panic("Failed to read the index file; ", "Run tldr --update");
		strcpy(path, getenv("HOME"));
		strcat(path, PAGES_PATH);
		display_page(path, OPTION, indexp, NULL);
	}
}
