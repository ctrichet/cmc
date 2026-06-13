#include "args.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PAT_CAP 64

static int add_pattern(char ***patterns, size_t *n, size_t *cap, const char *pat)
{
    if (*n >= *cap) {
        size_t new_cap = *cap ? *cap * 2 : PAT_CAP;
        char **tmp = realloc(*patterns, new_cap * sizeof(char *));
        if (!tmp) return -1;
        *patterns = tmp;
        *cap = new_cap;
    }
    (*patterns)[*n] = strdup(pat);
    if (!(*patterns)[*n]) return -1;
    (*n)++;
    return 0;
}

static int load_exclusion_file(config *cfg, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cmc: cannot read exclusion file '%s': %s\n", path, strerror(errno));
        return -1;
    }
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0) continue;
        if (add_pattern(&cfg->exclusion_patterns,
                        &cfg->n_exclusion_patterns,
                        &(size_t){0}, line) != 0) {
            fclose(f);
            return -1;
        }
    }
    if (ferror(f)) {
        fprintf(stderr, "cmc: error reading exclusion file '%s': %s\n", path, strerror(errno));
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

int parse_args(int argc, char *argv[], config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));

    static const char *short_opts = "Re:E:o:cspbhv";
    static const struct option long_opts[] = {
        {"recursive",    no_argument,       0, 'R'},
        {"exclude",      required_argument, 0, 'e'},
        {"exclude-file", required_argument, 0, 'E'},
        {"output",       required_argument, 0, 'o'},
        {"clipboard",    no_argument,       0, 'c'},
        {"symlinks",     no_argument,       0, 's'},
        {"paths",        no_argument,       0, 'p'},
        {"binary",       no_argument,       0, 'b'},
        {"help",         no_argument,       0, 'h'},
        {"version",      no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    size_t sel_cap = 0, exc_cap = 0;
    int ch;
    int opt_ind = 0;

    while ((ch = getopt_long(argc, argv, short_opts, long_opts, &opt_ind)) != -1) {
        switch (ch) {
        case 'R': cfg->recursive = true; break;
        case 'e':
            if (add_pattern(&cfg->exclusion_patterns,
                            &cfg->n_exclusion_patterns,
                            &exc_cap, optarg) != 0) {
                fprintf(stderr, "cmc: memory allocation failed\n");
                return 1;
            }
            break;
        case 'E':
            if (load_exclusion_file(cfg, optarg) != 0)
                return 6;
            break;
        case 'o': cfg->output_file = optarg; break;
        case 'c': cfg->clipboard = true; break;
        case 's': cfg->follow_symlinks = true; break;
        case 'p': cfg->path_mode = true; break;
        case 'b': cfg->binary_mode = true; break;
        case 'h':
            printf("Usage: cmc [OPTIONS] [PATHS...]\n");
            printf("Copy file contents to clipboard, file, or stdout.\n\n");
            printf("Options:\n");
            printf("  -R, --recursive         Recursively scan directories\n");
            printf("  -e, --exclude PATTERN   Exclude files matching a glob pattern\n");
            printf("  -E, --exclude-file FILE Load exclusion patterns from a file\n");
            printf("  -o, --output FILE       Write output to FILE\n");
            printf("  -c, --clipboard         Copy output to system clipboard\n");
            printf("  -s, --symlinks          Follow symbolic links\n");
            printf("  -p, --paths             Prepend each file with its relative path\n");
            printf("  -b, --binary            Include binary files\n");
            printf("  -h, --help              Display this help\n");
            printf("  -v, --version           Display version information\n");
            exit(0);
        case 'v':
            printf("cmc version 1.0.0\n");
            exit(0);
        default:
            fprintf(stderr, "Try 'cmc --help' for more information.\n");
            exit(2);
        }
    }

    argc -= optind;
    argv += optind;

    for (int i = 0; i < argc; i++) {
        if (add_pattern(&cfg->selection_patterns,
                        &cfg->n_selection_patterns,
                        &sel_cap, argv[i]) != 0) {
            fprintf(stderr, "cmc: memory allocation failed\n");
            return 1;
        }
    }

    return 0;
}

void free_config(config *cfg)
{
    for (size_t i = 0; i < cfg->n_selection_patterns; i++)
        free(cfg->selection_patterns[i]);
    free(cfg->selection_patterns);
    for (size_t i = 0; i < cfg->n_exclusion_patterns; i++)
        free(cfg->exclusion_patterns[i]);
    free(cfg->exclusion_patterns);
    memset(cfg, 0, sizeof(*cfg));
}
