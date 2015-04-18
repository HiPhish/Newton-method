# Turn on  Clang hard-mode (error on language extensions and treat warnings as
# errors). Always use debug symbols.
PROG    = newton
CFLAGS  = -std=c99 -g -pedantic -Werror -Wall
LDLIBS  =
OBJECTS = syntax_node.o \
          vm_code.o     \
          frontend.o    \
          backend.o     \
          vm.o          \
          method.o      \
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

backend.o: compiler/backend/backend.h compiler/backend/backend.c virtual_machine/opcodes.h syntax_node/syntax_node.h
	@$(CC) ${CFLAGS} -c compiler/$*/$*.c

frontend.o: compiler/frontend/frontend.h compiler/frontend/frontend.c virtual_machine/opcodes.h syntax_node/syntax_node.h
	@$(CC) ${CFLAGS} -c compiler/$*/$*.c

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

