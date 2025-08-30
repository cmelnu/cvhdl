#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "utils.h"
#include "token.h"
#include <ctype.h>

// Current token and functions to manage the token stream
extern Token current_token;

// Simple per-function array table for bounds checking
extern ArrayInfo g_arrays[128];
extern int g_array_count;

// Track loop depth to validate break/continue usage
static int s_loop_depth = 0;
// Simple struct table
StructInfo g_structs[64];
int g_struct_count = 0;

int find_struct_index(const char *name) {
    for (int i = 0; i < g_struct_count; i++) {
        if (strcmp(g_structs[i].name, name) == 0) return i;
    }
    return -1;
}

const char* struct_field_type(const char *struct_name, const char *field_name) {
    int idx = find_struct_index(struct_name);
    if (idx < 0) return NULL;
    for (int f = 0; f < g_structs[idx].field_count; f++) {
        if (strcmp(g_structs[idx].fields[f].field_name, field_name) == 0) {
            return g_structs[idx].fields[f].field_type;
        }
    }
    return NULL;
}

// Parse the entire program
ASTNode* parse_program(FILE *input) {

    Token func_name, return_type;
    ASTNode *func_node = NULL;
    ASTNode* program_node = create_node(NODE_PROGRAM);

    advance(input); // Ensure this is present

    while (!match(TOKEN_EOF)) {
        
        #ifdef DEBUG
        printf("Parsing token: type=%d, value='%s'\n", current_token.type, current_token.value ? current_token.value : "");
        #endif
        // Check for function declaration
        if (match(TOKEN_KEYWORD)) { 
            if (strcmp(current_token.value, "struct") == 0) {
                Token struct_tok = current_token; advance(input);
                if (match(TOKEN_IDENTIFIER)) {
                    Token name_tok = current_token; advance(input);
                    if (match(TOKEN_BRACE_OPEN)) {
                        ASTNode *s = parse_struct(input, name_tok);
                        if (s) add_child(program_node, s);
                        continue;
                    }
                }
            }

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
                        #ifdef DEBUG
                        printf("Parsed function: %s\n", func_node->value); // Debug print
                        #endif
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

// Parse struct definition: struct Name { type field; ... };
ASTNode* parse_struct(FILE *input, Token struct_name_tok) {
    if (!consume(input, TOKEN_BRACE_OPEN)) {
        printf("Error (line %d): Expected '{' after struct name\n", current_token.line);
        return NULL;
    }
    ASTNode *snode = create_node(NODE_STRUCT_DECL);
    snode->value = strdup(struct_name_tok.value);
    // Register in table
    if (g_struct_count < (int)(sizeof(g_structs)/sizeof(g_structs[0]))) {
        strncpy(g_structs[g_struct_count].name, struct_name_tok.value, sizeof(g_structs[g_struct_count].name)-1);
        g_structs[g_struct_count].field_count = 0;
    }
    int struct_index = g_struct_count;
    while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
        if (match(TOKEN_KEYWORD)) {
            Token ftype = current_token; advance(input);
            if (match(TOKEN_IDENTIFIER)) {
                Token fname = current_token; advance(input);
                ASTNode *field = create_node(NODE_VAR_DECL);
                field->token = ftype; field->value = strdup(fname.value);
                add_child(snode, field);
                if (struct_index == g_struct_count && g_struct_count < (int)(sizeof(g_structs)/sizeof(g_structs[0]))) {
                    StructInfo *si = &g_structs[struct_index];
                    if (si->field_count < 32) {
                        strncpy(si->fields[si->field_count].field_name, fname.value, sizeof(si->fields[si->field_count].field_name)-1);
                        strncpy(si->fields[si->field_count].field_type, ftype.value, sizeof(si->fields[si->field_count].field_type)-1);
                        si->field_count++;
                    }
                }
                if (!consume(input, TOKEN_SEMICOLON)) {
                    printf("Error (line %d): Expected ';' after struct field\n", current_token.line); exit(EXIT_FAILURE);
                }
            } else {
                printf("Error (line %d): Expected field name in struct\n", current_token.line); exit(EXIT_FAILURE);
            }
        } else {
            advance(input);
        }
    }
    if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after struct body\n", current_token.line); }
    if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after struct declaration\n", current_token.line); }
    if (struct_index == g_struct_count) g_struct_count++;
    return snode;
}

// Parse a function declaration
ASTNode* parse_function(FILE *input, Token return_type, Token func_name) {

    Token param_type, param_name;
    Token possible_struct_token;
    memset(&param_type, 0, sizeof(Token));
    memset(&param_name, 0, sizeof(Token));
    ASTNode *param_node = NULL;
    ASTNode* func_node = create_node(NODE_FUNCTION_DECL);
    // Reset per-function array table
    g_array_count = 0;
    
    // Store return type
    func_node->token = return_type;
    
    // We already have the function name
    func_node->value = strdup(func_name.value);
    
    // We're already at the opening parenthesis, consume it
    if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
        printf("Error (line %d): Expected '(' after function name\n", current_token.line);
        free_node(func_node);
        exit(EXIT_FAILURE);
    }
    
    // Parse parameter list
    // For now, just consume tokens until closing parenthesis
    while (!match(TOKEN_PARENTHESIS_CLOSE) && !match(TOKEN_EOF)) {
        // Expect a type (keyword)
        if (match(TOKEN_KEYWORD)) {
            // Handle 'struct Name' in parameter list
            if (strcmp(current_token.value, "struct") == 0) {
                possible_struct_token = current_token; advance(input);
                if (match(TOKEN_IDENTIFIER)) { // struct name
                    param_type = current_token; // reuse as type holder (struct name)
                    advance(input);
                } else { printf("Error (line %d): Expected struct name in parameter list\n", current_token.line); break; }
            } else {
                param_type = current_token;
                advance(input);
            }

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
                printf("Error (line %d): Expected parameter name\n", current_token.line);
                break;
            }
        } else {
            // If not a keyword, skip (could be void or error)
            advance(input);
        }
    }
    
    if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
        printf("Error (line %d): Expected ')' after parameter list\n", current_token.line);
        free_node(func_node);
        exit(EXIT_FAILURE);
    }
    
    // Expect function body starting with '{'
    if (!consume(input, TOKEN_BRACE_OPEN)) {
        printf("Error (line %d): Expected '{' to start function body\n", current_token.line);
        free_node(func_node);
        exit(EXIT_FAILURE);
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
    // removed unused lhs_node
    ASTNode *rhs_node = NULL;
    Token type_token;
    Token name_token;
    Token lhs_token;

    // Create a generic statement node
    stmt_node = create_node(NODE_STATEMENT);

    // Variable declaration: e.g., int x; or int arr[10];
    if (match(TOKEN_KEYWORD) && (
            strcmp(current_token.value, "int") == 0 ||
            strcmp(current_token.value, "float") == 0 ||
            strcmp(current_token.value, "char") == 0 ||
            strcmp(current_token.value, "double") == 0 ||
            strcmp(current_token.value, "struct") == 0)) {

        type_token = current_token;
        advance(input);
        if (strcmp(type_token.value, "struct") == 0) {
            if (!match(TOKEN_IDENTIFIER)) { printf("Error (line %d): Expected struct name after 'struct'\n", current_token.line); exit(EXIT_FAILURE);}            
            // Overwrite type_token with actual struct name for downstream handling
            type_token = current_token; advance(input);
        }

        // Expect an identifier (variable name)
        if (match(TOKEN_IDENTIFIER)) {
            name_token = current_token;
            advance(input);

            var_decl_node = create_node(NODE_VAR_DECL);
            var_decl_node->token = type_token;
            var_decl_node->value = strdup(name_token.value);

            // Array declaration: int arr[10];
            int is_array = 0;
            char arr_size_buf[256] = {0};
        if (match(TOKEN_BRACKET_OPEN)) {
                is_array = 1;
                advance(input);
                if (match(TOKEN_NUMBER)) {
                    snprintf(arr_size_buf, sizeof(arr_size_buf), "%s", current_token.value);
                    // Store array size in value as "name[size]"
                    char buf[1024];
                    snprintf(buf, sizeof(buf), "%s[%s]", name_token.value, current_token.value);
                    free(var_decl_node->value);
                    var_decl_node->value = strdup(buf);
                    advance(input);
            // Register array for bounds checking
            register_array(name_token.value, atoi(arr_size_buf));
                } else {
                    printf("Error (line %d): Expected array size after '['\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
                if (!consume(input, TOKEN_BRACKET_CLOSE)) {
                    printf("Error (line %d): Expected ']' after array size\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
            }

            // Optionally handle initialization (e.g., int x = 5; or int arr[3] = {1,2,3};)
            if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) {
                advance(input);
                if (is_array && match(TOKEN_BRACE_OPEN)) {
                    // Array initializer: {1,2,3}
                    advance(input);
                    ASTNode *init_list = create_node(NODE_EXPRESSION); // Use NODE_EXPRESSION for initializer list
                    init_list->value = strdup("array_init");
                    while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
                        if (match(TOKEN_NUMBER) || match(TOKEN_IDENTIFIER)) {
                            ASTNode *elem = create_node(NODE_EXPRESSION);
                            elem->value = strdup(current_token.value);
                            add_child(init_list, elem);
                            advance(input);
                        } else if (match(TOKEN_COMMA)) {
                            advance(input);
                        } else {
                            // Skip unknown tokens
                            advance(input);
                        }
                    }
                    if (!consume(input, TOKEN_BRACE_CLOSE)) {
                        printf("Error (line %d): Expected '}' after array initializer\n", current_token.line);
                        exit(EXIT_FAILURE);
                    }
                    add_child(var_decl_node, init_list);
                } else {
                    // Scalar initialization
                    init_expr = parse_expression(input);
                    if (init_expr) add_child(var_decl_node, init_expr);
                    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) {
                        advance(input);
                    }
                }
            }

            if (!consume(input, TOKEN_SEMICOLON)) {
                printf("Error (line %d): Expected ';' after variable declaration\n", current_token.line);
                exit(EXIT_FAILURE);
            }

            add_child(stmt_node, var_decl_node);
            return stmt_node;
        } else {
            printf("Error (line %d): Expected variable name after type\n", current_token.line);
            exit(EXIT_FAILURE);
        }
    }

    // Assignment statement: x = value; or arr[i] = value;
    if (match(TOKEN_IDENTIFIER)) {
        lhs_token = current_token;
        advance(input);

        // Check for array access: arr[i]
        ASTNode *lhs_expr = NULL;
    if (match(TOKEN_BRACKET_OPEN)) {
            advance(input);
            // Capture full index expression as a string until ']'
            char idx_buf[512] = {0};
            int paren_depth = 0;
            while (!match(TOKEN_EOF)) {
                if (match(TOKEN_BRACKET_CLOSE) && paren_depth == 0) break;
                if (match(TOKEN_PARENTHESIS_OPEN)) {
                    strncat(idx_buf, "(", sizeof(idx_buf) - strlen(idx_buf) - 1);
                    advance(input);
                    paren_depth++;
                    continue;
                }
                if (match(TOKEN_PARENTHESIS_CLOSE)) {
                    strncat(idx_buf, ")", sizeof(idx_buf) - strlen(idx_buf) - 1);
                    advance(input);
                    if (paren_depth > 0) paren_depth--;
                    continue;
                }
                if (current_token.value) {
                    strncat(idx_buf, current_token.value, sizeof(idx_buf) - strlen(idx_buf) - 1);
                }
                advance(input);
            }
            if (!consume(input, TOKEN_BRACKET_CLOSE)) {
                printf("Error (line %d): Expected ']' after array index\n", current_token.line);
                exit(EXIT_FAILURE);
            }
            lhs_expr = create_node(NODE_EXPRESSION);
            char buf[1024];
            snprintf(buf, sizeof(buf), "%s[%s]", lhs_token.value, idx_buf);
            lhs_expr->value = strdup(buf);
            // Static bounds check for numeric literal index
            if (is_number_str(idx_buf)) {
                int idx_val = atoi(idx_buf);
                int arr_size = find_array_size(lhs_token.value);
                if (arr_size > 0 && (idx_val < 0 || idx_val >= arr_size)) {
                    printf("Error (line %d): Array index %d out of bounds for '%s' with size %d\n", current_token.line, idx_val, lhs_token.value, arr_size);
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            lhs_expr = create_node(NODE_EXPRESSION);
            lhs_expr->value = strdup(lhs_token.value);
        }

        if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) {
            advance(input);

            assign_node = create_node(NODE_ASSIGNMENT);
            add_child(assign_node, lhs_expr);

            // Right-hand side node
            rhs_node = parse_expression(input);
            if (rhs_node) add_child(assign_node, rhs_node);

            // Expect semicolon
            if (!consume(input, TOKEN_SEMICOLON)) {
                printf("Error (line %d): Expected ';' after assignment\n", current_token.line);
                exit(EXIT_FAILURE);
            }

            add_child(stmt_node, assign_node);
            return stmt_node;
        } else {
            // Not an assignment, skip to semicolon
            while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) {
                advance(input);
            }
            if (match(TOKEN_SEMICOLON)) advance(input);
            free_node(lhs_expr);
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
            printf("Error (line %d): Expected ';' after return statement\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        return stmt_node;
    }

    // Parse if statement: if (<expr>) { ... }
    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "if") == 0) {
        advance(input);

        // Expect '('
        if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
            printf("Error (line %d): Expected '(' after 'if'\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Parse condition expression
        ASTNode *cond_expr = parse_expression(input);

        if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
            printf("Error (line %d): Expected ')' after if condition\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Expect '{'
        if (!consume(input, TOKEN_BRACE_OPEN)) {
            printf("Error (line %d): Expected '{' after if condition\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Parse statements inside if block
        ASTNode *if_node = create_node(NODE_IF_STATEMENT);
        if (cond_expr) add_child(if_node, cond_expr);

        while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
            ASTNode *inner_stmt = parse_statement(input);
            if (inner_stmt) add_child(if_node, inner_stmt);
        }

        if (!consume(input, TOKEN_BRACE_CLOSE)) {
            printf("Error (line %d): Expected '}' after if block\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Check for else if / else
        while (match(TOKEN_KEYWORD) && strcmp(current_token.value, "else") == 0) {
            advance(input);
            if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "if") == 0) {
                // else if
                advance(input);
                if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
                    printf("Error (line %d): Expected '(' after 'else if'\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
                ASTNode *elseif_cond = parse_expression(input);
                if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
                    printf("Error (line %d): Expected ')' after else if condition\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
                if (!consume(input, TOKEN_BRACE_OPEN)) {
                    printf("Error (line %d): Expected '{' after else if condition\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
                ASTNode *elseif_node = create_node(NODE_ELSE_IF_STATEMENT);
                if (elseif_cond) add_child(elseif_node, elseif_cond);
                while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
                    ASTNode *inner_stmt = parse_statement(input);
                    if (inner_stmt) add_child(elseif_node, inner_stmt);
                }
                if (!consume(input, TOKEN_BRACE_CLOSE)) {
                    printf("Error (line %d): Expected '}' after else if block\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
                add_child(if_node, elseif_node);
            } else {
                // else
                if (!consume(input, TOKEN_BRACE_OPEN)) {
                    printf("Error (line %d): Expected '{' after else\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
                ASTNode *else_node = create_node(NODE_ELSE_STATEMENT);
                while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
                    ASTNode *inner_stmt = parse_statement(input);
                    if (inner_stmt) add_child(else_node, inner_stmt);
                }
                if (!consume(input, TOKEN_BRACE_CLOSE)) {
                    printf("Error (line %d): Expected '}' after else block\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
                add_child(if_node, else_node);
                break; // Only one else allowed
            }
        }

        add_child(stmt_node, if_node);
        return stmt_node;
    }

    // Parse while loop: while (<expr>) { ... }
    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "while") == 0) {
        advance(input);

        // Expect '('
        if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
            printf("Error (line %d): Expected '(' after 'while'\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Parse condition expression
        ASTNode *cond_expr = parse_expression(input);

        if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
            printf("Error (line %d): Expected ')' after while condition\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Expect '{'
        if (!consume(input, TOKEN_BRACE_OPEN)) {
            printf("Error (line %d): Expected '{' after while condition\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        ASTNode *while_node = create_node(NODE_WHILE_STATEMENT);
        if (cond_expr) add_child(while_node, cond_expr);

        // Parse statements inside while block
        s_loop_depth++;
        while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
            ASTNode *inner_stmt = parse_statement(input);
            if (inner_stmt) add_child(while_node, inner_stmt);
        }
        s_loop_depth--;

        if (!consume(input, TOKEN_BRACE_CLOSE)) {
            printf("Error (line %d): Expected '}' after while block\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        add_child(stmt_node, while_node);
        return stmt_node;
    }

    // Parse for loop: for (init; cond; incr) { ... }
    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "for") == 0) {
        advance(input);
    if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
            printf("Error (line %d): Expected '(' after 'for'\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Parse optional init statement (reuse variable decl or assignment parsing in a limited fashion)
        ASTNode *init_node = NULL;
        if (!match(TOKEN_SEMICOLON)) {
            long saved_pos = ftell(input);
            Token saved_token = current_token;
            if (match(TOKEN_KEYWORD) && (
                strcmp(current_token.value, "int") == 0 ||
                strcmp(current_token.value, "float") == 0 ||
                strcmp(current_token.value, "char") == 0 ||
                strcmp(current_token.value, "double") == 0)) {
                // Parse as a standalone statement then extract first child
                ASTNode *init_stmt = parse_statement(input);
                if (init_stmt && init_stmt->num_children > 0) {
                    ASTNode *child0 = init_stmt->children[0];
                    if (child0->type == NODE_VAR_DECL || child0->type == NODE_ASSIGNMENT) {
                        init_node = child0;
                    }
                }
            } else {
                // Attempt assignment expression of form ident = expr;
                if (match(TOKEN_IDENTIFIER)) {
                    Token temp_lhs = current_token;
                    advance(input);
                    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) {
                        advance(input);
                        ASTNode *assign = create_node(NODE_ASSIGNMENT);
                        ASTNode *lhs_expr = create_node(NODE_EXPRESSION);
                        lhs_expr->value = strdup(temp_lhs.value);
                        add_child(assign, lhs_expr);
                        ASTNode *rhs_expr = parse_expression(input);
                        if (rhs_expr) add_child(assign, rhs_expr);
                        if (!consume(input, TOKEN_SEMICOLON)) {
                            printf("Error (line %d): Expected ';' after for-init assignment\n", current_token.line);
                            exit(EXIT_FAILURE);
                        }
                        init_node = assign;
                    } else {
                        // Rewind if not an assignment (treat as empty init)
                        fseek(input, saved_pos, SEEK_SET);
                        current_token = saved_token; // crude restore
                    }
                }
            }
        } else {
            // Empty init
        }
        if (match(TOKEN_SEMICOLON)) advance(input); // consume separator

        // Parse condition expression (or empty)
        ASTNode *cond_expr = NULL;
        if (!match(TOKEN_SEMICOLON)) {
            cond_expr = parse_expression(input);
        }
        if (!consume(input, TOKEN_SEMICOLON)) {
            printf("Error (line %d): Expected ';' after for condition\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        // Parse increment expression (or empty)
        ASTNode *incr_expr = NULL;
        if (!match(TOKEN_PARENTHESIS_CLOSE)) {
            // Support forms: i = i + 1; i++; ++i; i--; --i
            if (match(TOKEN_IDENTIFIER)) {
                Token inc_lhs = current_token;
                advance(input);
                if (match(TOKEN_OPERATOR) && (strcmp(current_token.value, "++") == 0 || strcmp(current_token.value, "--") == 0)) {
                    incr_expr = create_node(NODE_ASSIGNMENT);
                    ASTNode *lhs = create_node(NODE_EXPRESSION);
                    lhs->value = strdup(inc_lhs.value);
                    add_child(incr_expr, lhs);
                    ASTNode *rhs = create_node(NODE_BINARY_EXPR);
                    rhs->value = strdup(strcmp(current_token.value, "++") == 0 ? "+" : "-");
                    ASTNode *op_l = create_node(NODE_EXPRESSION); op_l->value = strdup(inc_lhs.value);
                    ASTNode *op_r = create_node(NODE_EXPRESSION); op_r->value = strdup("1");
                    add_child(rhs, op_l); add_child(rhs, op_r);
                    add_child(incr_expr, rhs);
                    advance(input);
                } else if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) {
                    // i = expr
                    advance(input);
                    incr_expr = create_node(NODE_ASSIGNMENT);
                    ASTNode *lhs = create_node(NODE_EXPRESSION); lhs->value = strdup(inc_lhs.value); add_child(incr_expr, lhs);
                    ASTNode *rhs = parse_expression(input); if (rhs) add_child(incr_expr, rhs);
                } else {
                    // Unsupported increment pattern – ignore
                }
            }
        }
        if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
            printf("Error (line %d): Expected ')' after for header\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        if (!consume(input, TOKEN_BRACE_OPEN)) {
            printf("Error (line %d): Expected '{' after for header\n", current_token.line);
            exit(EXIT_FAILURE);
        }

        ASTNode *for_node = create_node(NODE_FOR_STATEMENT);
        // Children order: init, condition, body statements..., increment (stored last for codegen)
        if (init_node) add_child(for_node, init_node);
        if (cond_expr) add_child(for_node, cond_expr); else {
            // If no condition, create a constant true (while true loop)
            ASTNode *true_expr = create_node(NODE_EXPRESSION); true_expr->value = strdup("1"); add_child(for_node, true_expr);
        }

        s_loop_depth++;
        while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
            ASTNode *inner = parse_statement(input);
            if (inner) add_child(for_node, inner);
        }
        s_loop_depth--;

        if (!consume(input, TOKEN_BRACE_CLOSE)) {
            printf("Error (line %d): Expected '}' after for body\n", current_token.line);
            exit(EXIT_FAILURE);
        }

    if (incr_expr) add_child(for_node, incr_expr);

        add_child(stmt_node, for_node);
        return stmt_node;
    }

    // Parse break statement: break;
    if ((match(TOKEN_KEYWORD) && strcmp(current_token.value, "break") == 0) ||
        (match(TOKEN_IDENTIFIER) && strcmp(current_token.value, "break") == 0)) {
        if (s_loop_depth <= 0) {
            printf("Error (line %d): 'break' not within a loop\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        advance(input);
        if (!consume(input, TOKEN_SEMICOLON)) {
            printf("Error (line %d): Expected ';' after 'break'\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        ASTNode *br = create_node(NODE_BREAK_STATEMENT);
        add_child(stmt_node, br);
        return stmt_node;
    }

    // Parse continue statement: continue;
    if ((match(TOKEN_KEYWORD) && strcmp(current_token.value, "continue") == 0) ||
        (match(TOKEN_IDENTIFIER) && strcmp(current_token.value, "continue") == 0)) {
        if (s_loop_depth <= 0) {
            printf("Error (line %d): 'continue' not within a loop\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        advance(input);
        if (!consume(input, TOKEN_SEMICOLON)) {
            printf("Error (line %d): Expected ';' after 'continue'\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        ASTNode *cn = create_node(NODE_CONTINUE_STATEMENT);
        add_child(stmt_node, cn);
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


// Primary: identifiers, numbers, unary minus, and parentheses
ASTNode* parse_primary(FILE *input) {

    // Unary logical NOT
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "!") == 0) {
        advance(input);
        ASTNode *inner = parse_primary(input);
        if (!inner) return NULL;
        ASTNode *node = create_node(NODE_BINARY_OP);
        node->value = strdup("!");
        add_child(node, inner);
        return node;
    }
    // Unary bitwise NOT
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "~") == 0) {
        advance(input);
        ASTNode *inner = parse_primary(input);
        if (!inner) return NULL;
        ASTNode *node = create_node(NODE_BINARY_OP);
        node->value = strdup("~");
        add_child(node, inner);
        return node;
    }

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
            printf("Error (line %d): Expected ')' after expression\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        return node;
    }

    // Identifier or number (support array access with full index expression but stored as string)
    if (match(TOKEN_IDENTIFIER)) {
        char ident_buf[128] = {0};
        strncpy(ident_buf, current_token.value, sizeof(ident_buf)-1);
        advance(input);
        // Field access chain: a.b.c (store as a__b__c for simplicity)
        while (match(TOKEN_OPERATOR) && strcmp(current_token.value, ".") == 0) {
            advance(input);
            if (!match(TOKEN_IDENTIFIER)) { printf("Error (line %d): Expected field name after '.'\n", current_token.line); exit(EXIT_FAILURE);}            
            strncat(ident_buf, "__", sizeof(ident_buf)-strlen(ident_buf)-1);
            strncat(ident_buf, current_token.value, sizeof(ident_buf)-strlen(ident_buf)-1);
            advance(input);
        }
        // Check for array indexing: ident[expr]
        if (match(TOKEN_BRACKET_OPEN)) {
            advance(input);
            // Capture full index expression as a string until ']'
            char idx_buf[512] = {0};
            int paren_depth = 0;
            while (!match(TOKEN_EOF)) {
                if (match(TOKEN_BRACKET_CLOSE) && paren_depth == 0) break;
                if (match(TOKEN_PARENTHESIS_OPEN)) {
                    strncat(idx_buf, "(", sizeof(idx_buf) - strlen(idx_buf) - 1);
                    advance(input);
                    paren_depth++;
                    continue;
                }
                if (match(TOKEN_PARENTHESIS_CLOSE)) {
                    strncat(idx_buf, ")", sizeof(idx_buf) - strlen(idx_buf) - 1);
                    advance(input);
                    if (paren_depth > 0) paren_depth--;
                    continue;
                }
                if (current_token.value) {
                    strncat(idx_buf, current_token.value, sizeof(idx_buf) - strlen(idx_buf) - 1);
                }
                advance(input);
            }
            if (!consume(input, TOKEN_BRACKET_CLOSE)) {
                printf("Error (line %d): Expected ']' after array index in expression\n", current_token.line);
                exit(EXIT_FAILURE);
            }
            ASTNode *node = create_node(NODE_EXPRESSION);
            char val_buf[700];
            snprintf(val_buf, sizeof(val_buf), "%s[%s]", ident_buf, idx_buf);
            node->value = strdup(val_buf);
            // Static bounds check when index is numeric literal
            if (is_number_str(idx_buf)) {
                int idx_val = atoi(idx_buf);
                int arr_size = find_array_size(ident_buf);
                if (arr_size > 0 && (idx_val < 0 || idx_val >= arr_size)) {
                    printf("Error (line %d): Array index %d out of bounds for '%s' with size %d\n", current_token.line, idx_val, ident_buf, arr_size);
                    exit(EXIT_FAILURE);
                }
            }
            return node;
        } else {
            ASTNode *node = create_node(NODE_EXPRESSION);
            node->value = strdup(ident_buf);
            return node;
        }
    }
    if (match(TOKEN_NUMBER)) {
        ASTNode *node = create_node(NODE_EXPRESSION);
        node->value = strdup(current_token.value);
        advance(input);
        return node;
    }

    return NULL;
}

// Precedence-climbing parser
ASTNode* parse_expression_prec(FILE *input, int min_prec) {
    ASTNode *left = parse_primary(input);
    if (!left) return NULL;

    while (match(TOKEN_OPERATOR)) {
        const char *op = current_token.value;
        int prec = get_precedence(op);
        if (prec < min_prec) break;

        // Capture operator and advance
    char op_buf[8] = {0};
        strncpy(op_buf, op, sizeof(op_buf) - 1);
        advance(input);

        // Parse right-hand side with higher minimum precedence (left-associative)
    ASTNode *right = parse_expression_prec(input, prec + 1);
        if (!right) {
            printf("Error (line %d): Expected right operand after operator '%s'\n", current_token.line, op_buf);
            exit(EXIT_FAILURE);
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
    // Start from the lowest precedence we support so logical ops are parsed
    return parse_expression_prec(input, -2);
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
            // Emit struct record type declarations
            for (int s = 0; s < g_struct_count; s++) {
                StructInfo *si = &g_structs[s];
                fprintf(output, "-- Struct %s as VHDL record\n", si->name);
                fprintf(output, "type %s_t is record\n", si->name);
                for (int f = 0; f < si->field_count; f++) {
                    fprintf(output, "  %s : %s;\n", si->fields[f].field_name, ctype_to_vhdl(si->fields[f].field_type));
                }
                fprintf(output, "end record;\n\n");
            }
            
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
            // Collect parameter nodes first (NODE_VAR_DECL immediately under function before statements)
            ASTNode *params[128]; int pcount = 0;
            for (int i = 0; i < node->num_children; i++) {
                ASTNode *c = node->children[i];
                if (c->type == NODE_VAR_DECL) {
                    params[pcount++] = c;
                } else {
                    // Stop on first non param (heuristic)
                    // break; // Optionally break; keep scanning in case order mixed
                    ;
                }
            }
            for (int i = 0; i < pcount; i++) {
                ASTNode *p = params[i];
                int is_struct = find_struct_index(p->token.value) >= 0;
                if (is_struct) {
                    fprintf(output, "    %s : in %s_t;\n", p->value, p->token.value);
                } else {
                    fprintf(output, "    %s : in %s;\n", p->value, ctype_to_vhdl(p->token.value));
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
                            // Struct variable
                            if (find_struct_index(stmt_child->token.value) >= 0) {
                                fprintf(output, "  signal %s : %s_t;\n", stmt_child->value, stmt_child->token.value);
                                continue;
                            }
                            // Array declaration: value is "name[size]"
                            char *arr_bracket = strchr(stmt_child->value, '[');
                            if (arr_bracket) {
                                // Parse name and size
                                int name_len = arr_bracket - stmt_child->value;
                                char arr_name[64] = {0};
                                strncpy(arr_name, stmt_child->value, name_len);
                                char arr_size[32] = {0};
                                const char *size_start = arr_bracket + 1;
                                const char *size_end = strchr(size_start, ']');
                                if (size_end && size_end > size_start) {
                                    strncpy(arr_size, size_start, size_end - size_start);
                                    // Map C type to VHDL type
                                    const char *vhdl_elem_type = ctype_to_vhdl(stmt_child->token.value);
                                    fprintf(output, "  type %s_type is array (0 to %d) of %s;\n", arr_name, atoi(arr_size)-1, vhdl_elem_type);
                                    fprintf(output, "  signal %s : %s_type;\n", arr_name, arr_name);
                                    // Array initialization
                                    if (stmt_child->num_children > 0 && stmt_child->children[0]->value && strcmp(stmt_child->children[0]->value, "array_init") == 0) {
                                        ASTNode *init_list = stmt_child->children[0];
                                        fprintf(output, "  -- Array initialization\n");
                                        fprintf(output, "  constant %s_init : %s_type := (", arr_name, arr_name);
                                        for (int k = 0; k < init_list->num_children; k++) {
                                            const char *val = init_list->children[k]->value;
                                            if (strcmp(stmt_child->token.value, "int") == 0) {
                                                // Convert integer to std_logic_vector bit string
                                                char bitstr[40] = {0};
                                                int num = atoi(val);
                                                for (int b = 31; b >= 0; b--) {
                                                    bitstr[31-b] = ((num >> b) & 1) ? '1' : '0';
                                                }
                                                bitstr[32] = '\0';
                                                fprintf(output, "\"%s\"%s", bitstr, (k < init_list->num_children - 1) ? ", " : "");
                                            } else if (strcmp(stmt_child->token.value, "float") == 0 || strcmp(stmt_child->token.value, "double") == 0) {
                                                // For float/double, use real literals
                                                fprintf(output, "%s%s", val, (k < init_list->num_children - 1) ? ", " : "");
                                            } else if (strcmp(stmt_child->token.value, "char") == 0) {
                                                // For char, use character literal
                                                fprintf(output, "'%s'%s", val, (k < init_list->num_children - 1) ? ", " : "");
                                            } else {
                                                // Default: emit as is
                                                fprintf(output, "%s%s", val, (k < init_list->num_children - 1) ? ", " : "");
                                            }
                                        }
                                        fprintf(output, ");\n");
                                        fprintf(output, "  signal %s : %s_type := %s_init;\n", arr_name, arr_name, arr_name);
                                    }
                                }
                            } else if (strcmp(stmt_child->value, "result") == 0) {
                                fprintf(output, "  signal internal_%s : %s;\n", 
                                    stmt_child->value,
                                    ctype_to_vhdl(stmt_child->token.value));
                            } else {
                                fprintf(output, "  signal %s : %s;\n",
                                    stmt_child->value,
                                    ctype_to_vhdl(stmt_child->token.value));
                            }
                        }
                        // Recurse into for-loop headers for var declarations
                        if (stmt_child->type == NODE_FOR_STATEMENT) {
                            for (int f = 0; f < stmt_child->num_children; f++) {
                                ASTNode *for_child = stmt_child->children[f];
                                if (for_child->type == NODE_VAR_DECL) {
                                    char *arr_br = for_child->value ? strchr(for_child->value, '[') : NULL;
                                    if (arr_br) {
                                        int name_len = arr_br - for_child->value; char arr_name[64]={0}; strncpy(arr_name, for_child->value, name_len);
                                        char arr_size[32]={0}; const char *size_start = arr_br+1; const char *size_end = strchr(size_start, ']');
                                        if (size_end && size_end>size_start) { strncpy(arr_size, size_start, size_end-size_start); const char *vhdl_elem_type = ctype_to_vhdl(for_child->token.value); fprintf(output, "  type %s_type is array (0 to %d) of %s;\n", arr_name, atoi(arr_size)-1, vhdl_elem_type); fprintf(output, "  signal %s : %s_type;\n", arr_name, arr_name); }
                                    } else {
                                        fprintf(output, "  signal %s : %s;\n", for_child->value, ctype_to_vhdl(for_child->token.value));
                                    }
                                }
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
                    // Only emit assignment for scalar variables, not arrays
                    char *arr_bracket = child->value ? strchr(child->value, '[') : NULL;
                    if (child->num_children > 0 && !arr_bracket) {
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
                        // Check for array access: arr[i]
                        if (lhs->value && strchr(lhs->value, '[')) {
                            // Convert arr[i] to arr(i) for VHDL
                            char arr_name[64] = {0};
                            char arr_idx[64] = {0};
                            const char *lbr = strchr(lhs->value, '[');
                            if (lbr) {
                                int name_len = lbr - lhs->value;
                                strncpy(arr_name, lhs->value, name_len);
                                const char *idx_start = lbr + 1;
                                const char *idx_end = strchr(idx_start, ']');
                                if (idx_end && idx_end > idx_start) {
                                    strncpy(arr_idx, idx_start, idx_end - idx_start);
                                    fprintf(output, "      %s(%s) <= ", arr_name, arr_idx);
                                    generate_vhdl(rhs, output);
                                    fprintf(output, ";\n");
                                } else {
                                    fprintf(output, "      -- Invalid array index\n");
                                }
                            }
                        } else {
                            fprintf(output, "      %s <= ", lhs->value ? lhs->value : "unknown");
                            generate_vhdl(rhs, output);
                            fprintf(output, ";\n");
                        }
                    }
                }
                // If statement VHDL generation
                if (child->type == NODE_IF_STATEMENT) {
                    generate_vhdl(child, output);
                }
                // While loop VHDL generation
                if (child->type == NODE_WHILE_STATEMENT) {
                    generate_vhdl(child, output);
                }
                // For loop VHDL generation
                if (child->type == NODE_FOR_STATEMENT) {
                    generate_vhdl(child, output);
                }
                // Break / Continue inside loops
                if (child->type == NODE_BREAK_STATEMENT || child->type == NODE_CONTINUE_STATEMENT) {
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
                if (child->type == NODE_BINARY_OP) {
                    fprintf(output, "      result <= ");
                    generate_vhdl(child, output);
                    fprintf(output, ";\n");
                }
            }
            break;
        }
        case NODE_WHILE_STATEMENT: {
            ASTNode *cond = node->children[0];
            fprintf(output, "      while ");
            if (cond->type == NODE_BINARY_EXPR) {
                // For comparisons/logicals, already boolean; otherwise compare != 0
                const char *cop = cond->value;
                int is_bool = (strcmp(cop, "==") == 0 || strcmp(cop, "!=") == 0 ||
                               strcmp(cop, "<") == 0 || strcmp(cop, "<=") == 0 ||
                               strcmp(cop, ">") == 0 || strcmp(cop, ">=") == 0 ||
                               strcmp(cop, "&&") == 0 || strcmp(cop, "||") == 0);
                if (is_bool) {
                    generate_vhdl(cond, output);
                } else {
                    fprintf(output, "unsigned(");
                    generate_vhdl(cond, output);
                    fprintf(output, ") /= 0");
                }
            } else if (cond->type == NODE_BINARY_OP) {
                generate_vhdl(cond, output);
            } else if (cond->type == NODE_EXPRESSION && cond->value) {
                fprintf(output, "unsigned(%s) /= 0", cond->value);
            } else {
                fprintf(output, "(%s)", cond->value ? cond->value : "false");
            }
            fprintf(output, " loop\n");
            for (int j = 1; j < node->num_children; j++) {
                generate_vhdl(node->children[j], output);
            }
            fprintf(output, "      end loop;\n");
            break;
        }
        case NODE_FOR_STATEMENT: {
            // Layout assumptions: first child (optional init assignment/var decl) already executed outside loop or must be executed before loop body.
            // We'll emit init first (if present and is assignment) then a while loop with condition, and append increment at end of body.
            if (node->num_children == 0) break;
            ASTNode *first = node->children[0];
            int cond_index = 0;
            if (first->type == NODE_ASSIGNMENT || first->type == NODE_VAR_DECL) {
                // Emit init
                if (first->type == NODE_ASSIGNMENT && first->num_children == 2) {
                    ASTNode *lhs = first->children[0];
                    ASTNode *rhs = first->children[1];
                    fprintf(output, "      ");
                    if (lhs->value && strchr(lhs->value, '[')) {
                        char arr_name[64]={0}; char arr_idx[64]={0}; const char *lbr=strchr(lhs->value,'[');
                        if (lbr){int name_len=lbr-lhs->value;strncpy(arr_name,lhs->value,name_len);const char *idx_start=lbr+1;const char *idx_end=strchr(idx_start,']'); if(idx_end&&idx_end>idx_start){strncpy(arr_idx,idx_start,idx_end-idx_start); fprintf(output, "%s(%s) <= ", arr_name, arr_idx); generate_vhdl(rhs, output); fprintf(output, ";\n");}}
                    } else {
                        fprintf(output, "%s <= ", lhs->value ? lhs->value : "unknown");
                        generate_vhdl(rhs, output);
                        fprintf(output, ";\n");
                    }
                    cond_index = 1;
                } else if (first->type == NODE_VAR_DECL) {
                    // Variable declarations with optional init handled similarly to NODE_STATEMENT generation logic
                    if (first->num_children > 0) {
                        ASTNode *init = first->children[0];
                        fprintf(output, "      %s <= ", first->value ? first->value : "unknown");
                        generate_vhdl(init, output);
                        fprintf(output, ";\n");
                    }
                    cond_index = 1;
                }
            }
            if (cond_index >= node->num_children) break;
            ASTNode *cond = node->children[cond_index];
            // Determine increment (last child if assignment and not part of body)
            int incr_index = node->num_children - 1;
            ASTNode *incr = NULL;
            if (node->children[incr_index]->type == NODE_ASSIGNMENT && incr_index != cond_index) {
                incr = node->children[incr_index];
            } else {
                incr_index = -1;
            }
            fprintf(output, "      while ");
            if (cond->type == NODE_BINARY_EXPR) {
                const char *cop = cond->value;
                int is_bool = (strcmp(cop, "==") == 0 || strcmp(cop, "!=") == 0 || strcmp(cop, "<") == 0 || strcmp(cop, "<=") == 0 || strcmp(cop, ">") == 0 || strcmp(cop, ">=") == 0 || strcmp(cop, "&&") == 0 || strcmp(cop, "||") == 0);
                if (is_bool) {
                    generate_vhdl(cond, output);
                } else {
                    fprintf(output, "unsigned("); generate_vhdl(cond, output); fprintf(output, ") /= 0");
                }
            } else if (cond->type == NODE_BINARY_OP) {
                generate_vhdl(cond, output);
            } else if (cond->type == NODE_EXPRESSION && cond->value) {
                fprintf(output, "unsigned(%s) /= 0", cond->value);
            } else {
                fprintf(output, "( %s )", cond->value ? cond->value : "false");
            }
            fprintf(output, " loop\n");
            for (int j = cond_index + 1; j < node->num_children; j++) {
                if (j == incr_index) continue; // skip increment, emit at end
                generate_vhdl(node->children[j], output);
            }
            if (incr) {
                if (incr->num_children == 2) {
                    ASTNode *lhs = incr->children[0];
                    ASTNode *rhs = incr->children[1];
                    fprintf(output, "        ");
                    if (lhs->value && strchr(lhs->value, '[')) {
                        char arr_name[64]={0}; char arr_idx[64]={0}; const char *lbr=strchr(lhs->value,'[');
                        if (lbr){int name_len=lbr-lhs->value;strncpy(arr_name,lhs->value,name_len);const char *idx_start=lbr+1;const char *idx_end=strchr(idx_start,']'); if(idx_end&&idx_end>idx_start){strncpy(arr_idx,idx_start,idx_end-idx_start); fprintf(output, "%s(%s) <= ", arr_name, arr_idx); generate_vhdl(rhs, output); fprintf(output, ";\n");}}
                    } else {
                        fprintf(output, "%s <= ", lhs->value ? lhs->value : "unknown");
                        generate_vhdl(rhs, output);
                        fprintf(output, ";\n");
                    }
                }
            }
            fprintf(output, "      end loop;\n");
            break;
        }
        case NODE_BREAK_STATEMENT: {
            fprintf(output, "      exit;\n");
            break;
        }
        case NODE_CONTINUE_STATEMENT: {
            fprintf(output, "      next;\n");
            break;
        }
            
        case NODE_BINARY_EXPR: {
            const char *op = node->value;
            // Map C operators to VHDL
            if (strcmp(op, "==") == 0) op = "=";
            else if (strcmp(op, "!=") == 0) op = "/=";

            ASTNode *left = node->children[0];
            ASTNode *right = node->children[1];

            // Logical AND/OR -> boolean expressions
            if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
                // Helper lambdas (expanded inline) to emit boolean value of an expression
                // If expr already boolean (comparison or logical), just emit it in parens
                // Else, compare its unsigned value against zero
                // Left side
                int left_is_bool = 0;
                if (left->type == NODE_BINARY_EXPR && left->value) {
                    const char *lop = left->value;
                    if (strcmp(lop, "==") == 0 || strcmp(lop, "!=") == 0 ||
                        strcmp(lop, "<") == 0  || strcmp(lop, "<=") == 0 ||
                        strcmp(lop, ">") == 0  || strcmp(lop, ">=") == 0 ||
                        strcmp(lop, "&&") == 0 || strcmp(lop, "||") == 0) {
                        left_is_bool = 1;
                    }
                }
                int right_is_bool = 0;
                if (right->type == NODE_BINARY_EXPR && right->value) {
                    const char *rop = right->value;
                    if (strcmp(rop, "==") == 0 || strcmp(rop, "!=") == 0 ||
                        strcmp(rop, "<") == 0  || strcmp(rop, "<=") == 0 ||
                        strcmp(rop, ">") == 0  || strcmp(rop, ">=") == 0 ||
                        strcmp(rop, "&&") == 0 || strcmp(rop, "||") == 0) {
                        right_is_bool = 1;
                    }
                }

                fprintf(output, "(");
                if (left_is_bool) {
                    fprintf(output, "(");
                    generate_vhdl(left, output);
                    fprintf(output, ")");
                } else {
                    fprintf(output, "unsigned(");
                    generate_vhdl(left, output);
                    fprintf(output, ") /= 0");
                }

                fprintf(output, "%s", strcmp(op, "&&") == 0 ? " and " : " or ");

                if (right_is_bool) {
                    fprintf(output, "(");
                    generate_vhdl(right, output);
                    fprintf(output, ")");
                } else {
                    fprintf(output, "unsigned(");
                    generate_vhdl(right, output);
                    fprintf(output, ") /= 0");
                }
                fprintf(output, ")");
                break;
            }

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
                int is_bool = (strcmp(cop, "==") == 0 || strcmp(cop, "!=") == 0 ||
                               strcmp(cop, "<") == 0 || strcmp(cop, "<=") == 0 ||
                               strcmp(cop, ">") == 0 || strcmp(cop, ">=") == 0 ||
                               strcmp(cop, "&&") == 0 || strcmp(cop, "||") == 0);
                if (is_bool) {
                    generate_vhdl(cond, output);
                } else {
                    fprintf(output, "unsigned(");
                    generate_vhdl(cond, output);
                    fprintf(output, ") /= 0");
                }
                fprintf(output, " then\n");
            } else if (cond->type == NODE_BINARY_OP) {
                // Unary logical '!' is a boolean expression
                generate_vhdl(cond, output);
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
                        // As above, handle boolean vs numeric for elseif condition
                        const char *ecop = elseif_cond->value;
                        int elseif_is_bool = (strcmp(ecop, "==") == 0 || strcmp(ecop, "!=") == 0 ||
                                              strcmp(ecop, "<") == 0 || strcmp(ecop, "<=") == 0 ||
                                              strcmp(ecop, ">") == 0 || strcmp(ecop, ">=") == 0 ||
                                              strcmp(ecop, "&&") == 0 || strcmp(ecop, "||") == 0);
                        if (elseif_is_bool) {
                            generate_vhdl(elseif_cond, output);
                        } else {
                            fprintf(output, "unsigned(");
                            generate_vhdl(elseif_cond, output);
                            fprintf(output, ") /= 0");
                        }
                        fprintf(output, " then\n");
                    } else if (elseif_cond->type == NODE_BINARY_OP) {
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
            // Array access: arr[i]
            if (node->value && strchr(node->value, '[')) {
                char arr_name[64] = {0};
                char arr_idx[64] = {0};
                const char *lbr = strchr(node->value, '[');
                if (lbr) {
                    int name_len = lbr - node->value;
                    strncpy(arr_name, node->value, name_len);
                    const char *idx_start = lbr + 1;
                    const char *idx_end = strchr(idx_start, ']');
                    if (idx_end && idx_end > idx_start) {
                        strncpy(arr_idx, idx_start, idx_end - idx_start);
                        fprintf(output, "%s(%s)", arr_name, arr_idx);
                    } else {
                        fprintf(output, "-- Invalid array index");
                    }
                }
            } else if (is_negative_literal(node->value)) {
                // If it's a negative identifier (e.g., -y)
                if (isalpha(node->value[1]) || node->value[1] == '_') {
                    fprintf(output, "-unsigned(%s)", node->value + 1);
                } else {
                    // Negative number (int or float)
                    fprintf(output, "to_signed(%s, 32)", node->value);
                }
            } else {
                // Struct field access encoded as a__b__c -> a.b.c
                if (node->value && strstr(node->value, "__")) {
                    char buf[256]; strncpy(buf, node->value, sizeof(buf)-1); buf[sizeof(buf)-1]='\0';
                    for (char *p = buf; *p; p++) { if (*p=='_' && *(p+1)=='_') { *p='.'; memmove(p+1, p+2, strlen(p+2)+1); } }
                    fprintf(output, "%s", buf);
                } else {
                    fprintf(output, "%s", node->value ? node->value : "unknown");
                }
            }
            break;
        }
        case NODE_BINARY_OP: {
            // Unary logical '!'
            if (node->value && strcmp(node->value, "!") == 0 && node->num_children == 1) {
                ASTNode *inner = node->children[0];
                int inner_is_bool = 0;
                if (inner->type == NODE_BINARY_EXPR && inner->value) {
                    const char *iop = inner->value;
                    if (strcmp(iop, "==") == 0 || strcmp(iop, "!=") == 0 ||
                        strcmp(iop, "<") == 0  || strcmp(iop, "<=") == 0 ||
                        strcmp(iop, ">") == 0  || strcmp(iop, ">=") == 0 ||
                        strcmp(iop, "&&") == 0 || strcmp(iop, "||") == 0) {
                        inner_is_bool = 1;
                    }
                } else if (inner->type == NODE_BINARY_OP && inner->value && strcmp(inner->value, "!") == 0) {
                    inner_is_bool = 1; // not of boolean is boolean
                }

                if (inner_is_bool) {
                    fprintf(output, "not (");
                    generate_vhdl(inner, output);
                    fprintf(output, ")");
                } else {
                    fprintf(output, "(unsigned(");
                    generate_vhdl(inner, output);
                    fprintf(output, ") = 0)");
                }
            }
            // Unary bitwise '~'
            else if (node->value && strcmp(node->value, "~") == 0 && node->num_children == 1) {
                ASTNode *inner = node->children[0];
                fprintf(output, "not unsigned(");
                generate_vhdl(inner, output);
                fprintf(output, ")");
            }
            else {
                fprintf(output, "-- unsupported unary op");
            }
            break;
        }
        default: {
            // Unhandled node types either don't emit VHDL directly or are covered elsewhere
            break;
        }
    }
}