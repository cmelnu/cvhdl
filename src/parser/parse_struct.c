#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_struct.h"
#include "symbol_structs.h"
#include "parse.h"
#include "token.h"

static inline void safe_copy(char *dst, size_t dst_size, const char *src) {
    if (!dst_size) {
        return;
    }
    size_t n = strlen(src);
    if (n >= dst_size) {
        n = dst_size - 1;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

extern Token current_token;

// Parse struct definition: struct Name { type field; ... };
ASTNode* parse_struct(FILE *input, Token struct_name_tok) {
    ASTNode *snode = NULL;
    int struct_index = 0;
    Token ftype = (Token){0};
    Token fname = (Token){0};
    ASTNode *field = NULL;
    StructInfo *si = NULL;

    if (!consume(input, TOKEN_BRACE_OPEN)) {
        printf("Error (line %d): Expected '{' after struct name\n", current_token.line);
        return NULL;
    }
    snode = create_node(NODE_STRUCT_DECL);
    snode->value = strdup(struct_name_tok.value);
    // Register in table
    if (g_struct_count < (int)(sizeof(g_structs)/sizeof(g_structs[0]))) {
        safe_copy(g_structs[g_struct_count].name, sizeof(g_structs[g_struct_count].name), struct_name_tok.value);
        g_structs[g_struct_count].field_count = 0;
    }
    struct_index = g_struct_count;
    while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
        if (match(TOKEN_KEYWORD)) {
            ftype = current_token;
            advance(input);
            if (match(TOKEN_IDENTIFIER)) {
                fname = current_token;
                advance(input);
                field = create_node(NODE_VAR_DECL);
                field->token = ftype;
                field->value = strdup(fname.value);
                add_child(snode, field);
                if (struct_index == g_struct_count && g_struct_count < (int)(sizeof(g_structs)/sizeof(g_structs[0]))) {
                    si = &g_structs[struct_index];
                    if (si->field_count < 32) {
                        safe_copy(si->fields[si->field_count].field_name, sizeof(si->fields[si->field_count].field_name), fname.value);
                        safe_copy(si->fields[si->field_count].field_type, sizeof(si->fields[si->field_count].field_type), ftype.value);
                        si->field_count++;
                    }
                }
                if (!consume(input, TOKEN_SEMICOLON)) {
                    printf("Error (line %d): Expected ';' after struct field\n", current_token.line);
                    exit(EXIT_FAILURE);
                }
            } else {
                printf("Error (line %d): Expected field name in struct\n", current_token.line);
                exit(EXIT_FAILURE);
            }
        } else {
            advance(input);
        }
    }
    if (!consume(input, TOKEN_BRACE_CLOSE)) {
        printf("Error (line %d): Expected '}' after struct body\n", current_token.line);
    }
    if (!consume(input, TOKEN_SEMICOLON)) {
        printf("Error (line %d): Expected ';' after struct declaration\n", current_token.line);
    }
    if (struct_index == g_struct_count) {
        g_struct_count++;
    }
    return snode;
}
