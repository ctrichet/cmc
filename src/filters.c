#include "filters.h"
#include <fnmatch.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int magic_ctx_init(magic_ctx *ctx)
{
    ctx->cookie = magic_open(MAGIC_MIME_TYPE);
    if (!ctx->cookie) {
        fprintf(stderr, "cmc: failed to initialize libmagic\n");
        ctx->initialized = 0;
        return -1;
    }
    if (magic_load(ctx->cookie, NULL) != 0) {
        fprintf(stderr, "cmc: failed to load magic database: %s\n",
                magic_error(ctx->cookie));
        magic_close(ctx->cookie);
        ctx->initialized = 0;
        return -1;
    }
    ctx->initialized = 1;
    return 0;
}

int is_binary(magic_ctx *ctx, const char *path)
{
    if (!ctx->initialized) return 0;
    const char *mime = magic_file(ctx->cookie, path);
    if (!mime) return 0;
    return strncmp(mime, "text/", 5) != 0;
}

void magic_ctx_free(magic_ctx *ctx)
{
    if (ctx->initialized)
        magic_close(ctx->cookie);
    ctx->initialized = 0;
}

static const char *file_basename(const char *path)
{
    const char *base = strrchr(path, '/');
    return base ? base + 1 : path;
}

static int path_under_dir(const char *path, const char *dir)
{
    size_t dlen = strlen(dir);
    if (strncmp(path, dir, dlen) == 0) {
        if (path[dlen] == '/' || path[dlen] == '\0')
            return 1;
    }
    return 0;
}

static int matches_selection(const char *path, config *cfg)
{
    for (size_t i = 0; i < cfg->n_selection_patterns; i++) {
        const char *pat = cfg->selection_patterns[i];
        if (fnmatch(pat, path, FNM_PATHNAME) == 0)
            return 1;
        if (fnmatch(pat, file_basename(path), 0) == 0)
            return 1;
        if (path_under_dir(path, pat))
            return 1;
    }
    return 0;
}

static int matches_exclusion(const char *path, config *cfg)
{
    for (size_t i = 0; i < cfg->n_exclusion_patterns; i++) {
        const char *pat = cfg->exclusion_patterns[i];
        if (fnmatch(pat, path, FNM_PATHNAME) == 0)
            return 1;
        if (fnmatch(pat, file_basename(path), 0) == 0)
            return 1;
    }
    return 0;
}

int should_include(const char *path, config *cfg, magic_ctx *ctx)
{
    const char *rel = path;
    while (rel[0] == '.' && rel[1] == '/') rel += 2;

    if (cfg->n_selection_patterns > 0) {
        if (!matches_selection(rel, cfg) && !matches_selection(path, cfg))
            return 0;
    }

    if (matches_exclusion(rel, cfg) || matches_exclusion(path, cfg))
        return 0;

    if (!cfg->binary_mode && ctx && ctx->initialized) {
        if (is_binary(ctx, path))
            return 0;
    }

    return 1;
}
