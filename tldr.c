/*
 * Copyright 2021 Ivan Kovmir
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>

/* Constants and Macros */
#define MAX_PATH 1024

/* Function prototypes */
/* Print error message and terminate the execution */
static void  error_terminate(const char *msg, const char *details);
/* Prints instructions on how to use the program. */
static void  tldr_usage(void);
/* Downloads pages. */
static void  fetch_pages(void);
/* Extracts pages and put them in place. */
static void  extract_pages(void);
/* Creates .csv index file of all pages. */
static void  index_pages(void);
/* Prints all page names. */
static void  list_pages(void);
/* Returns a file path to a given page. */
static char *find_page(const char *page_name);
/* Prints a given page. */
static void  display_page(const char *page_path);

#include "config.h"

inline void
error_terminate(const char *msg, const char *details)
{
	fprintf(stderr, "%s; details: %s.\n", msg, details? details : "none");
	exit(1);
}

inline void
tldr_usage(void)
{
	printf("USAGE: tldr [options] <[platform,]command>\n\n"
	    "[options]\n"
	    "\t-h:\tthis help overview\n"
	    "\t-l:\tshow all available pages\n"
	    "\t-u:\tfetch lastest copies of cached pages\n\n"
	    "[platform]\n"
	    "\tandroid\n"
	    "\tcommon\n"
	    "\tindex\n"
	    "\tlinux\n"
	    "\tosx\n"
	    "\tsunos\n"
	    "\twindows\n\n"
	    "\t<command>\n"
	    "\tShow examples for this command\n");
}

void
fetch_pages(void)
{
	CURL *ceh; /* cURL easy handle. */
	CURLcode cres; /* cURL operation result. */
	char err_curl[CURL_ERROR_SIZE]; /* Curl error message buffer. */
	FILE *tldr_archive; /* File to download to. */

	tldr_archive = fopen(PAGES_TMP, "w");
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
}

void
index_pages(void)
{

}

void
list_pages(void)
{

}

char *
find_page(const char *page_name)
{
	return "";
}

void
display_page(const char *page_path)
{

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
		fetch_pages();
		extract_pages();
		index_pages();
	} else if (!strcmp("-l", argv[1])) { /* Print all pages names. */
		list_pages();
	} else {
		display_page(find_page("")); /* Display a given page. */
	}
	return 0;
}
