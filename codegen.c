#include <codegen.h>
#include <stddef.h>
#include <stdlib.h>
#include <trie.h>
#include <string.h>

extern struct literal_pair *literals_head;

static const char *call_regs[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
};

static struct scope *scope_stack = NULL;
static int main_function = 0;

struct trie *variables = NULL;
static size_t str_count = 0;
static size_t base_offset = 0;

void push_scope(size_t base, size_t pointer)
{
    struct scope *scope = (struct scope *) malloc(sizeof(struct scope));
    scope->stack_base = base;
    scope->stack_pointer = pointer;

    scope->prev = scope_stack;
    scope_stack->next = scope;
    scope_stack = scope;
}

struct scope *pop_scope()
{
    struct scope *tmp = scope_stack;
    scope_stack = scope_stack->prev;
    tmp->prev = NULL;
    return tmp;
}

void gen_literals(FILE *f)
{
    fprintf(f, ".data\n");
    while (literals_head != NULL) {
        fprintf(f, "str%lu: .asciz \"%s\"\n", literals_head->label, literals_head->literal);

        literals_head = literals_head->next;
    }
}

void gen_block(FILE *f, struct block_member *block)
{
    while (block != NULL) {
        gen_statement(f, block->value);

        block = block->next;
    }
}

void gen_assignment(FILE *f, ast_statement_t *assignment)
{
    gen_mov(f, assignment->statement.var_assign.value, "r10");
    int32_t offset = trie_get(variables, assignment->statement.var_assign.identifier);// mejora #4
    fprintf(f, "mov %%r10, %d(%%rbp)\n", offset);
}

void gen_def(FILE *f, ast_statement_t *statement)
{
    int offset = -base_offset;
    trie_insert(variables, statement->statement.var_def.identifier, offset);
    base_offset += ast_type_size(statement->statement.var_def.ty);

    if (statement->statement.var_def.assignment != NULL) {
        gen_assignment(f, statement->statement.var_def.assignment);
    }
}

void gen_mov(FILE *f, ast_node_t *expr, const char *reg)
{
    if (expr == NULL) return;

    if (expr->ty == INTEGER) {
        fprintf(f, "mov $%lu, %%%s\n", expr->expr.integer, reg);
    } else if (expr->ty == STRING_LIT) {
        fprintf(f, "mov $str%lu, %%%s\n", expr->expr.string, reg);
    } else if (expr->ty == ID) { // mejora #3
        int32_t offset = trie_get(variables, expr->expr.identifier);
        fprintf(f, "mov %d(%%rbp), %%%s\n", offset, reg);
    } else if (expr->ty == CALL) {
        for (int i = 0; i < (int)expr->expr.call.arg_count; i++) {
            gen_mov(f, expr->expr.call.args[i], call_regs[i]);
        }

        fprintf(f, "call %s\n", expr->expr.call.identifier);
        fprintf(f, "mov %%rax, %%%s\n", reg);
    } else if (expr->ty == BINARY) {
        gen_mov(f, expr->expr.binary.left, "r15");
        gen_mov(f, expr->expr.binary.right, reg);
        switch (expr->expr.binary.op) {
            case OP_PLUS:
                fprintf(f, "add %%r15, %%%s\n", reg);
                break;
            case OP_MINUS:
                fprintf(f, "sub %%r15, %%%s\n", reg);
                break;
            case OP_STAR:
                fprintf(f, "mul %%r15, %%%s\n", reg);
                break;
            case OP_SLASH:
                fprintf(f, "div %%r15, %%%s\n", reg);
                break;
            case OP_GREATER:
                gen_mov(f, expr->expr.binary.left, "r15");
                gen_mov(f, expr->expr.binary.right, "r10");
                fprintf(f, "cmp %%%s, %%r15\n", "r10");  // cmp right, left
                fprintf(f, "setg %%al\n");
                fprintf(f, "movzb %%al, %%%s\n", reg);
                break;
            case OP_LESS:
                gen_mov(f, expr->expr.binary.left, "r15");
                gen_mov(f, expr->expr.binary.right, "r10");
                fprintf(f, "cmp %%%s, %%r15\n", "r10");
                fprintf(f, "setl %%al\n");
                fprintf(f, "movzb %%al, %%%s\n", reg);
                break;
            case OP_EQUAL:
                gen_mov(f, expr->expr.binary.left, "r15");
                gen_mov(f, expr->expr.binary.right, "r10");
                fprintf(f, "cmp %%%s, %%r15\n", "r10");
                fprintf(f, "sete %%al\n");
                fprintf(f, "movzb %%al, %%%s\n", reg);
                break;
                // Agrega OP_GREATER_EQ (setge), OP_LESS_EQ (setle), OP_NOT_EQ (setne), OP_NOT (not %reg)
            default:
                // TODO: Ignora o error
                break;
                // Agrega más: OP_GREATER_EQ (setge), OP_LESS (setl), etc.
        }
    }
}

void gen_expr(FILE *f, ast_node_t *expr) {
    if (expr->ty == CALL) {
        for (int i = 0; i < (int)expr->expr.call.arg_count; i++) {
            gen_mov(f, expr->expr.call.args[i], call_regs[i]);
        }

        fprintf(f, "call %s\n", expr->expr.call.identifier);
    }
}

void gen_function(FILE *f, ast_statement_t *statement)
{
    if (statement->statement.function.block == NULL) {
        return;
    }

    variables = (struct trie *) malloc(sizeof(struct trie));
    memset(variables, 0x0, sizeof(struct trie));
    base_offset = 0;

    if (strcmp(statement->statement.function.identifier, "main") == 0) {
        main_function = 1;
    }



    fprintf(f, "%s:\n", statement->statement.function.identifier);
    fprintf(f, "push %%rbp\n");// mejora #1
    fprintf(f, "mov %%rsp, %%rbp\n");
    if (statement->statement.function.block != NULL) {
        fprintf(f, "sub $0x%lx, %%rsp\n", statement->statement.function.block->stack_size);
        int arg_offset = 16;// mejora #2  // Primer arg en 16(%rbp) según ABI (después de saved rbp=0, ret addr=8)
        for (int i = 0; i < (int)statement->statement.function.arg_count; i++) {
            struct arg a = statement->statement.function.args[i];
            trie_insert(variables, a.identifier, arg_offset);
            fprintf(f, "mov %%%s, %d(%%rbp)\n", call_regs[i], arg_offset);
            arg_offset += 8;  // Slots de 8 bytes (pad para alineación, incluso si size=4)
        }
        base_offset = 8;  // Para locals: primer local en -8(%rbp), etc.

        gen_block(f, statement->statement.function.block);
    }

    free(variables);
    if (statement->statement.function.ty == VOID_T) {
        fprintf(f, "leave\nret\n");
    }
}

void gen_return(FILE *f, ast_statement_t *statement)
{
    if (main_function) {
        main_function = 0;
        fprintf(f, "leave\n");
        gen_mov(f, statement->statement.ret.value, "rdi");
        fprintf(f, "mov $0x3c, %%rax\nsyscall\n");
    } else {
        gen_mov(f, statement->statement.ret.value, "rax");
        fprintf(f, "leave\nret\n");
    }
}

void gen_statement(FILE *f, ast_statement_t *statement)
{
    if (statement != NULL) {
        ast_type_t ty = statement->t;
        switch (ty) {  // Mejor que if-else chain para escalar
            case FUNCTION:
                gen_function(f, statement);
                break;
            case VAR_DEF:
                gen_def(f, statement);
                break;
            case VAR_ASSIGN:
                gen_assignment(f, statement);
                break;
            case EXPRESSION:
                gen_expr(f, statement->statement.expression);
                break;
            case RETURN_STATEMENT:
                gen_return(f, statement);
                break;
            case IF_STATEMENT:  // ¡NUEVO! Tu código aquí
            {
                static int label_count = 0;
                char end_label[32];
                sprintf(end_label, "end_if_%d", label_count);
                char else_label[32];
                sprintf(else_label, "else_%d", label_count);
                label_count++;  // Incrementa al final

                gen_mov(f, statement->statement.if_cond.condition, "r10");  // Eval cond en r10
                fprintf(f, "cmp $0, %%r10\n");  // cmp con 0 (para bool)
                fprintf(f, "je %s\n", else_label);  // Salta si false
                gen_block(f, statement->statement.if_cond.block);  // Ejecuta block
                fprintf(f, "jmp %s\n", end_label);  // Salta fin
                fprintf(f, "%s:\n", else_label);  // Label else (vacío)
                fprintf(f, "%s:\n", end_label);  // Label end
            }
            break;
            case WHILE_STATEMENT:
            {
                static int label_count = 0;
                char loop_label[32];
                char end_label[32];
                sprintf(loop_label, "loop_%d", label_count);
                sprintf(end_label, "end_while_%d", label_count);
                label_count++;

                fprintf(f, "%s:\n", loop_label);  // Label inicio loop
                gen_mov(f, statement->statement.while_cond.condition, "r10");  // Eval cond
                fprintf(f, "cmp $0, %%r10\n");
                fprintf(f, "je %s\n", end_label);  // Salta si false
                gen_block(f, statement->statement.while_cond.block);  // Ejecuta block
                fprintf(f, "jmp %s\n", loop_label);  // Salta inicio
                fprintf(f, "%s:\n", end_label);  // Label fin
            }
            break;
            default:
                // TODO: Ignora o error para types no manejados
                break;
        }
    }
}


void gen_asm(FILE *f, struct statement_list *statements)
{
    gen_literals(f);

    scope_stack = (struct scope *) malloc(sizeof(struct scope));

    fprintf(f, ".text\n.global main\n");

    while (statements != NULL) {
        gen_statement(f, statements->statement);

        statements = statements->next;
    }

    while (scope_stack) {
        struct scope *tmp = scope_stack->prev;
        scope_stack = scope_stack->prev;
        free(tmp);
    }
}
