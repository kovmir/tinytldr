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

build: CFLAGS += -DBUILD_TYPE=\"release\"
build:
	$(CC) $(CFLAGS) -O3 $(PROJECT).c $(LDFLAGS) -o $(PROJECT)

debug: CFLAGS += -DBUILD_TYPE=\"debug\"
debug:
	$(CC) $(CFLAGS) -g $(PROJECT).c $(LDFLAGS) -o $(PROJECT)

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
