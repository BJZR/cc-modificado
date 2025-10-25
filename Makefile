CC:=cc
CFLAGS:=-Wall -Wextra -std=c11 -pedantic -g -I./
LIBS:=
BIN=cc

SRC:=main.c lexer.c trie.c parser.c codegen.c
OBJ:=${SRC:.c=.o}

all: ${OBJ}
	${CC} ${LIBS} ${OBJ} -o ${BIN}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

.PHONY: clean
clean:
	@rm -rfv ${OBJ} ${BIN}
