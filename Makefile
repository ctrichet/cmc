CC      = gcc
CFLAGS  = -std=c17 -Wall -Wextra -Werror -O2 -D_DEFAULT_SOURCE
LDFLAGS = -lmagic
SRCDIR  = src
OBJDIR  = obj
TARGET  = cmc

# If libmagic-dev is not installed system-wide, set MAGIC_DIR to the
# extracted libmagic-dev package path, e.g.:
#   make MAGIC_DIR=/tmp/libmagic-dev/usr
ifdef MAGIC_DIR
CFLAGS  += -I$(MAGIC_DIR)/include
LDFLAGS  = -L$(MAGIC_DIR)/lib/x86_64-linux-gnu -lmagic -Wl,-rpath,$(MAGIC_DIR)/lib/x86_64-linux-gnu
endif

SRCS = $(SRCDIR)/main.c $(SRCDIR)/args.c $(SRCDIR)/scan.c \
       $(SRCDIR)/filters.c $(SRCDIR)/output.c $(SRCDIR)/clipboard.c \
       $(SRCDIR)/buffer.c
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all clean install

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
