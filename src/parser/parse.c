#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "utils.h"
#include "token.h"
#include "symbol_structs.h"
#include "symbol_arrays.h"
#include "parse_expression.h"
#include "parse_struct.h"
#include "parse_function.h"
#include "parse_statement.h"
#include "codegen_vhdl.h"
#include <ctype.h>

// Current token shared by parsing modules
extern Token current_token;

// Parse the entire program: delegates to specialized modules
ASTNode* parse_program(FILE *input) {
    Token func_name = (Token){0};
    Token return_type = (Token){0};
    Token struct_name_tok = (Token){0};
    ASTNode *func_node = NULL;
    ASTNode *program_node = NULL;
    ASTNode *s = NULL;

    program_node = create_node(NODE_PROGRAM);

    advance(input); // prime tokenizer

    while (!match(TOKEN_EOF)) {
        #ifdef DEBUG
        printf("Parsing token: type=%d, value='%s'\n", current_token.type, current_token.value ? current_token.value : "");
        #endif
        if (match(TOKEN_KEYWORD)) {
            if (strcmp(current_token.value, "struct") == 0) {
                advance(input); // consume 'struct'
                if (match(TOKEN_IDENTIFIER)) {
                    struct_name_tok = current_token;
                    advance(input);
                    if (match(TOKEN_BRACE_OPEN)) { // struct definition
                        s = parse_struct(input, struct_name_tok);
                        if (s) {
                            add_child(program_node, s);
                        }
                        continue;
                    }
                    if (match(TOKEN_IDENTIFIER)) { // function returning struct
                        return_type = struct_name_tok;
                        func_name = current_token;
                        advance(input);
                        if (match(TOKEN_PARENTHESIS_OPEN)) {
                            func_node = parse_function(input, return_type, func_name);
                            if (func_node) {
                                add_child(program_node, func_node);
                            }
                            continue;
                        } else {
                            printf("Warning: Expected '(' after function name for struct return function '%s'\n", func_name.value);
                        }
                    } else {
                        printf("Warning: 'struct %s' not followed by function name or '{'\n", struct_name_tok.value);
                    }
                } else {
                    printf("Warning: 'struct' without name at line %d\n", current_token.line);
                }
                continue;
            }
            // primitive or known type function
            return_type = current_token;
            advance(input);
            if (match(TOKEN_IDENTIFIER)) {
                func_name = current_token;
                advance(input);
                if (match(TOKEN_PARENTHESIS_OPEN)) {
                    func_node = parse_function(input, return_type, func_name);
                    if (func_node) {
                        add_child(program_node, func_node);
                    }
                } else {
                    printf("Warning: Global variable declarations not yet implemented\n");
                    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) {
                        advance(input);
                    }
                    if (match(TOKEN_SEMICOLON)) {
                        advance(input);
                    }
                }
            } else {
                printf("Warning: Expected identifier after type at line %d\n", current_token.line);
                advance(input);
            }
        } else {
            advance(input); // Skip unknown token
        }
    }
    return program_node;
}