#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} buffer;

int buf_init(buffer *b);
int buf_append(buffer *b, const char *data, size_t len);
int buf_append_str(buffer *b, const char *s);
int buf_read_file(buffer *b, const char *path);
void buf_free(buffer *b);

#endif
