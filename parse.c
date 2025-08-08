#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

// Current token and functions to manage the token stream
extern Token current_token;

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



// Parse the entire program
ASTNode* parse_program(FILE *input) {

    Token func_name, return_type;
    ASTNode *func_node = NULL;
    ASTNode* program_node = create_node(NODE_PROGRAM);
    
    // Start by getting the first token
    advance(input);
    
    // Keep parsing functions until EOF
    while (!match(TOKEN_EOF)) {

        // Check for function declaration
        if (match(TOKEN_KEYWORD)) {

            // This could be a return type or variable declaration
            return_type = current_token;
            advance(input);
            
            if (match(TOKEN_IDENTIFIER)) {

                func_name = current_token;
                advance(input);
                
                if (match(TOKEN_PARENTHESIS_OPEN)) {
                    // This is likely a function declaration
                    // Don't rewind the file - this causes problems
                    func_node = parse_function(input, return_type, func_name);
                    if (func_node != NULL) {  // Check for NULL before adding
                        add_child(program_node, func_node);
                    }
                } else {
                    // This is likely a global variable declaration
                    printf("Warning: Global variable declarations not yet implemented\n");
                    // Skip until semicolon
                    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) {
                        advance(input);
                    }
                    if (match(TOKEN_SEMICOLON)) advance(input);
                }
            } else {
                // Skip unknown tokens
                printf("Warning: Expected identifier after type\n");
                advance(input);
            }
        } else {
            // Skip unknown tokens
            advance(input);
        }
    }
    
    return program_node;
}


// Parse a function declaration
ASTNode* parse_function(FILE *input, Token return_type, Token func_name) {

    Token param_type, param_name;
    memset(&param_type, 0, sizeof(Token));
    memset(&param_name, 0, sizeof(Token));
    ASTNode *param_node = NULL;
    ASTNode* func_node = create_node(NODE_FUNCTION_DECL);
    
    // Store return type
    func_node->token = return_type;
    
    // We already have the function name
    func_node->value = strdup(func_name.value);
    
    // We're already at the opening parenthesis, consume it
    if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
        printf("Error: Expected '(' after function name\n");
        free_node(func_node);
        return NULL;
    }
    
    // Parse parameter list
    // For now, just consume tokens until closing parenthesis
    while (!match(TOKEN_PARENTHESIS_CLOSE) && !match(TOKEN_EOF)) {
        // Expect a type (keyword)
        if (match(TOKEN_KEYWORD)) {
            param_type = current_token;
            advance(input);

            // Expect an identifier (parameter name)
            if (match(TOKEN_IDENTIFIER)) {
                param_name = current_token;
                advance(input);

                // Create a parameter node and add as child
                param_node = create_node(NODE_VAR_DECL);
                param_node->token = param_type;
                param_node->value = strdup(param_name.value);
                add_child(func_node, param_node);

                // If there's a comma, consume it and continue
                if (match(TOKEN_COMMA)) {
                    advance(input);
                }
            } else {
                printf("Error: Expected parameter name\n");
                break;
            }
        } else {
            // If not a keyword, skip (could be void or error)
            advance(input);
        }
    }
    
    if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
        printf("Error: Expected ')' after parameter list\n");
        free_node(func_node);
        return NULL;
    }
    
    // Expect function body starting with '{'
    if (!consume(input, TOKEN_BRACE_OPEN)) {
        printf("Error: Expected '{' to start function body\n");
        free_node(func_node);
        return NULL;
    }
    
    // Parse statements in function body
    // TODO: Implement full statement parsing
    // For now, just consume tokens until closing brace
    int brace_depth = 1;
    while (brace_depth > 0 && !match(TOKEN_EOF)) {
        if (match(TOKEN_BRACE_OPEN)) brace_depth++;
        else if (match(TOKEN_BRACE_CLOSE)) brace_depth--;
        advance(input);
    }
    
    return func_node;
}

// Generate VHDL code from an AST
void generate_vhdl(ASTNode* node, FILE* output) {
    
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            fprintf(output, "-- VHDL generated by compi\n\n");
            fprintf(output, "library IEEE;\n");
            fprintf(output, "use IEEE.STD_LOGIC_1164.ALL;\n");
            fprintf(output, "use IEEE.NUMERIC_STD.ALL;\n\n");
            
            // Generate code for all children (functions, etc.)
            for (int i = 0; i < node->num_children; i++) {
                generate_vhdl(node->children[i], output);
            }
            break;
            
        case NODE_FUNCTION_DECL:
            fprintf(output, "-- Function: %s\n", node->value);
            fprintf(output, "entity %s is\n", node->value);
            fprintf(output, "  port (\n");
            fprintf(output, "    clk : in std_logic;\n");
            fprintf(output, "    reset : in std_logic;\n");
            // TODO: Add proper ports based on function parameters
            fprintf(output, "    -- Parameters would be converted to ports\n");
            fprintf(output, "    result : out std_logic_vector(31 downto 0)\n");
            fprintf(output, "  );\n");
            fprintf(output, "end entity;\n\n");
            
            fprintf(output, "architecture behavioral of %s is\n", node->value);
            // TODO: Generate signals from local variables
            fprintf(output, "  -- Internal signals would be declared here\n");
            fprintf(output, "begin\n");
            fprintf(output, "  process(clk, reset)\n");
            fprintf(output, "  begin\n");
            fprintf(output, "    if reset = '1' then\n");
            fprintf(output, "      -- Reset logic\n");
            fprintf(output, "    elsif rising_edge(clk) then\n");
            fprintf(output, "      -- Function body would be translated here\n");
            fprintf(output, "    end if;\n");
            fprintf(output, "  end process;\n");
            fprintf(output, "end architecture;\n\n");
            break;
            
        // Add cases for other node types as you implement them
        default:
            break;
    }
}