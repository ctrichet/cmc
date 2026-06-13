#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_INIT_CAP 4096

int buf_init(buffer *b)
{
    b->data = malloc(BUF_INIT_CAP);
    if (!b->data) return -1;
    b->data[0] = '\0';
    b->len = 0;
    b->cap = BUF_INIT_CAP;
    return 0;
}

int buf_append(buffer *b, const char *data, size_t len)
{
    if (len == 0) return 0;
    while (b->len + len + 1 > b->cap) {
        if (b->cap > (size_t)-1 / 2) return -1;
        size_t new_cap = b->cap * 2;
        char *tmp = realloc(b->data, new_cap);
        if (!tmp) return -1;
        b->data = tmp;
        b->cap = new_cap;
    }
    memcpy(b->data + b->len, data, len);
    b->len += len;
    b->data[b->len] = '\0';
    return 0;
}

int buf_append_str(buffer *b, const char *s)
{
    return buf_append(b, s, strlen(s));
}

void buf_free(buffer *b)
{
    free(b->data);
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}
