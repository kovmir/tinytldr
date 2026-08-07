/* URL to download the archive with man pages. */
static const char *PAGES_URL = "https://github.com/tldr-pages/tldr/releases/download/v2.3/tldr-pages.en.zip";
/* Path to store man pages. */
static const char *PAGES_HOME = "~/.local/share/tinytldr/pages";
/* Print empty lines from pages? */
static const int SKIP_EMPTY = 1;
/* Apply ANSI styling to pages? */
static const int APPLY_STYLES = 1;
/* Page styling.
 * See: https://en.wikipedia.org/wiki/ANSI_escape_code */
static const char *HEADING_STYLE = "\033[31m";
static const char *SUMMARY_STYLE = "\033[22;4m";
static const char *COMMENT_STYLE = "\033[22;32m";
static const char *COMMAND_STYLE = "\033[1m";
static const char *RESET_STYLE   = "\033[0m\033[0K";
