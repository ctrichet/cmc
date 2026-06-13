#ifndef FILTERS_H
#define FILTERS_H

#include "args.h"
#include <magic.h>

typedef struct {
    magic_t cookie;
    int initialized;
} magic_ctx;

int magic_ctx_init(magic_ctx *ctx);
int is_binary(magic_ctx *ctx, const char *path);
void magic_ctx_free(magic_ctx *ctx);
int should_include(const char *path, config *cfg, magic_ctx *ctx);

#endif
