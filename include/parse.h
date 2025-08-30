#ifndef PARSE_H
#define PARSE_H

#include "astnode.h"

// Parsing interface (monolithic for now; will be split further)

// Forward declarations still needed locally
ASTNode* parse_program(FILE *input);

// Other parsing entry points are in their own headers now
#include "parse_struct.h"
#include "parse_function.h"
#include "parse_statement.h"
#include "parse_expression.h"

ASTNode* create_node(NodeType type);
void add_child(ASTNode *parent, ASTNode *child);
void free_node(ASTNode *node);
void generate_vhdl(ASTNode* node, FILE* output);
void print_ast(ASTNode* node, int level);
// Expression helpers now in parse_expression.h

// Helper for VHDL codegen (still provided by utils)
int is_negative_literal(const char* value);

#endif