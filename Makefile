CC ?= cc

LIBNOTIFY_LIBS = $(shell pkg-config --libs libarchive libcurl)
LIBNOTIFY_INCS = $(shell pkg-config --cflags libarchive libcurl)

LIBS += $(LIBNOTIFY_LIBS)
INCS += $(LIBNOTIFY_INCS)

CFLAGS += -std=c99
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wcast-align
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wno-format-truncation

CFLAGS += -D_XOPEN_SOURCE=500
CFLAGS += -DGIT_DESC=\"$(shell git describe --tags --always --dirty)\"

CFLAGS += $(INCS)
LDFLAGS += $(LIBS)

PREFIX ?= /usr/local
PROJECT = tldr

INSTALL ?= install
STRIP ?= strip

# Config values for debug build.
PAGES_URL = http://localhost:1337/archive.zip
PAGES_DIR = tldrpages
PAGES_PATH = ./debug_pages_dir
PAGES_LANG = lang
HEADING_STYLE = ^
SUBHEADING_STYLE = ^
COMMAND_DESC_STYLE = ^
COMMAND_STYLE = ^

build: CFLAGS += -DBUILD_TYPE=\"release\"
build:
	$(CC) -O2 $(CFLAGS) $(PROJECT).c $(LDFLAGS) -o $(PROJECT)

debug: CFLAGS += -DBUILD_TYPE=\"debug\"
debug:
	$(CC) -O0 -g $(CFLAGS) \
		-DDEBUG \
		-DDEBUG_PAGES_URL=\"$(PAGES_URL)\" \
		-DDEBUG_PAGES_DIR=\"$(PAGES_DIR)\" \
		-DDEBUG_PAGES_PATH=\"$(PAGES_PATH)\" \
		-DDEBUG_PAGES_LANG=\"$(PAGES_LANG)\" \
		-DDEBUG_HEADING_STYLE=\"$(HEADING_STYLE)\" \
		-DDEBUG_SUBHEADING_STYLE=\"$(SUBHEADING_STYLE)\" \
		-DDEBUG_COMMAND_DESC_STYLE=\"$(COMMAND_DESC_STYLE)\" \
		-DDEBUG_COMMAND_STYLE=\"$(COMMAND_STYLE)\" \
		$(PROJECT).c $(LDFLAGS) -o $(PROJECT)

gdb: debug
	gdb ./$(PROJECT)

memcheck: debug
	valgrind --leak-check=yes ./$(PROJECT)

memcheck_v: debug
	valgrind --leak-check=yes -v ./$(PROJECT)

memcheck_full: debug
	valgrind --leak-check=full --show-leak-kinds=all ./$(PROJECT)

clean:
	rm -f ./$(PROJECT)

strip:
	$(STRIP) ./$(PROJECT)

bench:
	hyperfine -N './tldr tar'

install:
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	$(INSTALL) ./$(PROJECT) "$(DESTDIR)$(PREFIX)/bin/$(PROJECT)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(PROJECT)"
	rmdir --ignore-fail-on-non-empty "$(DESTDIR)$(PREFIX)/bin"

.PHONY: all debug gdb memcheck memcheck_v memcheck_full clean strip install uninstall
