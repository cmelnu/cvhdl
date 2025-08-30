#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_struct.h"
#include "symbol_structs.h"
#include "parse.h" // for create_node/add_child declarations
#include "token.h"

extern Token current_token;

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
