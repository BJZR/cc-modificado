#ifndef LEXER_H
#define LEXER_H

typedef enum token_type {
    // literals
    NUMBER,
    IDENTIFIER,
    STRING,

    // arithmetic operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    MOD,

    PLUS_EQ,
    MINUS_EQ,
    STAR_EQ,
    SLASH_EQ,
    PLUS_PLUS,
    MINUS_MINUS,
    ASSIGN,

    // logic operators
    GREATER,
    LESS,
    GREATER_EQ,
    LESS_EQ,
    NOT_EQ,
    EQUAL,
    LOG_OR,
    LOG_AND,

    // bit operators
    NOT,
    OR,
    OR_EQ,
    AND,
    AND_EQ,
    XOR,
    XOR_EQ,

    // keywords
    TYPEDEF,
    STRUCT,
    FOR,
    IF,
    WHILE,
    ELSE,
    CONTINUE,
    CASE,
    SWITCH,
    BREAK,
    GOTO,
    STATIC,
    CONST,
    LONG,
    SHORT,
    UNSIGNED,
    CHAR,
    INT,
    FLOAT,
    DOUBLE,
    VOID,
    RETURN,
    ASM,

    // special
    L_PAREN,
    R_PAREN,
    L_SQUARE,
    R_SQUARE,
    L_CURLY,
    R_CURLY,
    COMMA,
    DOT,
    ARROW,
    COLON,
    SEMICOLON,

    MACRO,
} token_type_t;

typedef struct token {
    token_type_t ty;
    char *lexeme;
    struct token *next;
} token_t;

token_t *lexer_parse(char *source);
void lexer_push(token_type_t ty, char *lexeme);

#endif
