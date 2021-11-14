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

/* Function prototypes */
/* Print error message and terminate the execution */
void  error_terminate(const char *msg, const char *details);
/* Prints instructions on how to use the program. */
void  tldr_usage(void);
/* Downloads pages. */
void  fetch_pages(void);
/* Extracts pages and put them in place. */
void  extract_pages(void);
/* Creates .csv index file of all pages. */
void  index_pages(void);
/* Prints all page names. */
void  list_pages(void);
/* Returns a file path to a given page. */
char *find_page(const char *page_name);
/* Prints a given page. */
void  display_page(const char *page_path);

void
error_terminate(const char *msg, const char *details)
{
	fprintf(stderr, "%s; Details: %s\n", msg, details);
	exit(1);
}

void
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

	if (!strcmp("-u", argv[1])) {
		fetch_pages();
		extract_pages();
		index_pages();
	} else if (!strcmp("-l", argv[1])) {
		list_pages();
	} else {
		display_page(find_page(""));
	}

	return 0;
}
