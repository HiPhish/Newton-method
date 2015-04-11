#ifndef NEWTON_METHOD_H
#define NEWTON_METHOD_H

#include "../syntax_node/syntax_node.h"
#include "../virtual_machine/vm_code.h"

/** Performs the iterative steps of Newton's method.
 *
 *  This is the heart of the algorithm. The function performs the iterative
 *  steps and keeps track of the number of iterations. It will terminate when
 *  the approximation is close enough or if the amount of iterations has
 *  reached the limit, depending on which of the two occurs first.
 *
 *  @param function    VM code of the function.
 *  @param derivative  VM code of the derivative.
 *  @param guess       Starting value of the method.
 *  @param error       Pointer to store an error code in.
 *  @param print       Whether to print the individual steps.
 *
 *  @return  Approximation of the functions root according to Newton's method.
 */
double method_iterate(VMCode function, VMCode derivative, double guess, int *error, int print);

#endif /* NEWTON_METHOD_H */

