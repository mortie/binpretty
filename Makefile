DESTDIR ?=
PREFIX ?= /usr/local
CFLAGS := -O2 -Wall -Wextra -pedantic $(CFLAGS)

binpretty: binpretty.c

.PHONY: clean
clean:
	rm -f binpretty

.PHONY: install
install: binpretty
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f binpretty $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/binpretty
