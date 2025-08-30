#ifndef PARSE_EXPRESSION_H
#define PARSE_EXPRESSION_H

#include <stdio.h>
#include "astnode.h"

ASTNode* parse_primary(FILE *input);
ASTNode* parse_expression_prec(FILE *input, int min_prec);
ASTNode* parse_expression(FILE *input);

#endif // PARSE_EXPRESSION_H
