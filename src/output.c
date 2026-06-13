#include "output.h"
#include "exitcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int write_output(buffer *out, config *cfg)
{
    if (cfg->output_file) {
        FILE *f = fopen(cfg->output_file, "w");
        if (!f) {
            fprintf(stderr, "cmc: cannot open '%s' for writing: %s\n",
                    cfg->output_file, strerror(errno));
            return EXIT_OUTPUT;
        }
        if (fwrite(out->data, 1, out->len, f) != out->len) {
            fprintf(stderr, "cmc: error writing to '%s': %s\n",
                    cfg->output_file, strerror(errno));
            fclose(f);
            return EXIT_OUTPUT;
        }
        if (fclose(f) != 0) {
            fprintf(stderr, "cmc: warning: error closing '%s': %s\n",
                    cfg->output_file, strerror(errno));
        }
    } else {
        if (fwrite(out->data, 1, out->len, stdout) != out->len) {
            fprintf(stderr, "cmc: error writing to stdout: %s\n",
                    strerror(errno));
            return EXIT_OUTPUT;
        }
        if (fflush(stdout) != 0) {
            fprintf(stderr, "cmc: error flushing stdout: %s\n", strerror(errno));
            return EXIT_OUTPUT;
        }
    }
    return EXIT_OK;
}
