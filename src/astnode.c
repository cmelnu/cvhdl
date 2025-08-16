#include "astnode.h"
#include <stdlib.h>

// Create a new AST node
ASTNode* create_node(NodeType type) {

    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));

    if (!node) {
        perror("Failed to allocate memory for AST node");
        exit(EXIT_FAILURE);
    }
    
    node->type = type;
    node->value = NULL;
    node->parent = NULL;
    node->children = NULL;
    node->num_children = 0;
    node->capacity = 0;
    
    return node;
}

// Free an AST node and all its children
void free_node(ASTNode *node) {

    if (!node) return;
    
    for (int i = 0; i < node->num_children; i++) {
        free_node(node->children[i]);
    }
    
    free(node->children);
    free(node->value);
    free(node);
}

// Add a child node
void add_child(ASTNode *parent, ASTNode *child) {

    if (!parent->children) {
        parent->capacity = 4;  // Start with space for 4 children
        parent->children = (ASTNode**)malloc(parent->capacity * sizeof(ASTNode*));
        if (!parent->children) {
            perror("Failed to allocate memory for child nodes");
            exit(EXIT_FAILURE);
        }
    }
    
    if (parent->num_children >= parent->capacity) {
        parent->capacity *= 2;
        parent->children = (ASTNode**)realloc(parent->children, 
                                             parent->capacity * sizeof(ASTNode*));
        if (!parent->children) {
            perror("Failed to reallocate memory for child nodes");
            exit(EXIT_FAILURE);
        }
    }
    
    parent->children[parent->num_children++] = child;
    child->parent = parent;
}