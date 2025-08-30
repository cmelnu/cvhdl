#ifndef PARSE_H
#define PARSE_H

#include "astnode.h"
#include <stdio.h>   // for FILE
#include <stddef.h>  // for size_t

typedef struct {
    char name[64];
    int size; // number of elements
} ArrayInfo;

// Forward declarations for recursive descent parsing
ASTNode* parse_program(FILE *input);
ASTNode* parse_function(FILE *input, Token return_type, Token func_name);
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

// Exposed helpers (previously internal) for function call annotation and VHDL emission
void build_gen_name(const char *orig, int ordinal, char *out, size_t outsz);
void annotate_func_calls(ASTNode *n, int *counter);
void declare_func_call_signals(ASTNode *n, FILE *output);
void emit_inline_expr(ASTNode *expr, FILE *output);
void emit_func_call_instantiations(ASTNode *n, FILE *output);

#endif