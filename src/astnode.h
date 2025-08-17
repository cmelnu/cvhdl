#ifndef ASTNODE_H
#define ASTNODE_H

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
    NODE_ELSE_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_BREAK_STATEMENT,
    NODE_CONTINUE_STATEMENT
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


// Missing function declarations for AST node management
ASTNode* create_node(NodeType type);
void free_node(ASTNode* node);
void add_child(ASTNode* parent, ASTNode* child);

#endif // ASTNODE_H