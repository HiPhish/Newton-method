# Turn on  Clang hard-mode (error on language extensions and treat warnings as
# errors). Always use debug symbols.
PROG    = newton
CFLAGS  = -std=c99 -g -pedantic -Werror -Wall
LDLIBS  =
OBJECTS = syntax_node.o       \
          lexer.o             \
          parser.o            \
          vm_code.o           \
          compiler.o          \
          vm.o                \
          method.o            \
          newton.o
$(CC)   = clang

#===[ TARGETS ]=================================================================

# Unoptimised build for debugging.
debug: CFLAGS += -O0
debug: $(PROG)

# Optimised build for release.
release: CFLAGS += -O2
release: $(PROG)

# Run static analyser.
analyze: CFLAGS += --analyze
analyze: $(OBJECTS)

# Alternate spelling.
analyse: analyze

$(PROG): $(OBJECTS)
	@$(CC) ${CFLAGS} -o $(PROG) $(OBJECTS)

syntax_node.o: syntax_node/syntax_node.c syntax_node/syntax_node.h
	@$(CC) ${CFLAGS} -c $*/$*.c

lexer.o: lexer/lexer.c lexer/lexer.h
	@$(CC) ${CFLAGS} -c $*/$*.c

parser.o: parser/parser.c parser/parser.h
	@$(CC) ${CFLAGS} -c $*/$*.c

method.o: method/method.c method/method.h
	@$(CC) ${CFLAGS} -c $*/$*.c

compiler.o: compiler/compiler.c compiler/compiler.h virtual_machine/opcodes.h virtual_machine/vm_code.h syntax_node/syntax_node.h
	@$(CC) ${CFLAGS} -c $*/$*.c

vm.o: virtual_machine/vm.h virtual_machine/vm.c virtual_machine/vm_code.h virtual_machine/opcodes.h
	@$(CC) ${CFLAGS} -c virtual_machine/$*.c

vm_code.o: virtual_machine/vm_code.h virtual_machine/vm_code.c
	@$(CC) ${CFLAGS} -c virtual_machine/$*.c

newton.o: newton.c
	@$(CC) ${CFLAGS} -c $*.c

#---[ Phony targets ]-----------------------------------------------------------
clean:
	@rm -rf *.o *.plist $(PROG)

run: $(PROG)
	./$(PROG)

ctags:
	@ctags -R

