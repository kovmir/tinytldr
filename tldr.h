/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#ifndef TLDR_H
#define TLDR_H

#include <stdio.h>

typedef struct {
	/* URL where to download pages from. */
	const char *pages_url;
	/* User agent used to download pages. */
	const char *user_agent;
	/* Extract pages from the archive here. */
	const char *pages_home;
	/* ANSI output styling. */
	const char *heading_style;
	const char *summary_style;
	const char *comment_style;
	const char *command_style;
	const char *reset_style;
	/* Print empty lines from pages? */
	int skip_empty;
	/* Apply styles? */
	int apply_styles;
	/* Output stream for displaying pages. */
	FILE *out;
} ConfigOpts;

typedef struct Config Config; /* Defined in tldr.c */

/* Allocate and populate config. */
Config *create_cfg(const ConfigOpts *opts);
/* Deallocate config recursively. */
void destroy_cfg(Config *cfg);
/* Download the archive with pages. */
int fetch_pages(const Config *cfg, FILE *dest);
/* Extract pages from the archive. */
int extract_pages(const Config *cfg, FILE *archive);
/* Find a page by file name. The caller must free the returned string. */
char *find_page(const Config *cfg, const char *name, const char *platform);
/* Write page to the given file. */
int print_page(const Config *cfg, FILE *page);
/* List all available pages. */
int list_pages(const Config *cfg);

#endif /* TLDR_H */
