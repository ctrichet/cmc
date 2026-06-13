#include "args.h"
#include <stdio.h>

#ifndef VERSION
#define VERSION "unknown"
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

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

static void print_help(void)
{
    printf("Usage: cmc [OPTIONS] [PATHS...]\n");
    printf("Copy file contents to clipboard, file, or stdout.\n\n");
    printf("Options:\n");
    printf("  -R, --recursive         Recursively scan directories\n");
    printf("  -e, --exclude PATTERN   Exclude files matching a glob pattern\n");
    printf("  -E, --excludes PATTERN  Exclude files matching a glob (also loads $XDG_CONFIG_HOME/cmc/.cmc_excludes)\n");
    printf("  -o, --output FILE       Write output to FILE\n");
    printf("  -c, --clipboard         Copy output to system clipboard\n");
    printf("  -s, --symlinks          Follow symbolic links\n");
    printf("  -p, --paths             Prepend each file with its relative path\n");
    printf("  -b, --binary            Include binary files\n");
    printf("  -h, --help              Display this help\n");
    printf("  -v, --version           Display version information\n");
}

static int handle_short_opts(const char *s, int *i, int argc, const char *argv[],
                             config *cfg, bool *exclude_mode, size_t *exc_cap)
{
    for (int j = 1; s[j]; j++) {
        switch (s[j]) {
        case 'R':
            cfg->recursive = true;
            break;
        case 'e':
            *exclude_mode = true;
            if (s[j + 1]) {
                if (add_pattern(&cfg->exclusion_patterns,
                                &cfg->n_exclusion_patterns,
                                exc_cap, s + j + 1) != 0)
                    return -1;
                j = (int)strlen(s) - 1;
            } else if (*i + 1 < argc) {
                (*i)++;
                if (add_pattern(&cfg->exclusion_patterns,
                                &cfg->n_exclusion_patterns,
                                exc_cap, argv[*i]) != 0)
                    return -1;
            } else {
                fprintf(stderr, "cmc: option '-e' requires an argument\n");
                return -2;
            }
            break;
        case 'E':
            cfg->exclude_flag = true;
            *exclude_mode = true;
            if (s[j + 1]) {
                if (add_pattern(&cfg->exclusion_patterns,
                                &cfg->n_exclusion_patterns,
                                exc_cap, s + j + 1) != 0)
                    return -1;
                j = (int)strlen(s) - 1;
            } else if (*i + 1 < argc) {
                (*i)++;
                if (add_pattern(&cfg->exclusion_patterns,
                                &cfg->n_exclusion_patterns,
                                exc_cap, argv[*i]) != 0)
                    return -1;
            } else {
                fprintf(stderr, "cmc: option '-E' requires an argument\n");
                return -2;
            }
            break;
        case 'o':
            if (s[j + 1]) {
                cfg->output_file = s + j + 1;
                j = (int)strlen(s) - 1;
            } else if (*i + 1 < argc) {
                (*i)++;
                cfg->output_file = argv[*i];
            } else {
                fprintf(stderr, "cmc: option '-o' requires an argument\n");
                return -2;
            }
            break;
        case 'c': cfg->clipboard = true; break;
        case 's': cfg->follow_symlinks = true; break;
        case 'p': cfg->path_mode = true; break;
        case 'b': cfg->binary_mode = true; break;
        case 'h': print_help(); exit(0);
        case 'v': printf("cmc version %s\n", VERSION); exit(0);
        default:
            fprintf(stderr, "cmc: unknown option '-%c'\n", s[j]);
            fprintf(stderr, "Try 'cmc --help' for more information.\n");
            return -2;
        }
    }
    return 0;
}

static void load_cmc_excludes(config *cfg)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    char path[4096];
    if (xdg && xdg[0] != '\0') {
        snprintf(path, sizeof(path), "%s/cmc/.cmc_excludes", xdg);
    } else {
        const char *home = getenv("HOME");
        if (!home) {
            fprintf(stderr, "cmc: warning: $HOME is not set, skipping .cmc_excludes\n");
            return;
        }
        size_t home_len = strlen(home);
        while (home_len > 0 && home[home_len - 1] == '/')
            home_len--;
        snprintf(path, sizeof(path), "%.*s/.config/cmc/.cmc_excludes",
                 (int)home_len, home);
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cmc: warning: no .cmc_excludes at %s\n", path);
        return;
    }
    char line[4096];
    size_t exc_cap = 0;
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0 || line[0] == '#')
            continue;
        if (add_pattern(&cfg->exclusion_patterns,
                        &cfg->n_exclusion_patterns,
                        &exc_cap, line) != 0) {
            fprintf(stderr, "cmc: memory allocation failed\n");
            break;
        }
    }
    if (ferror(f))
        fprintf(stderr, "cmc: error reading %s\n", path);
    fclose(f);
}

int parse_args(int argc, char *argv[], config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));

    size_t sel_cap = 0, exc_cap = 0;
    bool exclude_mode = false;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "--") == 0) {
            i++;
            while (i < argc) {
                if (exclude_mode) {
                    if (add_pattern(&cfg->exclusion_patterns,
                                    &cfg->n_exclusion_patterns,
                                    &exc_cap, argv[i]) != 0) {
                        fprintf(stderr, "cmc: memory allocation failed\n");
                        return 1;
                    }
                } else {
                    if (add_pattern(&cfg->selection_patterns,
                                    &cfg->n_selection_patterns,
                                    &sel_cap, argv[i]) != 0) {
                        fprintf(stderr, "cmc: memory allocation failed\n");
                        return 1;
                    }
                }
                i++;
            }
            break;
        }

        if (arg[0] == '-' && arg[1] != '\0') {
            if (arg[1] == '-') {
                const char *opt = arg + 2;

                if (strcmp(opt, "recursive") == 0) {
                    cfg->recursive = true;
                } else if (strncmp(opt, "exclude=", 8) == 0) {
                    exclude_mode = true;
                    if (add_pattern(&cfg->exclusion_patterns,
                                    &cfg->n_exclusion_patterns,
                                    &exc_cap, opt + 8) != 0) {
                        fprintf(stderr, "cmc: memory allocation failed\n");
                        return 1;
                    }
                } else if (strcmp(opt, "exclude") == 0) {
                    exclude_mode = true;
                    if (i + 1 < argc) {
                        i++;
                        if (add_pattern(&cfg->exclusion_patterns,
                                        &cfg->n_exclusion_patterns,
                                        &exc_cap, argv[i]) != 0) {
                            fprintf(stderr, "cmc: memory allocation failed\n");
                            return 1;
                        }
                    } else {
                        fprintf(stderr, "cmc: option '--exclude' requires an argument\n");
                        return 2;
                    }
                } else if (strncmp(opt, "excludes=", 9) == 0) {
                    cfg->exclude_flag = true;
                    exclude_mode = true;
                    if (add_pattern(&cfg->exclusion_patterns,
                                    &cfg->n_exclusion_patterns,
                                    &exc_cap, opt + 9) != 0) {
                        fprintf(stderr, "cmc: memory allocation failed\n");
                        return 1;
                    }
                } else if (strcmp(opt, "excludes") == 0) {
                    cfg->exclude_flag = true;
                    exclude_mode = true;
                    if (i + 1 < argc) {
                        i++;
                        if (add_pattern(&cfg->exclusion_patterns,
                                        &cfg->n_exclusion_patterns,
                                        &exc_cap, argv[i]) != 0) {
                            fprintf(stderr, "cmc: memory allocation failed\n");
                            return 1;
                        }
                    } else {
                        fprintf(stderr, "cmc: option '--excludes' requires an argument\n");
                        return 2;
                    }
                } else if (strncmp(opt, "output=", 7) == 0) {
                    cfg->output_file = opt + 7;
                } else if (strcmp(opt, "output") == 0) {
                    if (i + 1 < argc) {
                        i++;
                        cfg->output_file = argv[i];
                    } else {
                        fprintf(stderr, "cmc: option '--output' requires an argument\n");
                        return 2;
                    }
                } else if (strcmp(opt, "clipboard") == 0) {
                    cfg->clipboard = true;
                } else if (strcmp(opt, "symlinks") == 0) {
                    cfg->follow_symlinks = true;
                } else if (strcmp(opt, "paths") == 0) {
                    cfg->path_mode = true;
                } else if (strcmp(opt, "binary") == 0) {
                    cfg->binary_mode = true;
                } else if (strcmp(opt, "help") == 0) {
                    print_help();
                    exit(0);
                } else if (strcmp(opt, "version") == 0) {
                    printf("cmc version %s\n", VERSION);
                    exit(0);
                } else {
                    fprintf(stderr, "cmc: unknown option '%s'\n", arg);
                    fprintf(stderr, "Try 'cmc --help' for more information.\n");
                    return 2;
                }
            } else {
                int ret = handle_short_opts(arg, &i, argc, (const char **)argv, cfg,
                                            &exclude_mode, &exc_cap);
                if (ret == -1) {
                    fprintf(stderr, "cmc: memory allocation failed\n");
                    return 1;
                } else if (ret == -2) {
                    return 2;
                }
            }
        } else {
            if (exclude_mode) {
                if (add_pattern(&cfg->exclusion_patterns,
                                &cfg->n_exclusion_patterns,
                                &exc_cap, arg) != 0) {
                    fprintf(stderr, "cmc: memory allocation failed\n");
                    return 1;
                }
            } else {
                if (add_pattern(&cfg->selection_patterns,
                                &cfg->n_selection_patterns,
                                &sel_cap, arg) != 0) {
                    fprintf(stderr, "cmc: memory allocation failed\n");
                    return 1;
                }
            }
        }
    }

    if (cfg->exclude_flag)
        load_cmc_excludes(cfg);

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
