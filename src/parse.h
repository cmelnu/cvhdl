#ifndef PARSE_H
#define PARSE_H

#include "astnode.h"

// Parsing interface (monolithic for now; will be split further)

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

// Helper for VHDL codegen (still provided by utils)
int is_negative_literal(const char* value);

#endif