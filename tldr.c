/*
 * Copyright 2022 Ivan Kovmir
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
/* cURL must be included before libarchive in order to avoid a compiler
 * warning on Windows regarding the Winsock2 library. */
#include <curl/curl.h>

#include <archive.h>
#include <archive_entry.h>
#include <dirent.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants and Macros */
#define BUF_SIZE 1024
#define D_NAME entry->d_name
#define ENABLE_WIN_VT100_OUT 7

/* Function prototypes */
/* Print error message and terminate the execution */
static void  error_terminate(const char *msg, const char *details);
/* Prints instructions on how to use the program. */
static void  tldr_usage(void);
/* Downloads pages. */
static void  fetch_pages(void);
/* Extracts pages and put them in place. */
static void  extract_pages(void);
/* Creates index file of all pages. */
static void  index_pages(void);
static int   ftw_callback(const char *path, const struct stat *sb, int typeflag);
/* Prints all page names. */
static void  list_pages(void);
/* Returns a file path to a given page. */
static char *find_page(const char *page_name);
/* Enables VT100 mode on Win10 1503+ ConHost & wt+mintty; else, empty. */
static void  setup_console(void);
/* Restores previous mode on Win10 1503+ ConHost & wt+mintty; else, empty. */
static void  restore_console(void);
/* Prints a given page. */
static void  display_page(const char *dest_path);
/* Opens index file performing all the necessary checking. */
static FILE *open_index(const char *mode);

/* Save locations, styling, and other settings are set via config.h. */
#include "config.h"
/* Resets console styling back to default (usually white-on-black),
   and clears rest of current line for consistency on Windows. */
static const char *RESET_STYLING = "\033[0m\033[0K";
/* Index file to hold available page names. */
static FILE *tldr_index;
/* Path to download the tldr archive to. */
static char zip_path[BUF_SIZE];

inline void
error_terminate(const char *msg, const char *details)
{
	fprintf(stderr, "%s; details: %s.\n", msg, details? details : "none");
	exit(1);
}

inline void
tldr_usage(void)
{
	printf("USAGE: tldr [options] <[platform/]command>\n\n"
	    "[options]\n"
	    "\t-h:\tthis help overview\n"
	    "\t-l:\tshow all available pages\n"
	    "\t-u:\tfetch lastest copies of man pages\n\n"
	    "[platform]\n"
	    "\tandroid\n"
	    "\tcommon\n"
	    "\tindex\n"
	    "\tlinux\n"
	    "\tosx\n"
	    "\tsunos\n"
	    "\twindows\n\n"
	    "<command>\n"
	    "\tShow examples for this command\n");
}

void
fetch_pages(void)
{
	CURL *ceh; /* cURL easy handle. */
	CURLcode cres; /* cURL operation result. */
	char err_curl[CURL_ERROR_SIZE]; /* Curl error message buffer. */
	FILE *tldr_archive; /* File to download to. */
	
	if(getenv("TEMP") != NULL) /* Defined by Windows. */
		strcpy(zip_path, getenv("TEMP"));
	else if (getenv("TEMPDIR") != NULL) /* Can be defined by *nix users. */
		strcpy(zip_path, getenv("TEMPDIR"));
	else  /* If neither defined, presume default *nix tmp path. */
		strcpy(zip_path, "/tmp");
	strcat(zip_path, "/tldr_pages.zip");
	
	/* Write in binary mode to avoid mangling with CRLFs in Windows. */
	tldr_archive = fopen(zip_path, "wb");
	if (!tldr_archive)
		error_terminate("Failed to create a temporary file", NULL);

	curl_global_init(CURL_GLOBAL_ALL);
	ceh = curl_easy_init();
	curl_easy_setopt(ceh, CURLOPT_WRITEDATA, tldr_archive);
	curl_easy_setopt(ceh, CURLOPT_URL, PAGES_URL);
	curl_easy_setopt(ceh, CURLOPT_ERRORBUFFER, err_curl);

	cres = curl_easy_perform(ceh);
	curl_easy_cleanup(ceh);
	curl_global_cleanup();
	fclose(tldr_archive);
	if (cres != CURLE_OK)
		error_terminate("Failed to fetch pages", err_curl);
}

void
extract_pages(void)
{
	char src_path[BUF_SIZE]; /* Path within archive to extract from. */
	char dest_path[BUF_SIZE]; /* Path to save pages to. */
	FILE *tldr_archive; /* Pointer to downloaded zip file. */
	int ares; /* libarchive status. */
	struct archive *ap;
	struct archive_entry *aep;
	tldr_archive = fopen(zip_path, "r");
	if (tldr_archive == NULL)
		error_terminate("Failed to open the archive", NULL);

	ap = archive_read_new();
	if (ap == NULL)
		error_terminate("Failed to archive_read_new()", NULL);
	archive_read_support_format_zip(ap);
	ares = archive_read_open_FILE(ap, tldr_archive);
	if (ares != ARCHIVE_OK)
		error_terminate("Failed to archive_read_open_FILE()",
		    archive_error_string(ap));

	/* A place inside the archive to extract pages from. */
	strcpy(src_path, "tldr-master");
	strcat(src_path, PAGES_LANG);
	strcat(src_path, "/");

	/* Find the folder within the archive to extract from. */
	while (archive_read_next_header(ap, &aep) != ARCHIVE_EOF)
		if (!strcmp(archive_entry_pathname(aep), src_path))
			break;
	while (archive_read_next_header(ap, &aep) != ARCHIVE_EOF) {
		if (strncmp(archive_entry_pathname(aep),
		    src_path, strlen(src_path)))
			break;

		/* A place to put the extracted pages to. */
		strcpy(dest_path, getenv("HOME"));
		strcat(dest_path, PAGES_PATH);
		strcat(dest_path, strchr(archive_entry_pathname(aep),'/'));
		archive_entry_set_pathname(aep, dest_path);
		ares = archive_read_extract(ap, aep, 0);
		if (ares != ARCHIVE_OK)
			error_terminate("Failed to archive_read_extract()",
			    archive_error_string(ap));
	}
	archive_read_free(ap);
	fclose(tldr_archive);
	remove(zip_path);
}

void
index_pages(void)
{
	char buf[BUF_SIZE];

	strcpy(buf, getenv("HOME"));
	strcat(buf, PAGES_PATH);
	strcat(buf, PAGES_LANG);
	tldr_index = open_index("w");
	ftw(buf, ftw_callback, 10);
	fclose(tldr_index);
}

int
ftw_callback(const char *path, const struct stat *sb, int typeflag)
{
	(void)sb; /* Suppress compiler warnings about unused *sb. */
	if (typeflag == FTW_F)
		fprintf(tldr_index, "%s\n", strchr(strstr(path, PAGES_LANG)+1, '/')+1);
	return 0;
}

void
list_pages(void)
{
	char buf[BUF_SIZE];
	tldr_index = open_index("r");
	while(fgets(buf, BUF_SIZE, tldr_index)) {
		printf("%s", buf);
	}
	fclose(tldr_index);
}

char *
find_page(const char *page_name)
{
	static char buf[BUF_SIZE];

	tldr_index = open_index("r");
	char page_filename[strlen(page_name)+5]; /* for '.md\n\0' */
	strcpy(page_filename, page_name);
	strcat(page_filename, ".md\n");
	while(fgets(buf, BUF_SIZE, tldr_index)) {
		/* page_name is either 'command' or 'platform/command'. */
		if (strchr(page_name, '/')) { /* platform/command */
			if(!strcmp(page_filename, buf)) {
				*strchr(buf, '\n') = '\0';
				return buf;
			}
		} else { /* command */
			if (!strcmp(page_filename, strchr(buf, '/')+1)) {
				*strchr(buf, '\n') = '\0';
				return buf;
			}
		}
	}
	fclose(tldr_index);
	return NULL;
}
#ifdef _WIN32
static DWORD  outmode_init;  /* will be set to initial console mode value. */
static HANDLE stdout_handle; /* handle for current console session. */
/* ref: https://docs.microsoft.com/en-us/windows/console/setconsolemode */
void
setup_console(void)
{
	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(stdout_handle, &outmode_init);
	if (!SetConsoleMode(stdout_handle, ENABLE_WIN_VT100_OUT)){
		error_terminate("\nYou are likely using an older version of Windows,"
		"\nwhich is not yet compatible with this client.\n", NULL);
	}
}
void
restore_console(void)
{	/* Error catching would be superfluous here given program flow. */
	SetConsoleMode(stdout_handle, outmode_init);
}
#else
void setup_console(void){}
void restore_console(void){}
#endif
void
display_page(const char *page_name)
{
	const char *dest_path = find_page(page_name);
	if (!dest_path) {
		puts("The page has not been found.");
		exit(1);
	}

	char buf[BUF_SIZE];
	FILE *page;
	strcpy(buf, getenv("HOME"));
	strcat(buf, PAGES_PATH);
	strcat(buf, PAGES_LANG);
	strcat(buf, "/");
	strcat(buf, dest_path);

	page = fopen(buf, "r");
	setup_console(); /* Enables VT100 processing in Win10 1503+ */
	while (fgets(buf, BUF_SIZE, page)) {
		if (!strcmp(buf, "\n")) {
			/* Skip empty lines. */
			continue;
		}
		if (buf[0] == '#') {
			printf("%s%s%s", HEADING_STYLE, buf, RESET_STYLING);
		}
		if (buf[0] == '>') {
			printf("%s%s%s", SUBHEADING_STYLE, buf, RESET_STYLING);
		}
		if (buf[0] == '-') {
			printf("%s%s%s", COMMAND_DESC_STYLE, buf, RESET_STYLING);
		}
		if (buf[0] == '`') {
			printf("%s%s%s", COMMAND_STYLE, buf, RESET_STYLING);
		}

	}
	fclose(page);
	restore_console(); /* Restores previous console mode in Win10 1503+ */

}

FILE *
open_index(const char *mode)
{
	char buf[BUF_SIZE];
	FILE *fp;
	strcpy(buf, getenv("HOME"));
	strcat(buf, PAGES_PATH);
	strcat(buf, "/index");
	fp = fopen(buf, mode);
	if (!fp)
		error_terminate("Failed to open index; "
		    "you should probably run 'tldr -u'", mode);
	return fp;
}

int
main(int argc, char *argv[])
{
	if (argc != 2) { /* Wrong number of arguments. */
		tldr_usage();
		return 1;
	}

	if (!strcmp("-h", argv[1])) { /* Show usage help. */
		tldr_usage();
	} else if (!strcmp("-u", argv[1])) { /* Update pages. */
		puts("Fetching pages...");
		fetch_pages();
		puts("Extracting pages...");
		extract_pages();
		puts("Indexing pages...");
		index_pages();
	} else if (!strcmp("-l", argv[1])) { /* Print all pages names. */
		list_pages();
	} else {
		display_page(argv[1]); /* Display a given page. */
	}
	return 0;
}
