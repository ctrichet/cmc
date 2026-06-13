#define _XOPEN_SOURCE 500

#include "scan.h"
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#define PATH_CAP 1024

static path_list *g_pl;

int path_list_init(path_list *pl)
{
    pl->paths = NULL;
    pl->count = 0;
    pl->capacity = 0;
    return 0;
}

int path_list_add(path_list *pl, const char *path)
{
    if (pl->count >= pl->capacity) {
        size_t new_cap = pl->capacity ? pl->capacity * 2 : PATH_CAP;
        char **tmp = realloc(pl->paths, new_cap * sizeof(char *));
        if (!tmp) return -1;
        pl->paths = tmp;
        pl->capacity = new_cap;
    }
    pl->paths[pl->count] = strdup(path);
    if (!pl->paths[pl->count]) return -1;
    pl->count++;
    return 0;
}

static int cmp_str(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void path_list_sort(path_list *pl)
{
    qsort(pl->paths, pl->count, sizeof(char *), cmp_str);
}

void path_list_free(path_list *pl)
{
    for (size_t i = 0; i < pl->count; i++)
        free(pl->paths[i]);
    free(pl->paths);
    pl->paths = NULL;
    pl->count = 0;
    pl->capacity = 0;
}

static int nftw_cb(const char *fpath, const struct stat *sb,
                    int typeflag, struct FTW *ftwbuf)
{
    (void)sb;
    (void)ftwbuf;
    if (typeflag == FTW_D || typeflag == FTW_DP)
        return 0;

    if (typeflag == FTW_SL || typeflag == FTW_NS)
        return 0;

    if (path_list_add(g_pl, fpath) != 0)
        return -1;

    return 0;
}

static int scan_recursive(path_list *pl, const char *dir, bool follow_symlinks)
{
    g_pl = pl;
    int flags = follow_symlinks ? 0 : FTW_PHYS;
    return nftw(dir, nftw_cb, 64, flags);
}

static int scan_flat(path_list *pl, const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        fprintf(stderr, "cmc: cannot access '%s': %s\n", path, strerror(errno));
        return -1;
    }

    if (S_ISREG(st.st_mode)) {
        return path_list_add(pl, path);
    }

    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(path);
        if (!d) {
            fprintf(stderr, "cmc: cannot open directory '%s': %s\n", path, strerror(errno));
            return -1;
        }
        struct dirent *entry;
        size_t path_len = strlen(path);
        while ((entry = readdir(d)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;
            size_t needed = path_len + 1 + strlen(entry->d_name) + 1;
            char *full = malloc(needed);
            if (!full) {
                closedir(d);
                return -1;
            }
            snprintf(full, needed, "%s/%s", path, entry->d_name);
            struct stat st2;
            if (stat(full, &st2) == 0 && S_ISREG(st2.st_mode)) {
                path_list_add(pl, full);
            }
            free(full);
        }
        closedir(d);
        return 0;
    }

    fprintf(stderr, "cmc: warning: '%s' is not a regular file or directory\n", path);
    return 0;
}

int scan_paths(config *cfg, path_list *pl)
{
    if (cfg->n_selection_patterns == 0) {
        return scan_recursive(pl, ".", cfg->follow_symlinks);
    }

    int had_error = 0;
    for (size_t i = 0; i < cfg->n_selection_patterns; i++) {
        struct stat st;
        if (stat(cfg->selection_patterns[i], &st) != 0) {
            fprintf(stderr, "cmc: error: '%s' does not exist\n",
                    cfg->selection_patterns[i]);
            had_error = 1;
            continue;
        }

        if (S_ISREG(st.st_mode)) {
            path_list_add(pl, cfg->selection_patterns[i]);
        } else if (S_ISDIR(st.st_mode)) {
            if (cfg->recursive) {
                scan_recursive(pl, cfg->selection_patterns[i], cfg->follow_symlinks);
            } else {
                scan_flat(pl, cfg->selection_patterns[i]);
            }
        } else {
            fprintf(stderr, "cmc: warning: '%s' is not a regular file or directory\n",
                    cfg->selection_patterns[i]);
        }
    }
    return had_error;
}
