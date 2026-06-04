#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

typedef enum {
    TOKEN_WORD,          // regular command
    TOKEN_REDIRECT_OUT,  // >
    TOKEN_REDIRECT_IN,   // <
    TOKEN_APPEND,        // >>
    TOKEN_PIPE           // |
} TokenType;

typedef struct {
    TokenType type;
    char *value;
}Token;

Token* tokenize(const char *input, int *token_count);

#endif