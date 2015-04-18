#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/compiler.h"
#include "virtual_machine/vm.h"
#include "method/method.h"

/** Exit codes of the program. */
enum exit_codes {
	EX_SUCCESS  , /**< Successful termination.     */
	EX_ARGS     , /**< Invalid function arguments. */
	EX_COMPILER , /**< Compiler failure.           */
};

/** Handles arguments passed to the program.
 *
 *  @param argc  Number of arguments passed.
 *  @param argv  Array or argument strings.
 *
 *  @return  0 if no errors were found, otherwise an error code.
 */
int handle_arguments(int argc, const char *argv[], const char **function, const char **guess, const char **print);

/** Prints usage instructions to the standard output. */
void print_usage(void);


int main(int argc, const char *argv[]) {
	int exit_status = EX_SUCCESS; /* Exist status of the program. */

	double x_0, x_n;     /* Initial and resulting value of Newton's method.  */
	int print_steps = 0; /* Print every step of the iteration.               */

	const char *function_string = NULL; /* Text representation of function.  */
	const char *guess_string    = NULL; /* Text representation of guess.     */
	const char *print_string    = NULL; /* Text representation print option. */

	SyntaxNode *function_tree   = NULL; /* Syntax tree of the function.      */
	SyntaxNode *derivative_tree = NULL; /* Syntax tree of the derivative.    */

	VMCode *function_vm_code    = NULL; /* VM code of the function.          */
	VMCode *derivative_vm_code  = NULL; /* VM code of the derivative.        */

	if (handle_arguments(argc, argv, &function_string, &guess_string, &print_string) != 0) {
		fprintf(stderr, "Error: invalid arguments.\n");
		print_usage();
		exit_status = EX_ARGS;
		goto end;
	}
	if (!function_string || !guess_string) {
		fprintf(stderr, "Error: invalid arguments.\n");
		print_usage();
		exit_status = EX_ARGS;
		goto end;
	}
	if (print_string) {print_steps = 1;}

	x_0 = strtod(guess_string, NULL);


	if (compiler_frontend(function_string, &function_tree)) {
		fprintf(stderr, "Compiler frontend error.\n");
		exit_status = EX_COMPILER;
		goto end;
	}

	syntax_node_condense(function_tree); // optimizes the syntax tree

	// Now derive the function tree and condense it again
	derivative_tree = syntax_node_derive(function_tree);
	syntax_node_condense(derivative_tree);

	if (compiler_backend(function_tree, &function_vm_code) != 0) {
		fprintf(stderr, "Compiler backend error.\n");
		exit_status = EX_COMPILER;
		goto end;
	}
	if (compiler_backend(derivative_tree, &derivative_vm_code) != 0) {
		fprintf(stderr, "Compiler backend error.\n");
		exit_status = 1;
		goto end;
	}

	// Perform Newton's method.
	x_n = method_iterate(*function_vm_code, *derivative_vm_code, x_0, &exit_status, print_steps);
	if (exit_status != EX_SUCCESS) {goto end;}


	printf("  The root of \'%s\' with starting value %f is: %f.\n",function_string, x_0, x_n);

end:
    return exit_status;
}


int handle_arguments(int argc, const char *argv[], const char **function, const char **guess, const char **print) {
	int error = 0; /* Will be non-0 if an error occurs. */

	if (argc == 3) { /* newton f g */
		*function = argv[1];
		*guess    = argv[2];
		goto end;
	}

	for (int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "--f", 3) == 0) {
			*function = argv[++i];
			continue;
		}
		if (strncmp(argv[i], "--g", 3) == 0) {
			*guess = argv[++i];
			continue;
		}
		if (strncmp(argv[i], "--p", 3) == 0) {
			*print = argv[i];
			continue;
		}
		/* If it does not match any known string. */
		error = 1;
		goto end;
	}

end:
	return error;
}

void print_usage(void) {
	printf(
		"Usage: newton --f function --g guess [--p]\n"
		"\n"
		"Or:    newtown 'f' 'g'\n"
		"       where 'f' is the function and 'g' the guessed root.\n"
		"\n"
		"Both options are equally valid, it's a matter of personal preference.\n"
		"The optional --p flat prints the iteration steps.\n"
	);
}

