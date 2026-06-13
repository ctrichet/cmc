#ifndef SCAN_H
#define SCAN_H

#include "args.h"

typedef struct {
    char **paths;
    size_t count;
    size_t capacity;
} path_list;

int path_list_init(path_list *pl);
int path_list_add(path_list *pl, const char *path);
void path_list_sort(path_list *pl);
void path_list_free(path_list *pl);

int scan_paths(config *cfg, path_list *pl);

#endif
