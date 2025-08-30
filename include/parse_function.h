#ifndef PARSE_FUNCTION_H
#define PARSE_FUNCTION_H

#include <stdio.h>
#include "astnode.h"
#include "token.h"

ASTNode* parse_function(FILE *input, Token return_type, Token func_name);

#endif // PARSE_FUNCTION_H
