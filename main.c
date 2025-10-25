#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lexer.h>
#include <codegen.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: cc [filename]\n");
        exit(1);
    }

    FILE *source = fopen(argv[1], "r");
    size_t size = 0;
    fseek(source, 0, SEEK_END);
    size = ftell(source);
    fseek(source, 0, SEEK_SET);

    char *buffer = (char *) malloc(size+1);
    fread(buffer, size, 1, source);
    buffer[size] = '\0';

    token_t *l = lexer_parse(buffer);

    struct statement_list *ast = ast_parse();
    FILE *f = fopen("out.S", "w");
    gen_asm(f, ast);

    fclose(f);

    return 0;
}
