#include "args.h"
#include "scan.h"
#include "filters.h"
#include "output.h"
#include "clipboard.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char *strip_dot_slash(const char *path)
{
    while (path[0] == '.' && path[1] == '/')
        path += 2;
    return path;
}

int main(int argc, char *argv[])
{
    config cfg;
    int ret = parse_args(argc, argv, &cfg);
    if (ret != 0) {
        free_config(&cfg);
        return ret == 6 ? 6 : (ret == 2 ? 2 : 1);
    }

    magic_ctx mctx;
    if (magic_ctx_init(&mctx) != 0) {
        free_config(&cfg);
        return 5;
    }

    if (cfg.clipboard && cfg.output_file) {
        fprintf(stderr, "cmc: -c and -o cannot be used together\n");
        magic_ctx_free(&mctx);
        free_config(&cfg);
        return 2;
    }

    path_list pl;
    path_list_init(&pl);

    int scan_err = scan_paths(&cfg, &pl);
    path_list_sort(&pl);

    buffer out;
    buf_init(&out);

    int include_error = 0;

    for (size_t i = 0; i < pl.count; i++) {
        const char *path = pl.paths[i];

        if (!should_include(path, &cfg, &mctx))
            continue;

        FILE *f = fopen(path, "rb");
        if (!f) {
            fprintf(stderr, "cmc: warning: cannot read '%s': %s\n",
                    path, strerror(errno));
            continue;
        }

        if (cfg.path_mode) {
            const char *rel = strip_dot_slash(path);
            buf_append_str(&out, "### FILE: ");
            buf_append_str(&out, rel);
            buf_append_str(&out, " ###\n\n");
        }

        char chunk[8192];
        size_t n;
        while ((n = fread(chunk, 1, sizeof(chunk), f)) > 0) {
            if (buf_append(&out, chunk, n) != 0) {
                fprintf(stderr, "cmc: memory allocation failed\n");
                fclose(f);
                include_error = 1;
                goto cleanup;
            }
        }
        if (ferror(f)) {
            fprintf(stderr, "cmc: warning: error reading '%s': %s\n",
                    path, strerror(errno));
        }
        fclose(f);

        if (cfg.path_mode)
            buf_append_str(&out, "\n");
    }

    if (cfg.clipboard) {
        ret = copy_to_clipboard(&out);
    } else {
        ret = write_output(&out, &cfg);
    }

cleanup:
    buf_free(&out);
    path_list_free(&pl);
    magic_ctx_free(&mctx);
    free_config(&cfg);

    if (include_error) return 1;
    if (scan_err) return 1;
    return ret;
}
