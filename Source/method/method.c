#include <stdio.h>
#include <math.h>

#include "method.h"
#include "../virtual_machine/vm.h"
#include "../virtual_machine/vm_code.h"
#include "../syntax_node/syntax_node.h"

/** Maximum amount of iterations before giving up.
 *
 *  We need a set limit because otherwise the algorithm might get caught up in
 *  an infinite loop because Newton's method is not guaranteed to terminate.
 */
#define MAX_ITERATIONS  100

/** Minimum precision needed for acceptable result
 *
 *  This is the maximal deviation a result may have from 0. Since Newton's
 *  method only approximates the root we can't be certain that our result will
 *  ever be perfect; this limit allows us to declare when the result is good
 *  enough.
 */
#define EPSILON  0.0000001

double method_iterate(VMCode function, VMCode derivative, double guess, int *error, int print) {
	VirtualMachine machine = {.code = {.length = 0, .capacity = 0, .code = NULL}, .reg_x = guess};

	double f_xn, d_xn, x_n_1; /* f(x_n) and f'(x_n), x_{n-1}  */
	int    iterations  = 0;   /* Number of iterations passed. */
	int    exit_status = 0;   /* No error.                    */

	do {
		machine_load_code(&machine, function);
		if((exit_status = machine_execute(&machine, &f_xn)) != 0) {goto end;}
		if (print) {x_n_1 = machine.reg_x;}

		if (fabs(f_xn) < EPSILON) {goto end;}

		machine_load_code(&machine, derivative);
		if((exit_status = machine_execute(&machine, &d_xn)) != 0) {goto end;}

		machine.reg_x = machine.reg_x - f_xn / d_xn;
		if (print) {
			printf("%3i: % .3f = % .3f - % .3f / % .3f; ", iterations, machine.reg_x, x_n_1, f_xn, d_xn);
			printf("\n");
		}

		++iterations;
	} while (iterations < MAX_ITERATIONS);
	fprintf(stderr, "Error: could not find a suitable result, aborting. \n"
	        "  After %i iterations the best result is %.4f with"
	        "  a function value of %.4f.\n", iterations, machine.reg_x, f_xn
	);
	exit_status = -1;

end:
	if (error) {*error = exit_status;}
	return machine.reg_x;
}

