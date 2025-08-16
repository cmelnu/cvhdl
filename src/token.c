#include "token.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Current token and functions to manage the token stream
Token current_token;

// List of C keywords
const char *keywords[] = {
    "if", "else", "while", "for", "return",
    "int", "float", "char", "double", "void",
    NULL
};

// Get the next token and update current_token
void advance(FILE *input) {
    current_token = get_next_token(input);
}


// Check if current token matches expected type
int match(TokenType type) {
    return current_token.type == type;
}


// Consume the current token if it matches expected type
int consume(FILE *input, TokenType type) {

    if (match(type)) {
        advance(input);
        return 1;
    }
    return 0;
}

// Check if a string is a keyword
int is_keyword(const char *str) {

    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Get the next token from input
Token get_next_token(FILE *input) {
    
    Token token;
    int c;

    // Skip whitespace
    while ((c = fgetc(input)) != EOF && isspace(c));
    if (c == EOF) {
        token.type = TOKEN_EOF;
        token.value[0] = '\0';
        return token;
    }

    // Handle comments starting with // or /* */
    if (c == '/') {
        int d = fgetc(input);
        if (d == '/') {
            // Line comment
            while ((c = fgetc(input)) != EOF && c != '\n');
            return get_next_token(input);
        } else if (d == '*') {
            // Block comment
            int prev = 0;
            while ((c = fgetc(input)) != EOF) {
                if (prev == '*' && c == '/') break;
                prev = c;
            }
            return get_next_token(input);
        } else {
            // It's just the division operator
            if (d != EOF) ungetc(d, input);
            token.type = TOKEN_OPERATOR;
            token.value[0] = '/';
            token.value[1] = '\0';
            return token;
        }
    }

    // Identifier or keyword
    if (isalpha(c) || c == '_') {

        int i = 0;
        token.value[i++] = c;

        while ((c = fgetc(input)) != EOF && (isalnum(c) || c == '_')) {
            if (i < 255) token.value[i++] = c;
        }
        token.value[i] = '\0';

        if (c != EOF) ungetc(c, input);

        token.type = is_keyword(token.value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
        return token;

    } 
    // Number
    else if (isdigit(c)) {

        int i = 0;
        token.value[i++] = c;

        while ((c = fgetc(input)) != EOF && (isdigit(c) || c == '.')) {
            if (i < 255) token.value[i++] = c;
        }
        token.value[i] = '\0';

        if (c != EOF) ungetc(c, input);

        token.type = TOKEN_NUMBER;
        return token;
    } 
    // Operators and punctuation (including multi-char ops)
    else {
        int d = fgetc(input);
        token.value[0] = (char)c;
        token.value[1] = '\0';
        token.value[2] = '\0';

        // Punctuation
        switch (c) {
            case ';': token.type = TOKEN_SEMICOLON; if (d != EOF) ungetc(d, input); return token;
            case '(': token.type = TOKEN_PARENTHESIS_OPEN; if (d != EOF) ungetc(d, input); return token;
            case ')': token.type = TOKEN_PARENTHESIS_CLOSE; if (d != EOF) ungetc(d, input); return token;
            case '{': token.type = TOKEN_BRACE_OPEN; if (d != EOF) ungetc(d, input); return token;
            case '}': token.type = TOKEN_BRACE_CLOSE; if (d != EOF) ungetc(d, input); return token;
            case '[': token.type = TOKEN_BRACKET_OPEN; if (d != EOF) ungetc(d, input); return token;
            case ']': token.type = TOKEN_BRACKET_CLOSE; if (d != EOF) ungetc(d, input); return token;
            case ',': token.type = TOKEN_COMMA; if (d != EOF) ungetc(d, input); return token;
            default: break;
        }

        // Multi-character operators: ==, !=, <=, >=, <<, >>
        token.type = TOKEN_OPERATOR;
        if (c == '=' && d == '=') {
            token.value[0] = '=';
            token.value[1] = '=';
            token.value[2] = '\0';
        } else if (c == '!' && d == '=') {
            token.value[0] = '!';
            token.value[1] = '=';
            token.value[2] = '\0';
        } else if (c == '<' && d == '=') {
            token.value[0] = '<';
            token.value[1] = '=';
            token.value[2] = '\0';
        } else if (c == '>' && d == '=') {
            token.value[0] = '>';
            token.value[1] = '=';
            token.value[2] = '\0';
        } else if (c == '<' && d == '<') {
            token.value[0] = '<';
            token.value[1] = '<';
            token.value[2] = '\0';
        } else if (c == '>' && d == '>') {
            token.value[0] = '>';
            token.value[1] = '>';
            token.value[2] = '\0';
        } else {
            // Single-character operator
            if (d != EOF) ungetc(d, input);
            token.value[0] = (char)c;
            token.value[1] = '\0';
        }

        return token;
    }
}