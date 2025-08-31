#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_function.h"
#include "parse_statement.h"
#include "symbol_arrays.h"
#include "parse.h" // create_node/add_child
#include "token.h"

extern Token current_token;
extern ArrayInfo g_arrays[128];
extern int g_array_count;

ASTNode* parse_function(FILE *input, Token return_type, Token func_name) {
    Token param_type = (Token){0};
    Token param_name = (Token){0};
    Token possible_struct_token = (Token){0}; (void)possible_struct_token;
    ASTNode *param_node = NULL;
    ASTNode *func_node = NULL;
    int brace_depth = 1;

    memset(&param_type, 0, sizeof(Token));
    memset(&param_name, 0, sizeof(Token));
    func_node = create_node(NODE_FUNCTION_DECL);
    g_array_count = 0;

    func_node->token = return_type;
    func_node->value = strdup(func_name.value);

    if (!consume(input, TOKEN_PARENTHESIS_OPEN)) {
        printf("Error (line %d): Expected '(' after function name\n", current_token.line);
        free_node(func_node);
        exit(EXIT_FAILURE);
    }

    while (!match(TOKEN_PARENTHESIS_CLOSE) && !match(TOKEN_EOF)) {
        if (match(TOKEN_KEYWORD)) {
            if (strcmp(current_token.value, "struct") == 0) {
                advance(input);
                if (match(TOKEN_IDENTIFIER)) {
                    param_type = current_token;
                    advance(input);
                } else {
                    printf("Error (line %d): Expected struct name in parameter list\n", current_token.line);
                    break;
                }
            } else {
                param_type = current_token;
                advance(input);
            }
            if (match(TOKEN_IDENTIFIER)) {
                param_name = current_token;
                advance(input);
                param_node = create_node(NODE_VAR_DECL);
                param_node->token = param_type;
                param_node->value = strdup(param_name.value);
                add_child(func_node, param_node);
                if (match(TOKEN_COMMA)) {
                    advance(input);
                }
            } else {
                printf("Error (line %d): Expected parameter name\n", current_token.line);
                break;
            }
        } else {
            advance(input);
        }
    }

    if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
        printf("Error (line %d): Expected ')' after parameter list\n", current_token.line);
        free_node(func_node);
        exit(EXIT_FAILURE);
    }
    if (!consume(input, TOKEN_BRACE_OPEN)) {
        printf("Error (line %d): Expected '{' to start function body\n", current_token.line);
        free_node(func_node);
        exit(EXIT_FAILURE);
    }

    brace_depth = 1;
    while (brace_depth > 0 && !match(TOKEN_EOF)) {
        if (match(TOKEN_BRACE_OPEN)) {
            brace_depth++;
            advance(input);
        } else if (match(TOKEN_BRACE_CLOSE)) {
            brace_depth--;
            advance(input);
        } else {
            ASTNode* stmt = parse_statement(input);
            if (stmt) {
                add_child(func_node, stmt);
            }
        }
    }

    return func_node;
}
