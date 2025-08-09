#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "utils.h"

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
    int brace_depth = 1;
    while (brace_depth > 0 && !match(TOKEN_EOF)) {
        if (match(TOKEN_BRACE_OPEN)) {
            brace_depth++;
            advance(input);
        } else if (match(TOKEN_BRACE_CLOSE)) {
            brace_depth--;
            advance(input);
        } else {
            // Parse a statement and add it as a child of the function node
            ASTNode* stmt = parse_statement(input);
            if (stmt) add_child(func_node, stmt);
        }
    }

    return func_node;
}

// Parse a single statement inside a function body
ASTNode* parse_statement(FILE *input) {
    // Declare all nodes and tokens at the beginning
    ASTNode *stmt_node = NULL;
    ASTNode *var_decl_node = NULL;
    ASTNode *init_expr = NULL;
    ASTNode *return_expr = NULL;
    ASTNode *assign_node = NULL;
    ASTNode *lhs_node = NULL;
    ASTNode *rhs_node = NULL;
    Token type_token;
    Token name_token;
    Token lhs_token;

    // Create a generic statement node
    stmt_node = create_node(NODE_STATEMENT);

    // Variable declaration: e.g., int x;
    if (match(TOKEN_KEYWORD) && (
            strcmp(current_token.value, "int") == 0 ||
            strcmp(current_token.value, "float") == 0 ||
            strcmp(current_token.value, "char") == 0 ||
            strcmp(current_token.value, "double") == 0)) {

        type_token = current_token;
        advance(input);

        // Expect an identifier (variable name)
        if (match(TOKEN_IDENTIFIER)) {
            name_token = current_token;
            advance(input);

            var_decl_node = create_node(NODE_VAR_DECL);
            var_decl_node->token = type_token;
            var_decl_node->value = strdup(name_token.value);

            // Optionally handle initialization (e.g., int x = 5;)
            if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) {
                advance(input);
                init_expr = parse_expression(input);
                if (init_expr) add_child(var_decl_node, init_expr);
                while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) {
                    advance(input);
                }
            }

            if (!consume(input, TOKEN_SEMICOLON)) {
                printf("Error: Expected ';' after variable declaration\n");
            }

            add_child(stmt_node, var_decl_node);
            return stmt_node;
        } else {
            printf("Error: Expected variable name after type\n");
            while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) {
                advance(input);
            }
            if (match(TOKEN_SEMICOLON)) advance(input);
            return stmt_node;
        }
    }

    // Assignment statement: x = value;
    if (match(TOKEN_IDENTIFIER)) {
        lhs_token = current_token;
        advance(input);

        if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) {
            advance(input);

            assign_node = create_node(NODE_ASSIGNMENT);

            // Left-hand side node
            lhs_node = create_node(NODE_EXPRESSION);
            lhs_node->value = strdup(lhs_token.value);
            add_child(assign_node, lhs_node);

            // Right-hand side node
            rhs_node = parse_expression(input);
            if (rhs_node) add_child(assign_node, rhs_node);

            // Expect semicolon
            if (!consume(input, TOKEN_SEMICOLON)) {
                printf("Error: Expected ';' after assignment\n");
            }

            add_child(stmt_node, assign_node);
            return stmt_node;
        } else {
            // Not an assignment, skip to semicolon
            while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) {
                advance(input);
            }
            if (match(TOKEN_SEMICOLON)) advance(input);
            return stmt_node;
        }
    }

    // Parse return statement: return <expr>;
    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "return") == 0) {
        stmt_node->token = current_token;
        advance(input);

        // Parse an optional expression after 'return'
        return_expr = parse_expression(input);
        if (return_expr) add_child(stmt_node, return_expr);

        if (!consume(input, TOKEN_SEMICOLON)) {
            printf("Error: Expected ';' after return statement\n");
        }
        return stmt_node;
    }

    // TODO: Add parsing for if, while, etc.

    // Skip unknown statement (consume until semicolon or brace)
    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
        advance(input);
    }
    if (match(TOKEN_SEMICOLON)) advance(input);
    return stmt_node;
}

// Parse a simple expression (currently only supports a single identifier or number)
ASTNode* parse_expression(FILE *input) {
    ASTNode *expr_node = NULL;

    // Handle a number literal
    if (match(TOKEN_NUMBER)) {
        expr_node = create_node(NODE_EXPRESSION);
        expr_node->value = strdup(current_token.value);
        advance(input);
        return expr_node;
    }

    // Handle an identifier (variable or parameter)
    if (match(TOKEN_IDENTIFIER)) {
        expr_node = create_node(NODE_EXPRESSION);
        expr_node->value = strdup(current_token.value);
        advance(input);
        return expr_node;
    }

    // (Optional) Extend here for operators, binary expressions, etc.

    // If not a valid expression, return NULL
    return NULL;
}

// Generate VHDL code from an AST
void generate_vhdl(ASTNode* node, FILE* output) {

    char* result_vhdl_type = "std_logic_vector(31 downto 0)"; // default
    
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
            
        case NODE_FUNCTION_DECL: {
            fprintf(output, "-- Function: %s\n", node->value);
            fprintf(output, "entity %s is\n", node->value);
            fprintf(output, "  port (\n");
            fprintf(output, "    clk : in std_logic;\n");
            fprintf(output, "    reset : in std_logic;\n");

            // Generate ports for parameters with type mapping
            int param_count = 0;
            for (int i = 0; i < node->num_children; i++) {
                ASTNode *child = node->children[i];
                if (child->type == NODE_VAR_DECL) {
                    fprintf(output, "    %s : in %s%s\n",
                        child->value,
                        ctype_to_vhdl(child->token.value),
                        (param_count == node->num_children - 1) ? "," : ";");
                    param_count++;
                }
            }

            // --- Determine return type for result port ---
            // Use function return type if possible
            if (node->token.value && strlen(node->token.value) > 0) {
                result_vhdl_type = ctype_to_vhdl(node->token.value);
            }
            // Optionally, check if the return value matches a parameter type
            // (already handled by function return type in most cases)

            fprintf(output, "    result : out %s\n", result_vhdl_type);
            fprintf(output, "  );\n");
            fprintf(output, "end entity;\n\n");

            fprintf(output, "architecture behavioral of %s is\n", node->value);

            // Declare internal signals for local variables (statements) with type mapping
            for (int i = 0; i < node->num_children; i++) {
                ASTNode *child = node->children[i];
                if (child->type == NODE_STATEMENT) {
                    for (int j = 0; j < child->num_children; j++) {
                        ASTNode *stmt_child = child->children[j];
                        if (stmt_child->type == NODE_VAR_DECL) {
                            // KNOWN ISSUE: When declaring local signals, rename any that collide with "result"
                            //TODO: fix it asap
                            if (strcmp(stmt_child->value, "result") == 0) {
                                fprintf(output, "  signal internal_%s : %s;\n", 
                                    stmt_child->value,
                                    ctype_to_vhdl(stmt_child->token.value));
                            } else {
                                fprintf(output, "  signal %s : %s;\n",
                                    stmt_child->value,
                                    ctype_to_vhdl(stmt_child->token.value));
                            }
                        }
                    }
                }
            }

            fprintf(output, "begin\n");
            fprintf(output, "  process(clk, reset)\n");
            fprintf(output, "  begin\n");
            fprintf(output, "    if reset = '1' then\n");
            fprintf(output, "      -- Reset logic\n");
            fprintf(output, "    elsif rising_edge(clk) then\n");
            for (int i = 0; i < node->num_children; i++) {
                ASTNode *child = node->children[i];
                if (child->type == NODE_STATEMENT) {
                    generate_vhdl(child, output); // This will emit result <= ...;
                }
            }
            fprintf(output, "    end if;\n");
            fprintf(output, "  end process;\n");
            fprintf(output, "end architecture;\n\n");
            break;
        }
            
        case NODE_STATEMENT: {
            for (int i = 0; i < node->num_children; i++) {
                ASTNode *child = node->children[i];
                if (child->type == NODE_EXPRESSION) {
                    fprintf(output, "      result <= %s;\n", child->value ? child->value : "std_logic_vector(to_unsigned(0, 32))");
                }
                // Assignment statement code generation
                if (child->type == NODE_ASSIGNMENT) {
                    // Expect: child->children[0] = lhs, child->children[1] = rhs
                    if (child->num_children == 2 &&
                        child->children[0]->type == NODE_EXPRESSION &&
                        child->children[1]->type == NODE_EXPRESSION) {
                        fprintf(output, "      %s <= %s;\n",
                            child->children[0]->value ? child->children[0]->value : "unknown",
                            child->children[1]->value ? child->children[1]->value : "unknown");
                    }
                }
            }
            break;
        }
            
        // Add cases for other node types as you implement them
        default:
            break;
    }
}