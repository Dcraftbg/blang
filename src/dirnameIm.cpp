#pragma once
#include <stdio.h>
#include <string.h>
char* dirname(char* path) {
    static char dir[FILENAME_MAX];
    char* last_slash = strrchr(path, '\\');
    if (last_slash == NULL) {
        last_slash = strrchr(path, '/');
    }
    if (last_slash == NULL) {
        strcpy_s(dir, ".");
    }
    else if (last_slash == path) {
        strcpy_s(dir, "/");
    }
    else {
        size_t len = last_slash - path;
        strncpy_s(dir, path, len);
        dir[len] = '\0';
    }
    return dir;
}
char* basename(char* path) {
    char* base = strrchr(path, '/');
    if (base == NULL) {
        base = strrchr(path, '\\');
    }
    if (base == NULL) {
        return path;
    }
    return base + 1;
}