#ifndef CODEGEN_H
#define CODEGEN_H

#include <parser.h>
#include <stdio.h>

struct scope {
    size_t stack_base;
    size_t stack_pointer;

    struct scope *next;
    struct scope *prev;
};

void gen_def(FILE *f, ast_statement_t *statement);
void gen_assignment(FILE *f, ast_statement_t *assignment);
void gen_mov(FILE *f, ast_node_t *expr, const char *reg);
void gen_function(FILE *f, ast_statement_t *statement);
void gen_block(FILE *f, struct block_member *block);
void gen_return(FILE *f, ast_statement_t *statement);
void gen_statement(FILE *f, ast_statement_t *statement);
void gen_asm(FILE *f, struct statement_list* statements);

#endif
