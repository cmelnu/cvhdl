#ifndef SYMBOL_STRUCTS_H
#define SYMBOL_STRUCTS_H

// Struct metadata description
typedef struct {
    char name[64];
    struct { char field_name[64]; char field_type[32]; } fields[32];
    int field_count;
} StructInfo;

// Global struct table (simple flat array for now)
extern StructInfo g_structs[64];
extern int g_struct_count;

// Lookup helpers
int find_struct_index(const char *name);
const char* struct_field_type(const char *struct_name, const char *field_name);

#endif // SYMBOL_STRUCTS_H
