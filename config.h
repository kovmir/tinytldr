/* See LICENSE file for copyright and license details. */

/* URL to download man pages. */
static const char *PAGES_URL = "https://codeload.github.com/tldr-pages/tldr/zip/master";

/* Path to temporary store the downloaded archive with pages. */
static const char *PAGES_TMP = "/tmp/tldr_pages.zip";

/* Path to store man pages relative to $HOME. */
static const char *PAGES_PATH = "/.config/tldr";

/* Pages language, uncomment ONE of the following. */
static const char *PAGES_LANG = "/pages"; /* English */
/* static const char *PAGES_LANG = "/pages.de"; */
/* static const char *PAGES_LANG = "/pages.es"; */
/* static const char *PAGES_LANG = "/pages.fr"; */
/* static const char *PAGES_LANG = "/pages.hbs"; */
/* static const char *PAGES_LANG = "/pages.it"; */
/* static const char *PAGES_LANG = "/pages.ja"; */
/* static const char *PAGES_LANG = "/pages.ko"; */
/* static const char *PAGES_LANG = "/pages.pt_BR"; */
/* static const char *PAGES_LANG = "/pages.pt_PT"; */
/* static const char *PAGES_LANG = "/pages.ta"; */
/* static const char *PAGES_LANG = "/pages.zh"; */

/* Colors and styling. */
static const char *HEADING_STYLE = "\033[31m";
static const char *SUBHEADING_STYLE = "\033[4m";
static const char *COMMAND_DESC_STYLE = "\033[22;32m";
static const char *COMMAND_STYLE = "\033[1m";
