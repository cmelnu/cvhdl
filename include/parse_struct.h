#ifndef PARSE_STRUCT_H
#define PARSE_STRUCT_H

#include <stdio.h>
#include "astnode.h"
#include "token.h"

// Parse struct definition: struct Name { ... };
ASTNode* parse_struct(FILE *input, Token struct_name_tok);

#endif // PARSE_STRUCT_H
