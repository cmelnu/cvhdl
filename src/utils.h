#ifndef UTILS_H
#define UTILS_H

#include "parse.h"     // For ASTNode definition
#include <stdio.h>   // For printf (used in print_ast and print_indent)
#include <string.h>  // For strcmp (used in ctype_to_vhdl)

char* ctype_to_vhdl(const char* ctype);
void print_ast(ASTNode* node, int level);
void print_indent(int level);

#endif