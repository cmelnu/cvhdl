#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "utils.h"
#include "parse_expression.h"
#include "symbol_arrays.h"
#include "symbol_structs.h"

// Primary: identifiers, numbers, unary minus, logical/bitwise NOT, parentheses, field & array access
ASTNode* parse_primary(FILE *input) {
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "!") == 0) {
        advance(input);
        ASTNode *inner = parse_primary(input);
        if (!inner) return NULL;
        ASTNode *node = create_node(NODE_BINARY_OP);
        node->value = strdup("!");
        add_child(node, inner);
        return node;
    }
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "~") == 0) {
        advance(input);
        ASTNode *inner = parse_primary(input);
        if (!inner) return NULL;
        ASTNode *node = create_node(NODE_BINARY_OP);
        node->value = strdup("~");
        add_child(node, inner);
        return node;
    }
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "-") == 0) {
        advance(input);
        ASTNode *inner = parse_primary(input);
        if (!inner) return NULL;
        if (inner->type == NODE_EXPRESSION && inner->value) {
            char buf[128];
            snprintf(buf, sizeof(buf), "-%s", inner->value);
            ASTNode *node = create_node(NODE_EXPRESSION);
            node->value = strdup(buf);
            free_node(inner);
            return node;
        } else {
            ASTNode *zero = create_node(NODE_EXPRESSION); zero->value = strdup("0");
            ASTNode *bin = create_node(NODE_BINARY_EXPR); bin->value = strdup("-");
            add_child(bin, zero); add_child(bin, inner); return bin;
        }
    }
    if (match(TOKEN_PARENTHESIS_OPEN)) {
        advance(input);
        ASTNode *node = parse_expression_prec(input, 1);
        if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
            printf("Error (line %d): Expected ')' after expression\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        return node;
    }
    if (match(TOKEN_IDENTIFIER)) {
        char ident_buf[128] = {0};
        strncpy(ident_buf, current_token.value, sizeof(ident_buf)-1);
        advance(input);
        while (match(TOKEN_OPERATOR) && strcmp(current_token.value, ".") == 0) {
            advance(input);
            if (!match(TOKEN_IDENTIFIER)) { printf("Error (line %d): Expected field name after '.'\n", current_token.line); exit(EXIT_FAILURE);}            
            strncat(ident_buf, "__", sizeof(ident_buf)-strlen(ident_buf)-1);
            strncat(ident_buf, current_token.value, sizeof(ident_buf)-strlen(ident_buf)-1);
            advance(input);
        }
        if (match(TOKEN_BRACKET_OPEN)) {
            advance(input);
            char idx_buf[512] = {0}; int paren_depth = 0;
            while (!match(TOKEN_EOF)) {
                if (match(TOKEN_BRACKET_CLOSE) && paren_depth == 0) break;
                if (match(TOKEN_PARENTHESIS_OPEN)) { strncat(idx_buf, "(", sizeof(idx_buf)-strlen(idx_buf)-1); advance(input); paren_depth++; continue; }
                if (match(TOKEN_PARENTHESIS_CLOSE)) { strncat(idx_buf, ")", sizeof(idx_buf)-strlen(idx_buf)-1); advance(input); if (paren_depth>0) paren_depth--; continue; }
                if (current_token.value) { strncat(idx_buf, current_token.value, sizeof(idx_buf)-strlen(idx_buf)-1); }
                advance(input);
            }
            if (!consume(input, TOKEN_BRACKET_CLOSE)) { printf("Error (line %d): Expected ']' after array index in expression\n", current_token.line); exit(EXIT_FAILURE); }
            ASTNode *node = create_node(NODE_EXPRESSION);
            char val_buf[700]; snprintf(val_buf, sizeof(val_buf), "%s[%s]", ident_buf, idx_buf); node->value = strdup(val_buf);
            if (is_number_str(idx_buf)) {
                int idx_val = atoi(idx_buf); int arr_size = find_array_size(ident_buf);
                if (arr_size > 0 && (idx_val < 0 || idx_val >= arr_size)) {
                    printf("Error (line %d): Array index %d out of bounds for '%s' with size %d\n", current_token.line, idx_val, ident_buf, arr_size);
                    exit(EXIT_FAILURE);
                }
            }
            return node;
        } else {
            ASTNode *node = create_node(NODE_EXPRESSION); node->value = strdup(ident_buf); return node;
        }
    }
    if (match(TOKEN_NUMBER)) {
        ASTNode *node = create_node(NODE_EXPRESSION); node->value = strdup(current_token.value); advance(input); return node;
    }
    return NULL;
}

ASTNode* parse_expression_prec(FILE *input, int min_prec) {
    ASTNode *left = parse_primary(input);
    if (!left) return NULL;
    while (match(TOKEN_OPERATOR)) {
        const char *op = current_token.value; int prec = get_precedence(op); if (prec < min_prec) break;
        char op_buf[8] = {0}; strncpy(op_buf, op, sizeof(op_buf)-1); advance(input);
        ASTNode *right = parse_expression_prec(input, prec + 1);
        if (!right) { printf("Error (line %d): Expected right operand after operator '%s'\n", current_token.line, op_buf); exit(EXIT_FAILURE);}        
        ASTNode *bin = create_node(NODE_BINARY_EXPR); bin->value = strdup(op_buf); add_child(bin, left); add_child(bin, right); left = bin;
    }
    return left;
}

ASTNode* parse_expression(FILE *input) { return parse_expression_prec(input, -2); }
