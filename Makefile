CC ?= cc

LIB_CFLAGS := $(shell pkg-config --cflags libcurl libarchive)
LIB_LDLIBS := $(shell pkg-config --libs libcurl libarchive)
GIT_VERSION := $(shell git describe --tags --always --dirty)

CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CFLAGS += $(LIB_CFLAGS)

LDLIBS += $(LIB_LDLIBS)

BUILD_BIN = tldr
TEST_BIN  = tldr_test

PREFIX ?= /usr/local
INSTALL ?= install

all: build

build: $(BUILD_BIN)

test: $(TEST_BIN)

$(BUILD_BIN): main.o tldr.o

$(TEST_BIN): tldr_test.o tldr.o

main.o: config.h tldr.h

tldr.o: tldr.h

install:
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	$(INSTALL) ./$(BUILD_BIN) "$(DESTDIR)$(PREFIX)/bin/$(BUILD_BIN)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(BUILD_BIN)"
	rmdir --ignore-fail-on-non-empty "$(DESTDIR)$(PREFIX)/bin"

clean:
	rm -f *.o $(BUILD_BIN) $(TEST_BIN)

.PHONY: all build test install uninstall clean
