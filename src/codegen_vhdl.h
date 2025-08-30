#ifndef CODEGEN_VHDL_H
#define CODEGEN_VHDL_H

#include <stdio.h>
#include "astnode.h"

// Generate VHDL code from an AST root node
void generate_vhdl(ASTNode* node, FILE* output);

#endif // CODEGEN_VHDL_H
