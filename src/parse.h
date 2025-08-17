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

#endif