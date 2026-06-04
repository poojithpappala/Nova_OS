#include <stdlib.h>
#include <string.h>

typedef enum{
    TOKEN_WORD, //regular command
    TOKEN_REDIRECT_OUT, // >
    TOKEN_REDIRECT_IN, // <
    TOKEN_APPEND, // >>
    TOKEN_PIPE   // |
} TokenType;

typedef struct{
    TokenType type;
    char *value;
} Token;

Token* tokenize(const char *input, int *token_count){
    int capacity = 10;
    int count = 0;

    Token *tokens = malloc(capacity * sizeof(Token));

    int i =0;
    int len = strlen(input);

    while(i < len){

        if(input[i] == ' ' || input[i] == '\t'){
            i++;
            continue;
        }

        //resizing token array if it has reached the max capacity
        if(count >= capacity){
            capacity *= 2;
            tokens = realloc(tokens, capacity*sizeof(Token));
        }

        //handling the special characters
        if(input[i] == '>'){
            if(i+1<len && input[i+1] == '>'){
                tokens[count++] = (Token){TOKEN_APPEND, strdup(">>")};
                i+=2;
            }else{
                tokens[count++] = (Token){TOKEN_REDIRECT_OUT, strdup(">")};
                i++;
            }
            continue;
        }
        if(input[i] == '<'){
            tokens[count++] = (Token){TOKEN_REDIRECT_IN, strdup("<")};
            i++;
            continue;
        }

        if(input[i] == "|"){
            tokens[count++] = (Token){TOKEN_PIPE, strdup("|")};
        }

        //handling words and Quoted arguments
        char token_buf[256];
        int buf_idx = 0;
        int in_quotes = 0;
        char quote_char = 0;

        while(i< len){
            if(!in_quotes && (input[i] == '"' || input[i] == '\'')){
                in_quotes = 1;
                quote_char = input[i];
                i++; //skipping the opening quote
                continue;
            }
            if(in_quotes && input[i] == quote_char){
                in_quotes = 0;
                i++; //last quote 
                continue;
            }

            if(!in_quotes && (input[i] == ' ' || input[i] == '\t' || input[i]== '>' || input[i]== '<' || input[i]== '|')){
                break;
            }
            token_buf[buf_idx++] = input[i++];
            if(buf_idx>=255)break;//preventing the overflow of the buffer
        }

        token_buf[buf_idx] = '\0';
        if(buf_idx > 0){
            tokens[count++] = (Token){TOKEN_WORD, strdup(token_buf)};
        }
    }

    *token_count = count;
    return 
}