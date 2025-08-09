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
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP
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
ASTNode* parse_term(FILE *input);
ASTNode* parse_factor(FILE *input);

ASTNode* create_node(NodeType type);
void add_child(ASTNode *parent, ASTNode *child);
void free_node(ASTNode *node);
void generate_vhdl(ASTNode* node, FILE* output);
void print_ast(ASTNode* node, int level);

#endif