#include <string.h>
#include "symbol_structs.h"

// Definition of global struct table
StructInfo g_structs[64];
int g_struct_count = 0;

int find_struct_index(const char *name) {
    int i = 0;

    if (!name) {
        return -1;
    }

    for (i = 0; i < g_struct_count; i++) {
        if (strcmp(g_structs[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

const char* struct_field_type(const char *struct_name, const char *field_name) {
    int idx = -1;
    int f = 0;

    if (!struct_name || !field_name) {
        return NULL;
    }

    idx = find_struct_index(struct_name);

    if (idx < 0) {
        return NULL;
    }

    for (f = 0; f < g_structs[idx].field_count; f++) {
        if (strcmp(g_structs[idx].fields[f].field_name, field_name) == 0) {
            return g_structs[idx].fields[f].field_type;
        }
    }

    return NULL;
}
