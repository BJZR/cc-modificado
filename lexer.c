#include <lexer.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <trie.h>

struct trie keywords = {0};
token_t *list;
token_t *head;

void lexer_push(token_type_t ty, char *lexeme)
{
    list->next = (token_t *) malloc(sizeof(token_t));
    list->next->ty = ty;
    list->next->lexeme = lexeme;

    list = list->next;
}

token_t *lexer_parse(char *source)
{
    trie_insert(&keywords, "typedef", TYPEDEF);
    trie_insert(&keywords, "struct", STRUCT);
    trie_insert(&keywords, "for", FOR);
    trie_insert(&keywords, "if", IF);
    trie_insert(&keywords, "while", WHILE);
    trie_insert(&keywords, "else", ELSE);
    trie_insert(&keywords, "continue", CONTINUE);
    trie_insert(&keywords, "case", CASE);
    trie_insert(&keywords, "switch", SWITCH);
    trie_insert(&keywords, "break", BREAK);
    trie_insert(&keywords, "goto", GOTO);
    trie_insert(&keywords, "static", STATIC);
    trie_insert(&keywords, "const", CONST);
    trie_insert(&keywords, "long", LONG);
    trie_insert(&keywords, "short", SHORT);
    trie_insert(&keywords, "unsigned", UNSIGNED);  // Fix typo
    trie_insert(&keywords, "char", CHAR);
    trie_insert(&keywords, "int", INT);
    trie_insert(&keywords, "float", FLOAT);
    trie_insert(&keywords, "double", DOUBLE);
    trie_insert(&keywords, "void", VOID);
    trie_insert(&keywords, "return", RETURN);
    trie_insert(&keywords, "asm", ASM);
    list = (token_t *) malloc(sizeof(token_t));
    head = list;
    bzero(list, sizeof(token_t));

    size_t len = strlen(source);// mejora #5  // Agrega esto ANTES del for (i=0; i < len; i++)
    for (size_t i=0; i < strlen(source); i++) {
        char current = source[i];
        char next;

        if (isspace(current)) {
            continue;
        }

        switch (current) {
            case '+':
                next = source[i+1];
                if (source[i+1] == '+') {
                    lexer_push(PLUS_PLUS, "++");
                    i++;
                } else if (source[i+1] == '=') {
                    lexer_push(PLUS_EQ, "+=");
                    i++;
                } else {
                    lexer_push(PLUS, "+");
                }
                break;
            case '-':
                next = source[i+1];
                if (source[i+1] == '-') {
                    lexer_push(MINUS_MINUS, "--");
                    i++;
                } else if (source[i+1] == '=') {
                    lexer_push(MINUS_EQ, "-=");
                    i++;
                } else if (source[i+1] == '>') {
                    lexer_push(ARROW, "->");
                    i++;
                } else {
                    lexer_push(MINUS, "-");
                }
                break;
            case '*':
                next = source[i+1];
                if (source[i+1] == '=') {
                    lexer_push(STAR_EQ, "*=");
                    i++;
                } else {
                    lexer_push(STAR, "*");
                }
                break;
            case '/':
                next = source[i+1];
                if (source[i+1] == '=') {
                    lexer_push(SLASH_EQ, "/=");
                    i++;
                } else if (source[i+1] == '/') {  // ¡NUEVO! Salta comentario single-line
                    i++;  // Salta el segundo /
                    while (i < strlen(source) && source[i] != '\n') {
                        i++;
                    }
                    // No push token —simplemente salta
                } else if (source[i+1] == '*') {  // Opcional: multi-line /* */
                    i += 2;  // Salta /*
                    while (i < strlen(source)) {
                        if (source[i] == '*' && i+1 < strlen(source) && source[i+1] == '/') {
                            i += 2;  // Salta */
                            break;
                        }
                        i++;
                    }
                } else {
                    lexer_push(SLASH, "/");
                }
                break;
            case '=':
                next = source[i+1];
                if (source[i+1] == '=') {
                    lexer_push(EQUAL, "==");
                    i++;
                } else {
                    lexer_push(ASSIGN, "=");
                }
                break;
            case '>':
                next = source[i+1];
                if (source[i+1] == '=') {
                    lexer_push(GREATER_EQ, ">=");
                    i++;
                } else {
                    lexer_push(GREATER, ">");
                }
                break;
            case '<':
                next = source[i+1];
                if (source[i+1] == '=') {
                    lexer_push(LESS_EQ, "<=");
                    i++;
                } else {
                    lexer_push(LESS, "<");
                }
                break;
            case '!':
                next = source[i+1];
                if (source[i+1] == '=') {
                    lexer_push(NOT_EQ, "!=");
                    i++;
                } else {
                    lexer_push(NOT, "!");
                }
                break;
            case '|':
                next = source[i+1];
                if (source[i+1] == '|') {
                    lexer_push(LOG_OR, "||");
                    i++;
                } else if (source[i+1] == '=') {
                    lexer_push(OR_EQ, "|=");
                    i++;
                } else {
                    lexer_push(OR, "|");
                }
                break;
            case '&':
                next = source[i+1];
                if (source[i+1] == '&') {
                    lexer_push(LOG_AND, "&&");
                    i++;
                } else if (source[i+1] == '=') {
                    lexer_push(AND_EQ, "&=");
                    i++;
                } else {
                    lexer_push(AND, "&");
                }
                break;
            case '^':
                next = source[i+1];
                if (source[i+1] == '=') {
                    lexer_push(XOR_EQ, "^=");
                    i++;
                } else {
                    lexer_push(XOR, "^");
                }
                break;
            case '(':
                lexer_push(L_PAREN, "(");
                break;
            case ')':
                lexer_push(R_PAREN, ")");
                break;
            case '[':
                lexer_push(L_SQUARE, "[");
                break;
            case ']':
                lexer_push(R_SQUARE, "]");
                break;
            case '{':
                lexer_push(L_CURLY, "{");
                break;
            case '}':
                lexer_push(R_CURLY, "}");
                break;
            case '.':
                lexer_push(DOT, ".");
                break;
            case ',':
                lexer_push(COMMA, ",");
                break;
            case ':':
                lexer_push(COLON, ":");
                break;
            case ';':
                lexer_push(SEMICOLON, ";");
                break;

        }

        if (isdigit(current)) {
            char *ptr = &source[i];
            size_t len = i;
            i++;
            current = source[i];

            while (isdigit(current)) {
                i++;
                current = source[i];
            }

            len = i - len;
            char *lexeme = malloc(sizeof(char) * (len+1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';
            lexer_push(NUMBER, lexeme);
            i--;
        }

        if (current == '"') {
            char *ptr = &source[i+1];
            size_t len = i+1;
            i++;
            current = source[i];

            while (current != '"') {
                i++;
                current = source[i];
            }

            len = i - len;
            char *lexeme = malloc(sizeof(char) * (len+1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';

            lexer_push(STRING, lexeme);
        }

        if (current == '#') {
            char *ptr = &source[i+1];
            size_t len = i+1;
            i++;
            current = source[i];

            while (current != '\n') {
                i++;
                current = source[i];
            }

            len = i - len;
            char *lexeme = malloc(sizeof(char) * (len+1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';

            lexer_push(MACRO, lexeme);
            i--;
        }

        if (isalpha(current)) {
            char *ptr = &source[i];
            size_t len = i;
            i++;
            current = source[i];

            while (isalnum(current)) {
                i++;
                current = source[i];
            }

            len = i - len;
            char *lexeme = malloc(sizeof(char) * (len+1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';
            token_type_t ty = trie_get(&keywords, lexeme);
            if (ty == -1) ty = IDENTIFIER;

            lexer_push(ty, lexeme);
            i--;
        }
    }

    return head;
}

