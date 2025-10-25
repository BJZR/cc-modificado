#include <parser.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <trie.h>

extern token_t *list;
extern token_t *head;
struct literal_pair *literals_head;
static size_t str_count;
static struct trie functions;


token_t *next(void)
{
    list = list->next;
    return list;
}

uint8_t check_consume(token_type_t ty)
{
    if (!list) return 0;
    if (list->ty == ty) {
        list = list->next;
        return 1;
    }

    return 0;
}

uint8_t check_next(token_type_t ty)
{
    if (!(list->next)) return 0;
    return list->next->ty == ty;
}

uint8_t check_token(token_type_t ty)
{
    if (!list) return 0;
    return list->ty == ty;
}

struct statement_list *ast_parse(void)
{
    list = head;
    list = list->next;

    struct statement_list *statements = (struct statement_list *) malloc(sizeof(struct statement_list));
    struct statement_list *statements_head = statements;

    while (list != NULL) {
        statements->statement = ast_statement();
        statements->next = (struct statement_list *) malloc(sizeof(struct statement_list));
        statements = statements->next;
    }

    return statements_head;
}

ast_statement_t *ast_function(void)
{
    ast_statement_t *fn = malloc(sizeof(ast_statement_t));
    fn->t = FUNCTION;
    fn->statement.function.ty = ast_type();

    if (!check_token(IDENTIFIER)) {
        // TODO: error
    }
    fn->statement.function.identifier = list->lexeme;
    next();
    check_consume(L_PAREN);

    size_t stack_size = 0;

    token_t *tmp = list;
    expr_type_t arg_type = ast_type();
    if ((arg_type & TYPE_MASK) == VOID_T) {
        if (arg_type & TYPE_POINTER) {
            goto check_args;
        } else {
            check_consume(R_PAREN);
            fn->statement.function.arg_count = 0;
        }
    } else if (!check_consume(R_PAREN)) {
        check_args:
        list = tmp;
        size_t arg_count = 0;
        while (!check_token(R_PAREN)) {
            if (check_consume(COMMA)) {
                arg_count += 1;
            } else {
                next();
            }
        }
        arg_count += 1;
        fn->statement.function.arg_count = arg_count;
        fn->statement.function.args = (struct arg *) malloc(sizeof(struct arg) * arg_count);
        struct arg *args = fn->statement.function.args;

        list = tmp;
        for (size_t i=0; !check_consume(R_PAREN); i++) {
            args[i].ty = ast_type();
            stack_size += ast_type_size(args[i].ty);
            if (!check_token(IDENTIFIER)) {
                args[i].identifier = NULL;
            } else {
                args[i].identifier = list->lexeme;
                next();
            }

            check_consume(COMMA);
        }
    } else {
        fn->statement.function.arg_count = 0;
    }

    if (check_consume(SEMICOLON)) {
        fn->statement.function.block = NULL;
    } else if (check_token(L_CURLY)) {
        fn->statement.function.block = ast_block();
        fn->statement.function.block->stack_size += stack_size;
    } else if (check_consume(SEMICOLON)) {
        fn->statement.function.block = NULL;
    } else {
        // TODO: error
    }

    trie_insert(&functions, fn->statement.function.identifier, fn->statement.function.ty);

    return fn;
}

// TODO: implement this
/*
 * ast_statement_t ast_asm(void)
 * {
 *    if (!check_consume(L_PAREN)) {
 *        // TODO: error
 *    }
 *
 *    ast_statement_t *inline_asm = (ast_statement_t *) malloc(sizeof(ast_statement_t));
 *    inline_asm->t = INLINE_ASM;
 *    if (!check_token(STRING)) {
 *        // TODO: error
 *    }
 *    inline_asm->statement.asm.source = list->lexeme;
 *    next();
 *
 *    if (!check_consume(COLON)) {
 *        // TODO: error
 *    }
 *
 *    size_t value_count = 0;
 *    token_t *tmp = list;
 *    while (!check_token(R_PAREN)) {
 *        if (check_consume(COMMA)) {
 *            value_count += 1;
 *        } else {
 *            next();
 *        }
 *    }
 *
 *    list = tmp;
 *    arg_count += 1;
 *    inline_asm->statement.asm.value_count = arg_count;
 *    inline_asm->statement.asm.values = (ast_node_t **) malloc(sizeof(ast_node_t *) * value_count);
 *    ast_node_t **values = inline_asm->statement.asm.values;
 *
 *    for (size_t i=0; !check_consume(R_PAREN); i++) {
 *        values[i] = expression();
 *
 *        check_consume(COMMA);
 *    }
 *
 *    if (!check_consume(R_PAREN)) {
 *        // TODO: error
 *    }
 *
 *    return inline_asm;
 * } */

struct block_member *ast_block(void)
{
    struct block_member *block = NULL;
    struct block_member *head = block;
    size_t stack_size = 0;
    if (!check_consume(L_CURLY)) {
        // TODO: error
    }

    if (check_consume(R_CURLY)) {
        return NULL;
    }

    while (!check_consume(R_CURLY)) {
        ast_statement_t *statement = ast_statement();
        if (statement->t == VAR_DEF) {
            stack_size += ast_type_size(statement->statement.var_def.ty);
        }
        if (block == NULL) {
            block = malloc(sizeof(struct block_member));
        }

        if (head == NULL) {
            head = block;
        }

        block->value = statement;

        block->next = malloc(sizeof(struct block_member));

        block = block->next;
        block->value = NULL;
    }

    head->stack_size = stack_size;
    return head;
}

ast_node_t *ast_call(void)
{
    ast_node_t *call = malloc(sizeof(ast_node_t));
    call->ty = CALL;
    if (!check_token(IDENTIFIER)) {
        // TODO: error
    }

    call->expr.call.identifier = list->lexeme;
    next();

    size_t arg_count = 0;

    if (!check_consume(L_PAREN)) {
        // TODO: error
    }

    token_t *tmp = list;
    while (!check_token(R_PAREN)) {
        if (check_consume(COMMA)) {
            arg_count += 1;
        } else {
            next();
        }
    }

    list = tmp;
    arg_count += 1;
    call->expr.call.arg_count = arg_count;
    call->expr.call.args = (ast_node_t **) malloc(sizeof(ast_node_t *) * arg_count);
    ast_node_t **args = call->expr.call.args;

    for (size_t i=0; !check_consume(R_PAREN); i++) {
        args[i] = expression();

        check_consume(COMMA);
    }

    if (!check_consume(SEMICOLON)) {
        // TODO: error
    }

    call->expr_ty = trie_get(&functions, call->expr.call.identifier);

    return call;
}

ast_statement_t *ast_variable(void)
{
    ast_statement_t *var = (ast_statement_t *) malloc(sizeof(ast_statement_t));

    var->t = VAR_DEF;
    var->statement.var_def.ty = ast_type();
    if (!check_token(IDENTIFIER)) {
        // TODO: error
    }

    var->statement.var_def.identifier = list->lexeme;
    next();

    if (check_consume(ASSIGN)) {
        ast_statement_t *assignment = (ast_statement_t *) malloc(sizeof(ast_statement_t));
        assignment->t = VAR_ASSIGN;
        assignment->statement.var_assign.identifier = var->statement.var_def.identifier;
        assignment->statement.var_assign.value = expression();
        var->statement.var_def.assignment = assignment;
    }

    if (!check_consume(SEMICOLON)) {
        // TODO: error
    }

    return var;
}


ast_statement_t *ast_return(void)
{
    ast_statement_t *value = (ast_statement_t *) malloc(sizeof(ast_statement_t));
    value->t = RETURN_STATEMENT;
    value->statement.ret.value = expression();
    if (!check_consume(SEMICOLON)) {
        // TODO: error
    }
    return value;
}

ast_statement_t *ast_if(void)
{
    ast_statement_t *if_stmt = malloc(sizeof(ast_statement_t));
    if_stmt->t = IF_STATEMENT;  // Usa el enum que agregaste

    if (!check_consume(L_PAREN)) {
        // TODO: error("if ( expected");
        free(if_stmt);
        return NULL;
    }
    if_stmt->statement.if_cond.condition = comparison();  // ¡Usa comparison() para y > 10!
    if (!check_consume(R_PAREN)) {
        // TODO: error(") expected");
        free(if_stmt);
        return NULL;
    }
    if_stmt->statement.if_cond.block = ast_block();  // Parsea el { ... }

    return if_stmt;
}

ast_statement_t *ast_while(void)
{
    ast_statement_t *while_stmt = malloc(sizeof(ast_statement_t));
    while_stmt->t = WHILE_STATEMENT;

    if (!check_consume(L_PAREN)) {
        // TODO: error("while ( expected");
        free(while_stmt);
        return NULL;
    }
    while_stmt->statement.while_cond.condition = comparison();  // Cond como i < a
    if (!check_consume(R_PAREN)) {
        // TODO: error(") expected");
        free(while_stmt);
        return NULL;
    }
    while_stmt->statement.while_cond.block = ast_block();  // { ... }

    return while_stmt;
}

ast_statement_t *ast_statement(void)
{
    token_t *tmp = list;
    if (check_consume(RETURN)) {
        return ast_return();
    } else if (check_consume(IF)) {
        return ast_if();
    } else if (check_consume(WHILE)) {
        return ast_while();
    } else {
        switch (list->ty) {
            case LONG:
            case SHORT:
            case UNSIGNED:
            case CHAR:
            case INT:
            case FLOAT:
            case DOUBLE:
            case VOID: {
                tmp = list;
                ast_type();
                check_consume(IDENTIFIER);
                if (check_token(ASSIGN)) {
                    list = tmp;
                    return ast_variable();
                } else if (check_token(L_PAREN)) {
                    list = tmp;
                    return ast_function();
                }
                break;
            }
            default:
                break;
        }

        // ¡NUEVO! Manejo de standalone assign: ID = expr;
        if (check_token(IDENTIFIER) && check_next(ASSIGN)) {
            ast_statement_t *assign = malloc(sizeof(ast_statement_t));
            assign->t = VAR_ASSIGN;
            assign->statement.var_assign.identifier = list->lexeme;
            next();  // Consume ID
            if (!check_consume(ASSIGN)) {
                // TODO: error("= expected");
            }
            assign->statement.var_assign.value = comparison();  // Expr como i + 1
            if (!check_consume(SEMICOLON)) {
                // TODO: error("; expected");
            }
            return assign;
        }

        // Statement como expresión (calls, etc.)
        ast_statement_t *expr = malloc(sizeof(ast_statement_t));
        expr->t = EXPRESSION;
        expr->statement.expression = comparison();  // Usa comparison() para >, <, etc.
        if (!check_consume(SEMICOLON)) {
            // TODO: error("; expected");
        }
        return expr;
    }
}

ast_node_t *expression(void)
{
    ast_node_t *t = term();

    while (check_token(PLUS) || check_token(MINUS)) {
        ast_node_t *b = malloc(sizeof(ast_node_t));
        b->ty = BINARY;
        b->expr.binary.op = check_token(PLUS) ? OP_PLUS : OP_MINUS;
        b->expr.binary.left = t;
        next();
        b->expr.binary.right = term();

        t = b;
    }

    return t;
}

ast_node_t *term(void)
{
    ast_node_t *t = factor();

    while (check_token(STAR) || check_token(SLASH)) {
        ast_node_t *b = malloc(sizeof(ast_node_t));
        b->ty = BINARY;
        b->expr.binary.op = check_token(STAR) ? OP_STAR : OP_SLASH;
        b->expr.binary.left = t;
        next();
        b->expr.binary.right = factor();

        t = b;
    }

    return t;
}

ast_node_t *factor(void)
{
    token_t *tmp = list;
    if (check_token(NUMBER)) {
        ast_node_t *node = malloc(sizeof(ast_node_t));
        node->expr.integer = atoi(list->lexeme);
        node->ty = INTEGER;
        next();

        return node;
    } else if (check_consume(IDENTIFIER)) {
        if (check_token(L_PAREN)) {
            list = tmp;
            return ast_call();
        }
        list = tmp;

        ast_node_t *node = malloc(sizeof(ast_node_t));
        node->expr.identifier = list->lexeme;
        node->ty = ID;
        next();

        return node;
    } else if (check_token(STRING)) {
        struct literal_pair *literal = malloc(sizeof(struct literal_pair));



        literal->literal = list->lexeme;
        literal->label = str_count;
        if (literals_head != NULL) {
            literal->next = literals_head;
        }
        literals_head = literal;

        ast_node_t *node = malloc(sizeof(ast_node_t));
        node->expr.string = str_count;
        node->ty = STRING_LIT;
        next();
        str_count++;

        return node;
    }

    return NULL;
}

ast_node_t *comparison(void)
{
    ast_node_t *t = expression();

    while (check_token(GREATER) || check_token(LESS) || check_token(EQUAL)) {
        ast_node_t *b = malloc(sizeof(ast_node_t));
        b->ty = BINARY;
        if (check_token(GREATER)) {
            b->expr.binary.op = OP_GREATER;
            next();
        } else if (check_token(LESS)) {
            b->expr.binary.op = OP_LESS;
            next();
        } else if (check_token(EQUAL)) {
            next();
            if (check_consume(EQUAL)) {
                b->expr.binary.op = OP_EQUAL;
            } else {
                free(b);
                return t;
            }
        }
        b->expr.binary.left = t;
        b->expr.binary.right = expression();
        t = b;
    }

    return t;
}

expr_type_t ast_type(void)
{
    expr_type_t res;

    if (check_token(STRUCT)) {
        char *s_name = next()->lexeme;
        char *s_ty = aligned_alloc(0x1000, strlen(s_name)+1);
        strcpy(s_ty, s_name);
        next();
        res = ((size_t)s_ty) | (STRUC & 0xfff);
    } else if (check_consume(LONG)) {
        check_consume(INT);
        res = INT64;
    } else if (check_consume(SHORT)) {
        check_consume(INT);
        res = INT16;
    } else if (check_consume(UNSIGNED)) {
        expr_type_t t = ast_type();
        switch (t) {
            case INT8:
                res = UINT8;
                break;
            case INT16:
                res = UINT16;
                break;
            case INT32:
                res = UINT32;
                break;
            case INT64:
                res = UINT64;
                break;
            default:
                // TODO: error
                break;
        }
    } else if (check_consume(CHAR)) {
        res = INT8;
    } else if (check_consume(INT)) {
        res = INT32;
    } else if (check_consume(VOID)) {
        res = VOID_T;
    }

    if (check_consume(STAR)) {
        res |= TYPE_POINTER;
    }

    return res;
}

size_t ast_type_size(expr_type_t ty)
{
    switch(ty) {
        case INT8:
            return 1;
        case INT16:
            return 2;
        case INT32:
            return 4;
        case INT64:
            return 8;
        case UINT8:
            return 1;
        case UINT16:
            return 2;
        case UINT32:
            return 4;
        case UINT64:
            return 8;
        case F32:
            return 4;
        case F64:
            return 8;
        case VOID_T:
            return 0;
        case STRUC:
            return 0;
        default:
            return -1;
    }
}

void ast_walk(ast_node_t *ast)
{
    if ((ast->ty & 0xfff) == INTEGER) {
        printf("n: %ld\n", ast->expr.integer);
    } else if ((ast->ty) == BINARY) {
        printf("(");
        ast_walk(ast->expr.binary.left);
        printf(" + ");
        ast_walk(ast->expr.binary.right);
        printf(")\n");
    }
}
