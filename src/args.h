#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool recursive;
    bool follow_symlinks;
    bool path_mode;
    bool binary_mode;
    bool clipboard;
    bool exclude_flag;
    const char *output_file;
    char **selection_patterns;
    size_t n_selection_patterns;
    char **exclusion_patterns;
    size_t n_exclusion_patterns;
} config;

int parse_args(int argc, char *argv[], config *cfg);
void free_config(config *cfg);

#endif
