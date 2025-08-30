#ifndef SYMBOL_ARRAYS_H
#define SYMBOL_ARRAYS_H

// Simple per-function array table info (used for static bounds checking)
typedef struct {
    char name[64];
    int size; // number of elements
} ArrayInfo;

extern ArrayInfo g_arrays[128];
extern int g_array_count;

void register_array(const char *name, int size);
int find_array_size(const char *name);

#endif // SYMBOL_ARRAYS_H
