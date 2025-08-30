#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse_statement.h"
#include "parse_expression.h"
#include "symbol_structs.h"
#include "symbol_arrays.h"
#include "utils.h"
#include "parse.h" // create_node/add_child
#include "token.h"

extern Token current_token;
extern int g_array_count;
extern ArrayInfo g_arrays[128];

// Track loop depth (moved from parse.c; kept internal static here)
static int s_loop_depth = 0;

ASTNode* parse_statement(FILE *input) {
    ASTNode *stmt_node = NULL, *var_decl_node = NULL, *init_expr = NULL, *return_expr = NULL, *assign_node = NULL, *rhs_node = NULL;
    Token type_token, name_token, lhs_token;
    stmt_node = create_node(NODE_STATEMENT);

    if (match(TOKEN_KEYWORD) && (
            strcmp(current_token.value, "int") == 0 ||
            strcmp(current_token.value, "float") == 0 ||
            strcmp(current_token.value, "char") == 0 ||
            strcmp(current_token.value, "double") == 0 ||
            strcmp(current_token.value, "struct") == 0)) {
        type_token = current_token; advance(input);
        int is_struct = 0;
        if (strcmp(type_token.value, "struct") == 0) {
            if (!match(TOKEN_IDENTIFIER)) { printf("Error (line %d): Expected struct name after 'struct'\n", current_token.line); exit(EXIT_FAILURE);}            
            type_token = current_token; advance(input); is_struct = 1;
        }
        if (match(TOKEN_IDENTIFIER)) {
            name_token = current_token; advance(input);
            var_decl_node = create_node(NODE_VAR_DECL); var_decl_node->token = type_token; var_decl_node->value = strdup(name_token.value);
            int is_array = 0; char arr_size_buf[256] = {0};
            if (match(TOKEN_BRACKET_OPEN)) {
                is_array = 1; advance(input);
                if (match(TOKEN_NUMBER)) {
                    snprintf(arr_size_buf, sizeof(arr_size_buf), "%s", current_token.value);
                    char buf[1024]; snprintf(buf, sizeof(buf), "%s[%s]", name_token.value, current_token.value); free(var_decl_node->value); var_decl_node->value = strdup(buf); advance(input);
                    register_array(name_token.value, atoi(arr_size_buf));
                } else { printf("Error (line %d): Expected array size after '['\n", current_token.line); exit(EXIT_FAILURE);}                
                if (!consume(input, TOKEN_BRACKET_CLOSE)) { printf("Error (line %d): Expected ']' after array size\n", current_token.line); exit(EXIT_FAILURE);}            }
            if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) {
                advance(input);
                if (is_array && match(TOKEN_BRACE_OPEN)) {
                    advance(input); ASTNode *init_list = create_node(NODE_EXPRESSION); init_list->value = strdup("array_init");
                    while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
                        if (match(TOKEN_NUMBER) || match(TOKEN_IDENTIFIER)) { ASTNode *elem = create_node(NODE_EXPRESSION); elem->value = strdup(current_token.value); add_child(init_list, elem); advance(input);} else if (match(TOKEN_COMMA)) { advance(input);} else { advance(input);} }
                    if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after array initializer\n", current_token.line); exit(EXIT_FAILURE);}                    add_child(var_decl_node, init_list);
                } else if (is_struct && match(TOKEN_BRACE_OPEN)) {
                    advance(input); ASTNode *init_list = create_node(NODE_EXPRESSION); init_list->value = strdup("struct_init");
                    while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) {
                        if (match(TOKEN_NUMBER) || match(TOKEN_IDENTIFIER)) { ASTNode *elem = create_node(NODE_EXPRESSION); elem->value = strdup(current_token.value); add_child(init_list, elem); advance(input);} else if (match(TOKEN_COMMA)) { advance(input);} else { advance(input);} }
                    if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after struct initializer\n", current_token.line); exit(EXIT_FAILURE);}                    add_child(var_decl_node, init_list);
                } else { init_expr = parse_expression(input); if (init_expr) add_child(var_decl_node, init_expr); while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) advance(input); }
            }
            if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after variable declaration\n", current_token.line); exit(EXIT_FAILURE);}            add_child(stmt_node, var_decl_node); return stmt_node;
        } else { printf("Error (line %d): Expected variable name after type\n", current_token.line); exit(EXIT_FAILURE);}    }

    if (match(TOKEN_IDENTIFIER)) {
        lhs_token = current_token; advance(input);
        char lhs_buf[1024] = {0}; strncpy(lhs_buf, lhs_token.value, sizeof(lhs_buf)-1);
        while (match(TOKEN_OPERATOR) && current_token.value && strcmp(current_token.value, ".") == 0) {
            advance(input); if (!match(TOKEN_IDENTIFIER)) { printf("Error (line %d): Expected field name after '.' in assignment\n", current_token.line); exit(EXIT_FAILURE);}            strncat(lhs_buf, "__", sizeof(lhs_buf)-strlen(lhs_buf)-1); strncat(lhs_buf, current_token.value, sizeof(lhs_buf)-strlen(lhs_buf)-1); advance(input); }
        if (match(TOKEN_BRACKET_OPEN)) {
            advance(input); char idx_buf[512] = {0}; int paren_depth = 0; while (!match(TOKEN_EOF)) { if (match(TOKEN_BRACKET_CLOSE) && paren_depth == 0) break; if (match(TOKEN_PARENTHESIS_OPEN)) { strncat(idx_buf, "(", sizeof(idx_buf)-strlen(idx_buf)-1); advance(input); paren_depth++; continue; } if (match(TOKEN_PARENTHESIS_CLOSE)) { strncat(idx_buf, ")", sizeof(idx_buf)-strlen(idx_buf)-1); advance(input); if (paren_depth>0) paren_depth--; continue; } if (current_token.value) { strncat(idx_buf, current_token.value, sizeof(idx_buf)-strlen(idx_buf)-1);} advance(input);} if (!consume(input, TOKEN_BRACKET_CLOSE)) { printf("Error (line %d): Expected ']' after array index in assignment\n", current_token.line); exit(EXIT_FAILURE);}            size_t rem = sizeof(lhs_buf)-strlen(lhs_buf)-1; strncat(lhs_buf, "[", rem); rem = sizeof(lhs_buf)-strlen(lhs_buf)-1; strncat(lhs_buf, idx_buf, rem); rem = sizeof(lhs_buf)-strlen(lhs_buf)-1; strncat(lhs_buf, "]", rem); if (is_number_str(idx_buf)) { char base_name[128]={0}; const char *delim = strstr(lhs_buf, "__"); size_t len = delim ? (size_t)(delim - lhs_buf) : strcspn(lhs_buf, "["); if (len >= sizeof(base_name)) len = sizeof(base_name)-1; strncpy(base_name, lhs_buf, len); int arr_size = find_array_size(base_name); if (arr_size > 0) { int idx_val = atoi(idx_buf); if (idx_val < 0 || idx_val >= arr_size) { printf("Error (line %d): Array index %d out of bounds for '%s' with size %d\n", current_token.line, idx_val, base_name, arr_size); exit(EXIT_FAILURE);} } } }
        ASTNode *lhs_expr = create_node(NODE_EXPRESSION); lhs_expr->value = strdup(lhs_buf);
        if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) { advance(input); assign_node = create_node(NODE_ASSIGNMENT); add_child(assign_node, lhs_expr); rhs_node = parse_expression(input); if (rhs_node) add_child(assign_node, rhs_node); if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after assignment\n", current_token.line); exit(EXIT_FAILURE);}            add_child(stmt_node, assign_node); return stmt_node; } else { while (!match(TOKEN_SEMICOLON) && !match(TOKEN_EOF)) advance(input); if (match(TOKEN_SEMICOLON)) advance(input); free_node(lhs_expr); return stmt_node; }
    }

    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "return") == 0) {
        stmt_node->token = current_token; advance(input); return_expr = parse_expression(input); if (return_expr) add_child(stmt_node, return_expr); if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after return statement\n", current_token.line); exit(EXIT_FAILURE);}        return stmt_node;
    }

    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "if") == 0) {
        advance(input); if (!consume(input, TOKEN_PARENTHESIS_OPEN)) { printf("Error (line %d): Expected '(' after 'if'\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *cond_expr = parse_expression(input); if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) { printf("Error (line %d): Expected ')' after if condition\n", current_token.line); exit(EXIT_FAILURE);}        if (!consume(input, TOKEN_BRACE_OPEN)) { printf("Error (line %d): Expected '{' after if condition\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *if_node = create_node(NODE_IF_STATEMENT); if (cond_expr) add_child(if_node, cond_expr); while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) { ASTNode *inner_stmt = parse_statement(input); if (inner_stmt) add_child(if_node, inner_stmt);} if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after if block\n", current_token.line); exit(EXIT_FAILURE);}        while (match(TOKEN_KEYWORD) && strcmp(current_token.value, "else") == 0) { advance(input); if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "if") == 0) { advance(input); if (!consume(input, TOKEN_PARENTHESIS_OPEN)) { printf("Error (line %d): Expected '(' after 'else if'\n", current_token.line); exit(EXIT_FAILURE);} ASTNode *elseif_cond = parse_expression(input); if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) { printf("Error (line %d): Expected ')' after else if condition\n", current_token.line); exit(EXIT_FAILURE);} if (!consume(input, TOKEN_BRACE_OPEN)) { printf("Error (line %d): Expected '{' after else if condition\n", current_token.line); exit(EXIT_FAILURE);} ASTNode *elseif_node = create_node(NODE_ELSE_IF_STATEMENT); if (elseif_cond) add_child(elseif_node, elseif_cond); while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) { ASTNode *inner_stmt = parse_statement(input); if (inner_stmt) add_child(elseif_node, inner_stmt);} if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after else if block\n", current_token.line); exit(EXIT_FAILURE);} add_child(if_node, elseif_node); } else { if (!consume(input, TOKEN_BRACE_OPEN)) { printf("Error (line %d): Expected '{' after else\n", current_token.line); exit(EXIT_FAILURE);} ASTNode *else_node = create_node(NODE_ELSE_STATEMENT); while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) { ASTNode *inner_stmt = parse_statement(input); if (inner_stmt) add_child(else_node, inner_stmt);} if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after else block\n", current_token.line); exit(EXIT_FAILURE);} add_child(if_node, else_node); break; } }
        add_child(stmt_node, if_node); return stmt_node; }

    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "while") == 0) {
        advance(input); if (!consume(input, TOKEN_PARENTHESIS_OPEN)) { printf("Error (line %d): Expected '(' after 'while'\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *cond_expr = parse_expression(input); if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) { printf("Error (line %d): Expected ')' after while condition\n", current_token.line); exit(EXIT_FAILURE);}        if (!consume(input, TOKEN_BRACE_OPEN)) { printf("Error (line %d): Expected '{' after while condition\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *while_node = create_node(NODE_WHILE_STATEMENT); if (cond_expr) add_child(while_node, cond_expr); s_loop_depth++; while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) { ASTNode *inner_stmt = parse_statement(input); if (inner_stmt) add_child(while_node, inner_stmt);} s_loop_depth--; if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after while block\n", current_token.line); exit(EXIT_FAILURE);}        add_child(stmt_node, while_node); return stmt_node; }

    if (match(TOKEN_KEYWORD) && strcmp(current_token.value, "for") == 0) {
        advance(input); if (!consume(input, TOKEN_PARENTHESIS_OPEN)) { printf("Error (line %d): Expected '(' after 'for'\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *init_node = NULL; if (!match(TOKEN_SEMICOLON)) { long saved_pos = ftell(input); Token saved_token = current_token; if (match(TOKEN_KEYWORD) && (strcmp(current_token.value, "int") == 0 || strcmp(current_token.value, "float") == 0 || strcmp(current_token.value, "char") == 0 || strcmp(current_token.value, "double") == 0)) { ASTNode *init_stmt = parse_statement(input); if (init_stmt && init_stmt->num_children > 0) { ASTNode *child0 = init_stmt->children[0]; if (child0->type == NODE_VAR_DECL || child0->type == NODE_ASSIGNMENT) { init_node = child0; } } } else { if (match(TOKEN_IDENTIFIER)) { Token temp_lhs = current_token; advance(input); if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) { advance(input); ASTNode *assign = create_node(NODE_ASSIGNMENT); ASTNode *lhs_expr = create_node(NODE_EXPRESSION); lhs_expr->value = strdup(temp_lhs.value); add_child(assign, lhs_expr); ASTNode *rhs_expr = parse_expression(input); if (rhs_expr) add_child(assign, rhs_expr); if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after for-init assignment\n", current_token.line); exit(EXIT_FAILURE);} init_node = assign; } else { fseek(input, saved_pos, SEEK_SET); current_token = saved_token; } } } } if (match(TOKEN_SEMICOLON)) advance(input); ASTNode *cond_expr = NULL; if (!match(TOKEN_SEMICOLON)) { cond_expr = parse_expression(input); } if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after for condition\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *incr_expr = NULL; if (!match(TOKEN_PARENTHESIS_CLOSE)) { if (match(TOKEN_IDENTIFIER)) { Token inc_lhs = current_token; advance(input); if (match(TOKEN_OPERATOR) && (strcmp(current_token.value, "++") == 0 || strcmp(current_token.value, "--") == 0)) { incr_expr = create_node(NODE_ASSIGNMENT); ASTNode *lhs = create_node(NODE_EXPRESSION); lhs->value = strdup(inc_lhs.value); add_child(incr_expr, lhs); ASTNode *rhs = create_node(NODE_BINARY_EXPR); rhs->value = strdup(strcmp(current_token.value, "++") == 0 ? "+" : "-"); ASTNode *op_l = create_node(NODE_EXPRESSION); op_l->value = strdup(inc_lhs.value); ASTNode *op_r = create_node(NODE_EXPRESSION); op_r->value = strdup("1"); add_child(rhs, op_l); add_child(rhs, op_r); add_child(incr_expr, rhs); advance(input); } else if (match(TOKEN_OPERATOR) && strcmp(current_token.value, "=") == 0) { advance(input); incr_expr = create_node(NODE_ASSIGNMENT); ASTNode *lhs = create_node(NODE_EXPRESSION); lhs->value = strdup(inc_lhs.value); add_child(incr_expr, lhs); ASTNode *rhs = parse_expression(input); if (rhs) add_child(incr_expr, rhs); } else { /* unsupported pattern */ } } } if (!consume(input, TOKEN_PARENTHESIS_CLOSE)) { printf("Error (line %d): Expected ')' after for header\n", current_token.line); exit(EXIT_FAILURE);}        if (!consume(input, TOKEN_BRACE_OPEN)) { printf("Error (line %d): Expected '{' after for header\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *for_node = create_node(NODE_FOR_STATEMENT); if (init_node) add_child(for_node, init_node); if (cond_expr) add_child(for_node, cond_expr); else { ASTNode *true_expr = create_node(NODE_EXPRESSION); true_expr->value = strdup("1"); add_child(for_node, true_expr); } s_loop_depth++; while (!match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) { ASTNode *inner = parse_statement(input); if (inner) add_child(for_node, inner);} s_loop_depth--; if (!consume(input, TOKEN_BRACE_CLOSE)) { printf("Error (line %d): Expected '}' after for body\n", current_token.line); exit(EXIT_FAILURE);}        if (incr_expr) add_child(for_node, incr_expr); add_child(stmt_node, for_node); return stmt_node; }

    if ((match(TOKEN_KEYWORD) && strcmp(current_token.value, "break") == 0) || (match(TOKEN_IDENTIFIER) && strcmp(current_token.value, "break") == 0)) {
        if (s_loop_depth <= 0) { printf("Error (line %d): 'break' not within a loop\n", current_token.line); exit(EXIT_FAILURE);}        advance(input); if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after 'break'\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *br = create_node(NODE_BREAK_STATEMENT); add_child(stmt_node, br); return stmt_node; }

    if ((match(TOKEN_KEYWORD) && strcmp(current_token.value, "continue") == 0) || (match(TOKEN_IDENTIFIER) && strcmp(current_token.value, "continue") == 0)) {
        if (s_loop_depth <= 0) { printf("Error (line %d): 'continue' not within a loop\n", current_token.line); exit(EXIT_FAILURE);}        advance(input); if (!consume(input, TOKEN_SEMICOLON)) { printf("Error (line %d): Expected ';' after 'continue'\n", current_token.line); exit(EXIT_FAILURE);}        ASTNode *cn = create_node(NODE_CONTINUE_STATEMENT); add_child(stmt_node, cn); return stmt_node; }

    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_BRACE_CLOSE) && !match(TOKEN_EOF)) advance(input);
    if (match(TOKEN_SEMICOLON)) advance(input);
    return stmt_node;
}
