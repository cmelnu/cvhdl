#ifndef UTILS_H
#define UTILS_H

#include "astnode.h"  // ASTNode definition

char* ctype_to_vhdl(const char* ctype);
void print_ast(ASTNode* node, int level);
int is_number_str(const char *s);
int is_negative_literal(const char* value);
int get_precedence(const char *op);

#endif