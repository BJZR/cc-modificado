#ifndef PARSER_H
#define PARSER_H

#include <lexer.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#define TYPE_POINTER 0x100
#define TYPE_MASK 0xff

struct literal_pair {
    char *literal;
    size_t label;
    struct literal_pair *next;
};

typedef enum op {
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_SLASH,

    OP_GREATER,
    OP_GREATER_EQ,
    OP_LESS,
    OP_LESS_EQ,
    OP_EQUAL,
    OP_NOT_EQ,

    OP_NOT,
} op_t;

typedef enum ast_type {
    EXPRESSION,
    INTEGER,
    DECIMAL,
    ID,
    STRING_LIT,
    UNARY,
    BINARY,
    FUNCTION,
    VAR_DEF,
    VAR_ASSIGN,
    CALL,
    RETURN_STATEMENT,
    INLINE_ASM,
    IF_STATEMENT,  // ¡NUEVO!
    WHILE_STATEMENT,
} ast_type_t;

typedef enum expr_type {
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    F32,
    F64,
    VOID_T,
    STRUC,

    MAX = UINT64_MAX,
} expr_type_t;

struct arg {
    char *identifier;
    expr_type_t ty;
};

typedef struct ast_node {
    ast_type_t ty;
    expr_type_t expr_ty;
    union ast_expr {
        uint64_t integer;
        double fl;
        char *identifier;
        size_t string;
        struct {
            op_t op;
            struct ast_node *node;
        } unary;

        struct {
            struct ast_node *left;
            struct ast_node *right;
            op_t op;
        } binary;
        struct {
            char *identifier;
            size_t arg_count;
            struct ast_node **args;
        } call;
    } expr;
} ast_node_t;

struct block_member;

typedef struct ast_statement {
    ast_type_t t;
    union {
        struct {
            ast_node_t *condition;
            struct block_member *block;
            // struct block_member *else_block;  // Para futuro
        } if_cond;
        struct {
            ast_node_t *condition;
            struct block_member *block;
        } while_cond;
        struct {
            expr_type_t ty;
            char *identifier;
            size_t arg_count;
            struct arg *args;
            struct block_member *block;
        } function;
        struct {
            expr_type_t ty;
            char *identifier;
            struct ast_statement *assignment;
        } var_def;
        struct {
            char *identifier;
            ast_node_t *value;
        } var_assign;

        struct {
            ast_node_t *value;
        } ret;
        struct {
            char *source;
            size_t value_count;
            ast_node_t **values;
        } asm;
        ast_node_t *expression;
    } statement;
} ast_statement_t;

struct block_member {
    ast_statement_t *value;
    size_t stack_size;
    struct block_member *next;
};

struct statement_list {
    ast_statement_t *statement;
    struct statement_list *next;
};

token_t *next(void);
uint8_t check_token(token_type_t ty);
struct statement_list *ast_parse(void);
ast_node_t *expression(void);
ast_node_t *comparison(void);  // Para manejar >, <, ==
ast_node_t *term(void);
ast_node_t *factor(void);
ast_node_t *ast_call(void);
ast_statement_t *ast_statement(void);
ast_statement_t *ast_function(void);
ast_statement_t *ast_variable(void);
ast_statement_t *ast_return(void);
struct block_member*ast_block(void);
expr_type_t ast_type(void);
size_t ast_type_size(expr_type_t ty);
void ast_walk(ast_node_t *ast);

#endif
