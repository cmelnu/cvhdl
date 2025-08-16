#ifndef PARSE_H
#define PARSE_H

#include "token.h"

// AST Node Types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_VAR_DECL,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_BINARY_EXPR,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
    NODE_IF_STATEMENT,
    NODE_ELSE_IF_STATEMENT,
    NODE_ELSE_STATEMENT
} NodeType;


// AST Node structure
typedef struct ASTNode {
    NodeType type;
    Token token;               // Original token (if applicable)
    char *value;               // Value or additional info
    struct ASTNode *parent;    // Parent node
    struct ASTNode **children; // Child nodes
    int num_children;          // Number of children
    int capacity;              // Capacity of children array
} ASTNode;


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
static ASTNode* parse_expression_prec(FILE *input, int min_prec);
static ASTNode* parse_primary(FILE *input);

// Helper for VHDL codegen
int is_negative_literal(const char* value);

#endif