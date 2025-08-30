#ifndef PARSE_H
#define PARSE_H

#include "astnode.h"

typedef struct {
    char name[64];
    int size; // number of elements
} ArrayInfo;

// Forward declarations for recursive descent parsing
ASTNode* parse_program(FILE *input);
ASTNode* parse_function(FILE *input, Token return_type, Token func_name);
ASTNode* parse_struct(FILE *input, Token struct_token);
ASTNode* parse_statement(FILE *input);
ASTNode* parse_expression(FILE *input);

ASTNode* create_node(NodeType type);
void add_child(ASTNode *parent, ASTNode *child);
void free_node(ASTNode *node);
void generate_vhdl(ASTNode* node, FILE* output);
void print_ast(ASTNode* node, int level);
ASTNode* parse_expression_prec(FILE *input, int min_prec);
ASTNode* parse_primary(FILE *input);

// Helper for VHDL codegen
int is_negative_literal(const char* value);

// Struct metadata helpers
typedef struct {
    char name[64];
    struct { char field_name[64]; char field_type[32]; } fields[32];
    int field_count;
} StructInfo;

extern StructInfo g_structs[64];
extern int g_struct_count;
int find_struct_index(const char *name);
const char* struct_field_type(const char *struct_name, const char *field_name);

#endif