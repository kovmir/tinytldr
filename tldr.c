/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */

/* Includes */
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <ftw.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* cURL must be included before libarchive in order to avoid a compiler
 * warning on Windows regarding the Winsock2 library. */
#include <curl/curl.h>
/* libarchive */
#include <archive.h>
#include <archive_entry.h>

/* Constants and Macros */
#define BUF_SIZE 4096
#define D_NAME entry->d_name
#define ENABLE_WIN_VT100_OUT 7
#define NULL_TERMINATE(arr, size) (arr[(size)-1] = 0)

/* Function prototypes */
/* Prints instructions on how to use the program. */
static void  print_usage(void);
/* Print the contents of 'config.h'. */
static void  print_config(void);
/* Downloads pages. */
static void  fetch_pages(void);
/* Extracts pages and put them in place. */
static void  extract_pages(void);
/* Creates index file of all pages. */
static void  index_pages(void);
static int   index_nftw_cb(const char *path, const struct stat *sb,
				int typeflag, struct FTW *ftwbuf);
/* Delete all pages from disk. */
static void  delete_pages(void);
static int   delete_nftw_cb(const char *path, const struct stat *sb,
				int typeflag, struct FTW *ftwbuf);
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

#ifndef DEBUG
/* Save locations, styling, and other settings are set via config.h. */
#include "config.h"
#else
static const char *PAGES_URL = DEBUG_PAGES_URL;
static const char *PAGES_DIR = DEBUG_PAGES_DIR;
static const char *PAGES_PATH = DEBUG_PAGES_PATH;
static const char *PAGES_LANG = DEBUG_PAGES_LANG;
static const char *HEADING_STYLE = DEBUG_HEADING_STYLE;
static const char *SUBHEADING_STYLE = DEBUG_SUBHEADING_STYLE;
static const char *COMMAND_DESC_STYLE = DEBUG_COMMAND_DESC_STYLE;
static const char *COMMAND_STYLE = DEBUG_COMMAND_STYLE;
#endif

/* Resets console styling back to default (usually white-on-black),
   and clears rest of current line for consistency on Windows. */
#ifndef DEBUG
static const char *RESET_STYLING = "\033[0m\033[0K";
#else
static const char *RESET_STYLING = "-";
#endif /* DEBUG */
/* Index file to hold available page names. */
static FILE *tldr_index;
/* Path to download the tldr archive to. */
static char zip_path[BUF_SIZE];

inline void
print_usage(void)
{
	printf("USAGE: tldr [options] <[platform/]command>\n\n"
	    "[options]\n"
	    "\t-u\tfetch lastest copies of man pages\n"
	    "\t-d\tdelete pages from disk\n"
	    "\t-i\trebuild pages index\n"
	    "\t-l\tlist all available pages\n"
	    "\t-h\tshow this help message\n"
	    "\t-c\tview compiled config values\n"
	    "\t-v\tdisplay program version\n\n"
	    "[platform]\n"
	    "\tandroid\n"
	    "\tcommon\n"
	    "\tfreebsd\n"
	    "\tlinux\n"
	    "\tnetbsd\n"
	    "\topenbsd\n"
	    "\tosx\n"
	    "\tsunos\n"
	    "\twindows\n\n"
	    "<command>\n"
	    "\tShow examples for this command\n");
}

inline void
print_config(void)
{
	printf("PAGES_URL='%s'\n"
		"PAGES_DIR='%s'\n"
		"PAGES_PATH='%s'\n"
		"PAGES_LANG='%s'\n"
		"%sHEADING_STYLE%s\n"
		"%sSUBHEADING_STYLE%s\n"
		"%sCOMMAND_DESC_STYLE%s\n"
		"%sCOMMAND_STYLE%s\n",
		PAGES_URL,
		PAGES_DIR,
		PAGES_PATH,
		PAGES_LANG,
		HEADING_STYLE, RESET_STYLING,
		SUBHEADING_STYLE, RESET_STYLING,
		COMMAND_DESC_STYLE, RESET_STYLING,
		COMMAND_STYLE, RESET_STYLING);
}

void
fetch_pages(void)
{
	CURL *curl_handle; /* cURL easy handle. */
	CURLcode curl_res; /* cURL operation result. */
	char curl_err[CURL_ERROR_SIZE]; /* Curl error message buffer. */
	FILE *tldr_archive; /* Downloaded file. */

	if (getenv("TEMP") != NULL) /* Defined by Windows. */
		strcpy(zip_path, getenv("TEMP"));
	else if (getenv("TEMPDIR") != NULL) /* Can be defined by *nix users. */
		strcpy(zip_path, getenv("TEMPDIR"));
	else  /* If neither defined, presume default *nix tmp path. */
		strcpy(zip_path, "/tmp");
	strcat(zip_path, "/tldr_pages.zip");
	NULL_TERMINATE(zip_path, BUF_SIZE);

	/* Write in binary mode to avoid mangling with CRLFs in Windows. */
	tldr_archive = fopen(zip_path, "wb");
	if (tldr_archive == NULL)
		err(1, "failed to open %s", zip_path);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, tldr_archive);
	curl_easy_setopt(curl_handle, CURLOPT_URL, PAGES_URL);
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, curl_err);

	curl_res = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	fclose(tldr_archive);

	if (curl_res != CURLE_OK)
		errx(1, "failed to fetch pages: %s", curl_err);
}

void
extract_pages(void)
{
	char src_path[BUF_SIZE]; /* Path within archive to extract from. */
	char dest_path[BUF_SIZE]; /* Save the extracted pages here. */
	FILE *tldr_archive; /* Pointer to downloaded zip file. */
	int liba_res; /* libarchive result. */
	struct archive *archp;
	struct archive_entry *entryp;

	tldr_archive = fopen(zip_path, "r");
	if (tldr_archive == NULL)
		err(1, "failed to open %s", zip_path);

	archp = archive_read_new();
	if (archp == NULL)
		errx(1, "failed to archive_read_new()");

	archive_read_support_format_zip(archp);

	liba_res = archive_read_open_FILE(archp, tldr_archive);
	if (liba_res != ARCHIVE_OK) {
		errx(1, "failed to archive_read_open_FILE(): %s",
		    archive_error_string(archp));
	}

	snprintf(src_path, BUF_SIZE, "%s/%s/", PAGES_DIR, PAGES_LANG);

	/* Find the directory within the archive to extract from. */
	while (archive_read_next_header(archp, &entryp) != ARCHIVE_EOF) {
		if (strcmp(archive_entry_pathname(entryp), src_path) == 0)
			break;
	}
	while (archive_read_next_header(archp, &entryp) != ARCHIVE_EOF) {
		if (strncmp(archive_entry_pathname(entryp), src_path,
				strlen(src_path)) != 0) {
			break; /* Done processing the pages directory. */
		}

		if (PAGES_PATH[0] == '~') {
			snprintf(dest_path, BUF_SIZE, "%s/%s/%s",
				getenv("HOME"), PAGES_PATH+2,
				strchr(archive_entry_pathname(entryp),'/')+1);
		} else {
			snprintf(dest_path, BUF_SIZE, "%s/%s",
				PAGES_PATH,
				strchr(archive_entry_pathname(entryp),'/')+1);
		}

		archive_entry_set_pathname(entryp, dest_path);
		liba_res = archive_read_extract(archp, entryp, 0);
		if (liba_res != ARCHIVE_OK) {
			errx(1, "failed to archive_read_extract(): %s",
				archive_error_string(archp));
		}
	}
	archive_read_free(archp);
	fclose(tldr_archive);
	remove(zip_path);
}

void
index_pages(void)
{
	char buf[BUF_SIZE];

	if (PAGES_PATH[0] == '~') {
		snprintf(buf, BUF_SIZE, "%s/%s/%s",
			getenv("HOME"), PAGES_PATH+2, PAGES_LANG);
	} else {
		snprintf(buf, BUF_SIZE, "%s/%s", PAGES_PATH, PAGES_LANG);
	}

	tldr_index = open_index("w");
	nftw(buf, index_nftw_cb, 10, FTW_PHYS);
	fclose(tldr_index);
}

int
index_nftw_cb(const char *path, const struct stat *sb,
			int typeflag, struct FTW *ftwbuf)
{
	(void)sb; /* Suppress compiler warnings about unused arguments. */
	(void)ftwbuf;

	if (typeflag != FTW_F)
		return 0; /* Skip everything except files. */

	/* Truncate the full path to include only the filename and last dir. */
	fprintf(tldr_index, "%s\n",
			strchr(strstr(path, PAGES_LANG)+1, '/')+1);
	return 0;
}

void
delete_pages(void)
{
	char buf[BUF_SIZE];

	if (PAGES_PATH[0] == '~')
		snprintf(buf, BUF_SIZE, "%s/%s", getenv("HOME"), PAGES_PATH+2);
	else
		strncpy(buf, PAGES_PATH, BUF_SIZE);
	nftw(buf, delete_nftw_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int
delete_nftw_cb(const char *path, const struct stat *sb,
				int typeflag, struct FTW *ftwbuf)
{
	(void)sb; /* Suppress compiler warnings about unused arguments. */
	(void)ftwbuf;
	(void)typeflag;

	if (remove(path) != 0)
		err(1, "failed to remove %s", path);
	return 0;
}

void
list_pages(void)
{
	char buf[BUF_SIZE];

	tldr_index = open_index("r");
	while(fgets(buf, BUF_SIZE, tldr_index))
		puts(buf);
	fclose(tldr_index);
}

char *
find_page(const char *page_name)
{
	static char buf[BUF_SIZE];
	char page_filename[strlen(page_name)+5]; /* for '.md\n\0' */

	tldr_index = open_index("r");
	snprintf(page_filename, BUF_SIZE, "%s.md\n", page_name);
	while(fgets(buf, BUF_SIZE, tldr_index)) {
		/* page_name is either 'command' or 'platform/command'. */
		if (strchr(page_name, '/')) { /* platform/command */
			if(strcmp(page_filename, buf) == 0) {
				*strchr(buf, '\n') = 0;
				fclose(tldr_index);
				return buf;
			}
		} else { /* command */
			if (!strcmp(page_filename, strchr(buf, '/')+1)) {
				*strchr(buf, '\n') = 0;
				fclose(tldr_index);
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
		fprintf(stderr, "\nyou are likely using an older version of Windows,"
		"\nwhich is not yet compatible with this client\n");
		exit(1);
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
	char buf[BUF_SIZE];
	char *dest_path;
	FILE *page;

	dest_path = find_page(page_name);
	if (!dest_path)
		errx(1, "%s has not been found", page_name);

	if (PAGES_PATH[0] == '~') {
		snprintf(buf, BUF_SIZE, "%s/%s/%s/%s",
			getenv("HOME"), PAGES_PATH+2, PAGES_LANG, dest_path);
	} else {
		snprintf(buf, BUF_SIZE, "%s/%s/%s",
			PAGES_PATH, PAGES_LANG, dest_path);
	}

	page = fopen(buf, "r");
	setup_console(); /* Enables VT100 processing in Win10 1503+ */
	while (fgets(buf, BUF_SIZE, page)) {
		if (strcmp(buf, "\n") == 0) {
			continue; /* Skip empty lines. */
		}
		if (buf[0] == '#') {
			printf("%s%s%s",
				HEADING_STYLE, buf, RESET_STYLING);
		}
		if (buf[0] == '>') {
			printf("%s%s%s",
				SUBHEADING_STYLE, buf, RESET_STYLING);
		}
		if (buf[0] == '-') {
			printf("%s%s%s",
				COMMAND_DESC_STYLE, buf, RESET_STYLING);
		}
		if (buf[0] == '`') {
			printf("%s%s%s",
				COMMAND_STYLE, buf, RESET_STYLING);
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

	if (PAGES_PATH[0] == '~') {
		snprintf(buf, BUF_SIZE, "%s/%s/%s",
				getenv("HOME"), PAGES_PATH+2, "index");
	} else {
		snprintf(buf, BUF_SIZE, "%s/%s", PAGES_PATH, "index");
	}

	fp = fopen(buf, mode);
	if (fp == NULL)
		err(1, "failed to open index, probably run `tldr -u`\n");
	return fp;
}

int
main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "udilhcv")) != -1) {
		switch (opt) {
		case 'u':
			puts("Fetching pages...");
			fetch_pages();
			puts("Extracting pages...");
			extract_pages();
			puts("Indexing pages...");
			index_pages();
			return 0;
		case 'd':
			puts("Deleting pages...");
			delete_pages();
			return 0;
		case 'i':
			puts("Indexing pages...");
			index_pages();
			return 0;
		case 'l':
			list_pages();
			return 0;
		case 'h':
			print_usage();
			return 0;
		case 'c':
			print_config();
			return 0;
		case 'v':
			puts(GIT_DESC" "BUILD_TYPE);
			return 0;
		default:
			print_usage();
			return 1;
		}
	}

	if (argc != 2) {
		print_usage();
		return 1;
	}

	display_page(argv[1]);
	return 0;
}
