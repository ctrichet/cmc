#ifndef UTIL_H
#define UTIL_H

static inline const char *strip_dot_slash(const char *path)
{
    while (path[0] == '.' && path[1] == '/')
        path += 2;
    return path;
}

#endif
