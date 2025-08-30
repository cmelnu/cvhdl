#include <string.h>
#include <ctype.h>
#include "symbol_arrays.h"

ArrayInfo g_arrays[128];
int g_array_count = 0;

int find_array_size(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < g_array_count; i++) {
        if (strcmp(g_arrays[i].name, name) == 0) return g_arrays[i].size;
    }
    return -1;
}

void register_array(const char *name, int size) {
    if (!name || size <= 0) return;
    if (g_array_count >= (int)(sizeof(g_arrays) / sizeof(g_arrays[0]))) return;
    // Prevent duplicates; update size if already present
    for (int i = 0; i < g_array_count; i++) {
        if (strcmp(g_arrays[i].name, name) == 0) {
            g_arrays[i].size = size;
            return;
        }
    }
    strncpy(g_arrays[g_array_count].name, name, sizeof(g_arrays[g_array_count].name) - 1);
    g_arrays[g_array_count].name[sizeof(g_arrays[g_array_count].name) - 1] = '\0';
    g_arrays[g_array_count].size = size;
    g_array_count++;
}
