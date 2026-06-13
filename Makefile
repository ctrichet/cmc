CC      = gcc
VERSION ?= 1.0.0
CFLAGS  = -std=c17 -Wall -Wextra -Werror -O2 -D_DEFAULT_SOURCE -DVERSION=\"$(VERSION)\"
LDFLAGS = -lmagic
SRCDIR  = src
OBJDIR  = obj
TARGET  = cmc
DEB_HOST_MULTIARCH := $(shell dpkg-architecture -qDEB_HOST_MULTIARCH 2>/dev/null || echo x86_64-linux-gnu)

# If libmagic-dev is not installed system-wide, set MAGIC_DIR to the
# extracted libmagic-dev package path, e.g.:
#   make MAGIC_DIR=/tmp/libmagic-dev/usr
ifdef MAGIC_DIR
CFLAGS  += -I$(MAGIC_DIR)/include
LDFLAGS  = -L$(MAGIC_DIR)/lib/$(DEB_HOST_MULTIARCH) -lmagic -Wl,-rpath,$(MAGIC_DIR)/lib/$(DEB_HOST_MULTIARCH)
endif

SRCS = $(SRCDIR)/main.c $(SRCDIR)/args.c $(SRCDIR)/scan.c \
       $(SRCDIR)/filters.c $(SRCDIR)/output.c $(SRCDIR)/clipboard.c \
       $(SRCDIR)/buffer.c
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all clean install check

all: $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJDIR) $(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)/usr/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/bin/cmc

check: $(TARGET)
	@cd tests && for t in *.sh; do echo "Running $$t..."; bash "$$t" || exit 1; done
	@echo "All tests passed."
