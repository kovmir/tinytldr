/* MIT License
 * Copyright 2024 Ivan Kovmir */

/* Includes */
/* cURL must be included before libarchive in order to avoid a compiler
 * warning on Windows regarding the Winsock2 library. */
#include <curl/curl.h>

#include <dirent.h>
#include <ftw.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
/* libarchive */
#include <archive.h>
#include <archive_entry.h>

/* Constants and Macros */
#define BUF_SIZE 1024
#define D_NAME entry->d_name
#define ENABLE_WIN_VT100_OUT 7
#define NULL_TERMINATE(arr, size) (arr[(size)-1] = 0)

/* Function prototypes */
/* Prints instructions on how to use the program. */
static void  print_usage(void);
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
print_usage(void)
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
	if (!tldr_archive) {
		fprintf(stderr,
			"failed to create a temporary file: %s\n", zip_path);
		exit(1);
	}

	curl_global_init(CURL_GLOBAL_ALL);
	ceh = curl_easy_init();
	curl_easy_setopt(ceh, CURLOPT_WRITEDATA, tldr_archive);
	curl_easy_setopt(ceh, CURLOPT_URL, PAGES_URL);
	curl_easy_setopt(ceh, CURLOPT_ERRORBUFFER, err_curl);

	cres = curl_easy_perform(ceh);

	curl_easy_cleanup(ceh);
	curl_global_cleanup();
	fclose(tldr_archive);
	if (cres != CURLE_OK) {
		fprintf(stderr, "failed to fetch pages: %s\n", err_curl);
		exit(1);
	}
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
	if (tldr_archive == NULL) {
		fprintf(stderr, "failed to open the archive: %s\n", zip_path);
		exit(1);
	}

	ap = archive_read_new();
	if (ap == NULL) {
		fprintf(stderr, "failed to archive_read_new()\n");
		exit(1);
	}
	archive_read_support_format_zip(ap);
	ares = archive_read_open_FILE(ap, tldr_archive);
	if (ares != ARCHIVE_OK) {
		fprintf(stderr, "failed to archive_read_open_FILE(): %s\n",
		    archive_error_string(ap));
		exit(1);
	}

	/* A place inside the archive to extract pages from. */
	snprintf(src_path, BUF_SIZE, "%s/%s/", PAGES_DIR, PAGES_LANG);

	/* Find the folder within the archive to extract from. */
	while (archive_read_next_header(ap, &aep) != ARCHIVE_EOF)
		if (!strcmp(archive_entry_pathname(aep), src_path))
			break;
	while (archive_read_next_header(ap, &aep) != ARCHIVE_EOF) {
		if (strncmp(archive_entry_pathname(aep),
				    src_path, strlen(src_path)))
			break;

		snprintf(dest_path, BUF_SIZE, "%s/%s%s",
				getenv("HOME"), PAGES_PATH,
				strchr(archive_entry_pathname(aep),'/'));

		archive_entry_set_pathname(aep, dest_path);
		ares = archive_read_extract(ap, aep, 0);
		if (ares != ARCHIVE_OK) {
			fprintf(stderr,
				"failed to archive_read_extract(): %s\n",
				archive_error_string(ap));
			exit(1);
		}
	}
	archive_read_free(ap);
	fclose(tldr_archive);
	remove(zip_path);
}

void
index_pages(void)
{
	char buf[BUF_SIZE];

	snprintf(buf, BUF_SIZE, "%s/%s/%s",
		getenv("HOME"), PAGES_PATH, PAGES_LANG);

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

	if (typeflag != FTW_F) /* Skip everything except files. */
		return 0;

	/* Truncate the full path to include only the filename and last dir. */
	fprintf(tldr_index, "%s\n",
			strchr(strstr(path, PAGES_LANG)+1, '/')+1);
	return 0;
}

void
delete_pages(void)
{
	char buf[BUF_SIZE];

	snprintf(buf, BUF_SIZE, "%s/%s", getenv("HOME"), PAGES_PATH);
	nftw(buf, delete_nftw_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int
delete_nftw_cb(const char *path, const struct stat *sb,
				int typeflag, struct FTW *ftwbuf)
{
	(void)sb; /* Suppress compiler warnings about unused arguments. */
	(void)ftwbuf;
	(void)typeflag;

	if (remove(path) != 0) {
		fprintf(stderr, "failed to remove file %s: %s\n",
				path, strerror(errno));
		exit(1);
	}
	return 0;
}

void
list_pages(void)
{
	char buf[BUF_SIZE];

	tldr_index = open_index("r");
	while(fgets(buf, BUF_SIZE, tldr_index)) {
		NULL_TERMINATE(buf, BUF_SIZE);
		printf("%s", buf);
	}
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
			if(!strcmp(page_filename, buf)) {
				*strchr(buf, '\n') = '\0';
				fclose(tldr_index);
				return buf;
			}
		} else { /* command */
			if (!strcmp(page_filename, strchr(buf, '/')+1)) {
				*strchr(buf, '\n') = '\0';
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
	if (!dest_path) {
		fprintf(stderr, "the page has not been found: %s\n",
				page_name);
		exit(1);
	}

	snprintf(buf, BUF_SIZE, "%s/%s/%s/%s",
			getenv("HOME"), PAGES_PATH, PAGES_LANG, dest_path);

	page = fopen(buf, "r");
	setup_console(); /* Enables VT100 processing in Win10 1503+ */
	while (fgets(buf, BUF_SIZE, page)) {
		NULL_TERMINATE(buf, BUF_SIZE);
		if (!strcmp(buf, "\n")) {
			continue; /* Skip empty lines. */
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

	snprintf(buf, BUF_SIZE, "%s/%s/%s",
			getenv("HOME"), PAGES_PATH, "index");
	fp = fopen(buf, mode);
	if (!fp) {
		fprintf(stderr, "failed to open index, run 'tldr -u'\n");
		exit(1);
	}
	return fp;
}

int
main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "ludh")) != -1) {
		switch (opt) {
		case 'l':
			list_pages();
			return 0;
		case 'u':
			puts("Fetching pages...");
			fetch_pages();
			puts("Extracting pages...");
			extract_pages();
			puts("Indexing pages...");
			index_pages();
			return 0;
		case 'd':
			delete_pages();
			return 0;
		case 'h':
			print_usage();
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
