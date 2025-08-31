#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "utils.h"
#include "parse_expression.h"
#include "symbol_arrays.h"
#include "symbol_structs.h"

// Safe append helper to avoid strncat truncation warnings
static inline void safe_append(char *dst, size_t dst_size, const char *src) {
    size_t used = strlen(dst);
    if (used >= dst_size - 1) {
        return;
    }
    size_t avail = dst_size - 1 - used;
    size_t copy = strlen(src);
    if (copy > avail) {
        copy = avail;
    }
    memcpy(dst + used, src, copy);
    dst[used + copy] = '\0';
}

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

// Primary: identifiers, numbers, unary minus, logical/bitwise NOT, parentheses, field & array access
ASTNode* parse_primary(FILE *input) {
    ASTNode *inner = NULL;
    ASTNode *node = NULL;
    ASTNode *zero = NULL;
    ASTNode *bin = NULL;
    char buf[128] = {0};
    char ident_buf[128] = {0};
    char idx_buf[512] = {0};
    char val_buf[700] = {0};
    int paren_depth = 0;
    int idx_val = 0;
    int arr_size = 0;

    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "!") == 0) {
        advance(input);
        inner = parse_primary(input);
        if (!inner) {
            return NULL;
        }
        node = create_node(NODE_BINARY_OP);
        node->value = strdup("!");
        add_child(node, inner);
        return node;
    }
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "~") == 0) {
        advance(input);
        inner = parse_primary(input);
        if (!inner) {
            return NULL;
        }
        node = create_node(NODE_BINARY_OP);
        node->value = strdup("~");
        add_child(node, inner);
        return node;
    }
    if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "-") == 0) {
        advance(input);
        inner = parse_primary(input);
        if (!inner) {
            return NULL;
        }
        if (inner->type == NODE_EXPRESSION && inner->value) {
            snprintf(buf, sizeof(buf), "-%s", inner->value);
            node = create_node(NODE_EXPRESSION);
            node->value = strdup(buf);
            free_node(inner);
            return node;
        } else {
            zero = create_node(NODE_EXPRESSION);
            zero->value = strdup("0");
            bin = create_node(NODE_BINARY_EXPR);
            bin->value = strdup("-");
            add_child(bin, zero);
            add_child(bin, inner);
            return bin;
        }
    }
    if (match(TOKEN_PARENTHESIS_OPEN)) {
        advance(input);
        node = parse_expression_prec(input, 1);
        if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) {
            printf("Error (line %d): Expected ')' after expression\n", current_token.line);
            exit(EXIT_FAILURE);
        }
        return node;
    }
    if (match(TOKEN_IDENTIFIER)) {
        safe_copy(ident_buf, sizeof(ident_buf), current_token.value);
        advance(input);
        while (match(TOKEN_OPERATOR) && strcmp(current_token.value, ".") == 0) {
            advance(input);
            if (!match(TOKEN_IDENTIFIER)) {
                printf("Error (line %d): Expected field name after '.'\n", current_token.line);
                exit(EXIT_FAILURE);
            }
            safe_append(ident_buf, sizeof(ident_buf), "__");
            safe_append(ident_buf, sizeof(ident_buf), current_token.value);
            advance(input);
        }
        if (match(TOKEN_BRACKET_OPEN)) {
            advance(input);
            paren_depth = 0;
            while (!match(TOKEN_EOF)) {
                if (match(TOKEN_BRACKET_CLOSE) && paren_depth == 0) {
                    break;
                }
                if (match(TOKEN_PARENTHESIS_OPEN)) {
                    safe_append(idx_buf, sizeof(idx_buf), "(");
                    advance(input);
                    paren_depth++;
                    continue;
                }
                if (match(TOKEN_PARENTHESIS_CLOSE)) {
                    safe_append(idx_buf, sizeof(idx_buf), ")");
                    advance(input);
                    if (paren_depth > 0) {
                        paren_depth--;
                    }
                    continue;
                }
                if (current_token.value) {
                    safe_append(idx_buf, sizeof(idx_buf), current_token.value);
                }
                advance(input);
            }
            if (!consume(input, TOKEN_BRACKET_CLOSE)) {
                printf("Error (line %d): Expected ']' after array index in expression\n", current_token.line);
                exit(EXIT_FAILURE);
            }
            node = create_node(NODE_EXPRESSION);
            snprintf(val_buf, sizeof(val_buf), "%s[%s]", ident_buf, idx_buf);
            node->value = strdup(val_buf);
            if (is_number_str(idx_buf)) {
                idx_val = atoi(idx_buf);
                arr_size = find_array_size(ident_buf);
                if (arr_size > 0 && (idx_val < 0 || idx_val >= arr_size)) {
                    printf("Error (line %d): Array index %d out of bounds for '%s' with size %d\n", current_token.line, idx_val, ident_buf, arr_size);
                    exit(EXIT_FAILURE);
                }
            }
            return node;
        } else {
            node = create_node(NODE_EXPRESSION);
            node->value = strdup(ident_buf);
            return node;
        }
    }
    if (match(TOKEN_NUMBER)) {
        node = create_node(NODE_EXPRESSION);
        node->value = strdup(current_token.value);
        advance(input);
        return node;
    }
    return NULL;
}

ASTNode* parse_expression_prec(FILE *input, int min_prec) {
    ASTNode *left = NULL;
    ASTNode *right = NULL;
    ASTNode *bin = NULL;
    const char *op = NULL;
    int prec = 0;
    char op_buf[8] = {0};

    left = parse_primary(input);
    if (!left) {
        return NULL;
    }
    while (match(TOKEN_OPERATOR)) {
        op = current_token.value;
        prec = get_precedence(op);
        if (prec < min_prec) {
            break;
        }
        safe_copy(op_buf, sizeof(op_buf), op);
        advance(input);
        right = parse_expression_prec(input, prec + 1);
        if (!right) {
            printf("Error (line %d): Expected right operand after operator '%s'\n", current_token.line, op_buf);
            exit(EXIT_FAILURE);
        }
        bin = create_node(NODE_BINARY_EXPR);
        bin->value = strdup(op_buf);
        add_child(bin, left);
        add_child(bin, right);
        left = bin;
    }
    return left;
}

ASTNode* parse_expression(FILE *input) { 
    return parse_expression_prec(input, -2); 
}
