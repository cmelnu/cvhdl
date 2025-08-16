#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "utils.h"
#include <ctype.h>

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

    advance(input); // Ensure this is present

    while (!match(TOKEN_EOF)) {
        printf("Parsing token: type=%d, value='%s'\n", current_token.type, current_token.value ? current_token.value : "");
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
                    if (func_node != NULL) {
                        printf("Parsed function: %s\n", func_node->value); // Debug print
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

    // Parse if statement: if (<expr>) { ... }
    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "if") == 0) {
        advance(input);

        // Expect '('
        if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
            printf("Error: Expected '(' after 'if'\n");
            return stmt_node;
        }

        // Parse condition expression
        ASTNode *cond_expr = parse_expression(input);

        if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
            printf("Error: Expected ')' after if condition\n");
            return stmt_node;
        }

        // Expect '{'
        if (!consume(input, TOKEN_BRACE_OPEN)) {
            printf("Error: Expected '{' after if condition\n");
            return stmt_node;
        }

        // Parse statements inside if block
        ASTNode *if_node = create_node(NODE_IF_STATEMENT);
        if (cond_expr) add_child(if_node, cond_expr);

        while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
            ASTNode *inner_stmt = parse_statement(input);
            if (inner_stmt) add_child(if_node, inner_stmt);
        }

        if (!consume(input, TOKEN_BRACE_CLOSE)) {
            printf("Error: Expected '}' after if block\n");
        }

        // Check for else if / else
        while (match(TOKEN_KEYWORD) && strcmp(current_token.value, "else") == 0) {
            advance(input);
            if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "if") == 0) {
                // else if
                advance(input);
                if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
                    printf("Error: Expected '(' after 'else if'\n");
                    break;
                }
                ASTNode *elseif_cond = parse_expression(input);
                if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
                    printf("Error: Expected ')' after else if condition\n");
                    break;
                }
                if (!consume(input, TOKEN_BRACE_OPEN)) {
                    printf("Error: Expected '{' after else if condition\n");
                    break;
                }
                ASTNode *elseif_node = create_node(NODE_ELSE_IF_STATEMENT);
                if (elseif_cond) add_child(elseif_node, elseif_cond);
                while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
                    ASTNode *inner_stmt = parse_statement(input);
                    if (inner_stmt) add_child(elseif_node, inner_stmt);
                }
                if (!consume(input, TOKEN_BRACE_CLOSE)) {
                    printf("Error: Expected '}' after else if block\n");
                }
                add_child(if_node, elseif_node);
            } else {
                // else
                if (!consume(input, TOKEN_BRACE_OPEN)) {
                    printf("Error: Expected '{' after else\n");
                    break;
                }
                ASTNode *else_node = create_node(NODE_ELSE_STATEMENT);
                while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
                    ASTNode *inner_stmt = parse_statement(input);
                    if (inner_stmt) add_child(else_node, inner_stmt);
                }
                if (!consume(input, TOKEN_BRACE_CLOSE)) {
                    printf("Error: Expected '}' after else block\n");
                }
                add_child(if_node, else_node);
                break; // Only one else allowed
            }
        }

        add_child(stmt_node, if_node);
        return stmt_node;
    }

    // TODO: Add parsing for while, etc.

    // Skip unknown statement (consume until semicolon or brace)
    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
        advance(input);
    }
    if (match(TOKEN_SEMICOLON)) advance(input);
    return stmt_node;
}

// Forward declarations
static ASTNode* parse_expression_prec(FILE *input, int min_prec);
static ASTNode* parse_primary(FILE *input);

static int get_precedence(const char *op) {
    if (!op) return -1;
    // Higher number = higher precedence
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 6;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 5;
    if (strcmp(op, "<<") == 0 || strcmp(op, ">>") == 0) return 4;
    // Bitwise ops higher than equality so (x ^ 0) == 0 parses as intended
    if (strcmp(op, "&") == 0 || strcmp(op, "^") == 0 || strcmp(op, "|") == 0) return 3;
    if (strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
        strcmp(op, ">") == 0 || strcmp(op, ">=") == 0) return 2;
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) return 1;
    return -1;
}

// Primary: identifiers, numbers, unary minus, and parentheses
static ASTNode* parse_primary(FILE *input) {
    // Unary minus
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "-") == 0) {
        advance(input);
        ASTNode *inner = parse_primary(input);
        if (!inner) return NULL;

        // Represent unary minus by storing "-<expr>" in a NODE_EXPRESSION where possible
        if (inner->type == NODE_EXPRESSION && inner->value) {
            char buf[128];
            snprintf(buf, sizeof(buf), "-%s", inner->value);
            ASTNode *node = create_node(NODE_EXPRESSION);
            node->value = strdup(buf);
            free_node(inner);
            return node;
        } else {
            // Fallback: build as binary 0 - inner
            ASTNode *zero = create_node(NODE_EXPRESSION);
            zero->value = strdup("0");
            ASTNode *bin = create_node(NODE_BINARY_EXPR);
            bin->value = strdup("-");
            add_child(bin, zero);
            add_child(bin, inner);
            return bin;
        }
    }

    // Parenthesized expression
    if (match(TOKEN_PARENTHESIS_OPEN)) {
        advance(input);
        ASTNode *node = parse_expression_prec(input, 1);
        if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
            printf("Error: Expected ')' in expression\n");
        }
        return node;
    }

    // Identifier or number
    if (match(TOKEN_IDENTIFIER) || match(TOKEN_NUMBER)) {
        ASTNode *node = create_node(NODE_EXPRESSION);
        node->value = strdup(current_token.value);
        advance(input);
        return node;
    }

    return NULL;
}

// Precedence-climbing parser
static ASTNode* parse_expression_prec(FILE *input, int min_prec) {
    ASTNode *left = parse_primary(input);
    if (!left) return NULL;

    while (match(TOKEN_OPERATOR)) {
        const char *op = current_token.value;
        int prec = get_precedence(op);
        if (prec < min_prec) break;

        // Capture operator and advance
        char op_buf[4] = {0};
        strncpy(op_buf, op, sizeof(op_buf) - 1);
        advance(input);

        // Parse right-hand side with higher minimum precedence (left-associative)
        ASTNode *right = parse_expression_prec(input, prec + 1);
        if (!right) {
            printf("Error: Expected right operand after operator '%s'\n", op_buf);
            return left;
        }

        ASTNode *bin = create_node(NODE_BINARY_EXPR);
        bin->value = strdup(op_buf);
        add_child(bin, left);
        add_child(bin, right);
        left = bin;
    }

    return left;
}

// Replace the old parse_expression with precedence-aware version
ASTNode* parse_expression(FILE *input) {
    return parse_expression_prec(input, 1);
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
                // Handle variable declaration with initialization
                if (child->type == NODE_VAR_DECL) {
                    if (child->num_children > 0) {
                        ASTNode *init = child->children[0];
                        fprintf(output, "      %s <= ", child->value ? child->value : "unknown");
                        generate_vhdl(init, output);
                        fprintf(output, ";\n");
                    }
                }
                // Assignment statement code generation
                if (child->type == NODE_ASSIGNMENT) {
                    if (child->num_children == 2) {
                        ASTNode *lhs = child->children[0];
                        ASTNode *rhs = child->children[1];
                        fprintf(output, "      %s <= ", lhs->value ? lhs->value : "unknown");
                        generate_vhdl(rhs, output);
                        fprintf(output, ";\n");
                    }
                }
                // If statement VHDL generation
                if (child->type == NODE_IF_STATEMENT) {
                    generate_vhdl(child, output);
                }
                // Handle return value: support arithmetic, unary, direct values, bitwise, and comparison expressions
                if (child->type == NODE_EXPRESSION) {
                    fprintf(output, "      result <= ");
                    if (child->value && child->value[0] == '-' && strlen(child->value) > 1) {
                        // If it's a negative identifier (e.g., -y)
                        if (isalpha(child->value[1]) || child->value[1] == '_') {
                            fprintf(output, "-unsigned(%s)", child->value + 1);
                        } else {
                            // Negative number (int or float)
                            fprintf(output, "to_signed(%s, 32)", child->value);
                        }
                    } else {
                        generate_vhdl(child, output);
                    }
                    fprintf(output, ";\n");
                }
                if (child->type == NODE_BINARY_EXPR) {
                    fprintf(output, "      result <= ");
                    generate_vhdl(child, output);
                    fprintf(output, ";\n");
                }
            }
            break;
        }
            
        case NODE_BINARY_EXPR: {
            const char *op = node->value;
            // Map C operators to VHDL
            if (strcmp(op, "==") == 0) op = "=";
            else if (strcmp(op, "!=") == 0) op = "/=";

            ASTNode *left = node->children[0];
            ASTNode *right = node->children[1];

            // Comparison operators
            if (strcmp(op, "=") == 0 || strcmp(op, "/=") == 0 ||
                strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
                strcmp(op, ">") == 0 || strcmp(op, ">=") == 0) {
                // Left operand
                if (left->type == NODE_EXPRESSION) {
                    if (is_negative_literal(left->value)) {
                        fprintf(output, "to_signed(%s, 32)", left->value);
                    } else {
                        // If it's a non-negative number literal, convert to unsigned literal
                        int is_num = 1; const char *p = left->value; if (!*p) is_num = 0; while (*p) { if (!isdigit(*p) && *p != '.') { is_num = 0; break; } p++; }
                        if (is_num) {
                            fprintf(output, "to_unsigned(%s, 32)", left->value);
                        } else {
                            fprintf(output, "unsigned(%s)", left->value);
                        }
                    }
                } else {
                    // Complex expression
                    fprintf(output, "unsigned(");
                    generate_vhdl(left, output);
                    fprintf(output, ")");
                }

                fprintf(output, " %s ", op);

                // Right operand
                if (right->type == NODE_EXPRESSION) {
                    if (is_negative_literal(right->value)) {
                        fprintf(output, "to_signed(%s, 32)", right->value);
                    } else {
                        int is_num = 1; const char *p = right->value; if (!*p) is_num = 0; while (*p) { if (!isdigit(*p) && *p != '.') { is_num = 0; break; } p++; }
                        if (is_num) {
                            fprintf(output, "to_unsigned(%s, 32)", right->value);
                        } else {
                            fprintf(output, "unsigned(%s)", right->value);
                        }
                    }
                } else {
                    fprintf(output, "unsigned(");
                    generate_vhdl(right, output);
                    fprintf(output, ")");
                }
            }
            // Bitwise AND, OR, XOR
            else if (strcmp(op, "&") == 0) {
                fprintf(output, "unsigned(");
                generate_vhdl(left, output);
                fprintf(output, ") and unsigned(");
                generate_vhdl(right, output);
                fprintf(output, ")");
            } else if (strcmp(op, "|") == 0) {
                fprintf(output, "unsigned(");
                generate_vhdl(left, output);
                fprintf(output, ") or unsigned(");
                generate_vhdl(right, output);
                fprintf(output, ")");
            } else if (strcmp(op, "^") == 0) {
                fprintf(output, "unsigned(");
                generate_vhdl(left, output);
                fprintf(output, ") xor unsigned(");
                generate_vhdl(right, output);
                fprintf(output, ")");
            }
            // Bitwise shift left/right
            else if (strcmp(op, "<<") == 0) {
                fprintf(output, "shift_left(unsigned(");
                generate_vhdl(left, output);
                fprintf(output, "), to_integer(unsigned(");
                generate_vhdl(right, output);
                fprintf(output, ")))");
            } else if (strcmp(op, ">>") == 0) {
                fprintf(output, "shift_right(unsigned(");
                generate_vhdl(left, output);
                fprintf(output, "), to_integer(unsigned(");
                generate_vhdl(right, output);
                fprintf(output, ")))");
            }
            // Arithmetic
            else {
                generate_vhdl(left, output);
                fprintf(output, " %s ", op);
                generate_vhdl(right, output);
            }
            break;
        }
        case NODE_IF_STATEMENT: {
            ASTNode *cond = node->children[0];
            fprintf(output, "      if ");
            if (cond->type == NODE_BINARY_EXPR) {
                // For comparisons, result is boolean already; for bitwise/arithmetic, compare against zero
                const char *cop = cond->value;
                int is_cmp = (strcmp(cop, "==") == 0 || strcmp(cop, "!=") == 0 ||
                              strcmp(cop, "<") == 0 || strcmp(cop, "<=") == 0 ||
                              strcmp(cop, ">") == 0 || strcmp(cop, ">=") == 0);
                if (is_cmp) {
                    generate_vhdl(cond, output);
                } else {
                    fprintf(output, "unsigned(");
                    generate_vhdl(cond, output);
                    fprintf(output, ") /= 0");
                }
                fprintf(output, " then\n");
            } else if (cond->type == NODE_EXPRESSION && cond->value) {
                fprintf(output, "unsigned(%s) /= 0 then\n", cond->value);
            } else {
                fprintf(output, "(%s) then\n", cond->value ? cond->value : "false");
            }
            // If block statements
            for (int j = 1; j < node->num_children; j++) {
                ASTNode *branch = node->children[j];
                if (branch->type == NODE_ELSE_IF_STATEMENT) {
                    ASTNode *elseif_cond = branch->children[0];
                    fprintf(output, "      elsif ");
                    if (elseif_cond->type == NODE_BINARY_EXPR) {
                        generate_vhdl(elseif_cond, output);
                        fprintf(output, " then\n");
                    } else if (elseif_cond->type == NODE_EXPRESSION && elseif_cond->value) {
                        fprintf(output, "unsigned(%s) /= 0 then\n", elseif_cond->value);
                    } else {
                        fprintf(output, "(%s) then\n", elseif_cond->value ? elseif_cond->value : "false");
                    }
                    for (int k = 1; k < branch->num_children; k++) {
                        generate_vhdl(branch->children[k], output);
                    }
                } else if (branch->type == NODE_ELSE_STATEMENT) {
                    fprintf(output, "      else\n");
                    for (int k = 0; k < branch->num_children; k++) {
                        generate_vhdl(branch->children[k], output);
                    }
                } else {
                    generate_vhdl(branch, output);
                }
            }
            fprintf(output, "      end if;\n");
            break;
        }
        case NODE_EXPRESSION: {
            if (is_negative_literal(node->value)) {
                // If it's a negative identifier (e.g., -y)
                if (isalpha(node->value[1]) || node->value[1] == '_') {
                    fprintf(output, "-unsigned(%s)", node->value + 1);
                } else {
                    // Negative number (int or float)
                    fprintf(output, "to_signed(%s, 32)", node->value);
                }
            } else {
                fprintf(output, "%s", node->value ? node->value : "unknown");
            }
            break;
        }
    }
}

// Add this helper function above generate_vhdl:
int is_negative_literal(const char* value) {
    if (!value || value[0] != '-' || strlen(value) < 2)
        return 0;
    // Accept -123, -1.23, -0.5, -123.0, -y, -var, etc.
    int i = 1;
    int has_valid = 0;
    while (value[i]) {
        if (isdigit(value[i]) || value[i] == '.' || isalpha(value[i]) || value[i] == '_') {
            has_valid = 1;
        } else {
            return 0;
        }
        i++;
    }
    return has_valid;
}