#ifndef TOKEN_H
#define TOKEN_H
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Token types
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_SEMICOLON,
    TOKEN_PARENTHESIS_OPEN,
    TOKEN_PARENTHESIS_CLOSE,
    TOKEN_BRACE_OPEN,
    TOKEN_BRACE_CLOSE,
    TOKEN_BRACKET_OPEN,
    TOKEN_BRACKET_CLOSE,
    TOKEN_COMMA,
    TOKEN_EOF
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char value[256];
    int line; // Add this field
} Token;

// Current token (shared parsing state - to be moved into ParserContext later)
extern Token current_token;

// Keyword check
int is_keyword(const char *str);

// Tokenizer function
Token get_next_token(FILE* input);

void advance(FILE *input);
int match(TokenType type);
int consume(FILE *input, TokenType type);

extern int current_line;

#endif // TOKEN_H