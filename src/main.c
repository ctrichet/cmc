#include "args.h"
#include "scan.h"
#include "filters.h"
#include "output.h"
#include "clipboard.h"
#include "buffer.h"
#include "exitcodes.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    config cfg;
    int ret = parse_args(argc, argv, &cfg);
    if (ret != 0) {
        free_config(&cfg);
        return ret;
    }

    magic_ctx mctx;
    if (magic_ctx_init(&mctx) != 0) {
        free_config(&cfg);
        return EXIT_MAGIC;
    }

    if (cfg.clipboard && cfg.output_file) {
        fprintf(stderr, "cmc: -c and -o cannot be used together\n");
        magic_ctx_free(&mctx);
        free_config(&cfg);
        return EXIT_BADARGS;
    }

    path_list pl;
    path_list_init(&pl);

    int scan_err = scan_paths(&cfg, &pl);
    path_list_sort(&pl);

    buffer out;
    if (buf_init(&out) != 0) {
        fprintf(stderr, "cmc: memory allocation failed\n");
        path_list_free(&pl);
        magic_ctx_free(&mctx);
        free_config(&cfg);
        return EXIT_ERROR;
    }

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
        if (fclose(f) != 0) {
            fprintf(stderr, "cmc: warning: error closing '%s': %s\n",
                    path, strerror(errno));
        }

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

    if (include_error) return EXIT_ERROR;
    if (ret) return ret;
    if (scan_err) return EXIT_ERROR;
    return EXIT_OK;
}
