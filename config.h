/* See LICENSE file for copyright and license details. */

/* URL to download man pages. */
static const char *PAGES_URL = "https://codeload.github.com/tldr-pages/tldr/zip/main";

/* Path to store man pages relative to $HOME. */
static const char *PAGES_PATH = "/.config/tldr";

/* Pages language, uncomment ONE of the following. */
static const char *PAGES_LANG = "/pages";              /*  English */
/* static const char *PAGES_LANG = "/pages.de"; */     /*   German */
/* static const char *PAGES_LANG = "/pages.es"; */     /*  Spanish */
/* static const char *PAGES_LANG = "/pages.fr"; */     /*   French */
/* static const char *PAGES_LANG = "/pages.hbs"; */    /* Serbo-Croatian */
/* static const char *PAGES_LANG = "/pages.it"; */     /*  Italian */
/* static const char *PAGES_LANG = "/pages.ja"; */     /* Japanese */
/* static const char *PAGES_LANG = "/pages.ko"; */     /*   Korean */
/* static const char *PAGES_LANG = "/pages.pt_BR"; */  /* Portuguese: BR */
/* static const char *PAGES_LANG = "/pages.pt_PT"; */  /* Portuguese: PT */
/* static const char *PAGES_LANG = "/pages.ta"; */     /*   Tamil  */ 
/* static const char *PAGES_LANG = "/pages.zh"; */     /*  Chinese */

/* Colors and styling. */
static const char *HEADING_STYLE = "\033[31m";
static const char *SUBHEADING_STYLE = "\033[22;4m";
static const char *COMMAND_DESC_STYLE = "\033[22;32m";
static const char *COMMAND_STYLE = "\033[1m";
