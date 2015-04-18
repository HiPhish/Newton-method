#ifndef NEWTON_COMPILER_BACKEND_H
#define NEWTON_COMPILER_BACKEND_H

/** @file backend.h
 *
 *  Interface to the compiler backend, not visible outside the compiler module.
 */

#include "../../syntax_node/syntax_node.h"
#include "../../virtual_machine/vm_code.h"

/** Run the compiler backend.
 *
 *  @param tree  Pointer to the root of the syntax tree.
 *  @param code  Pointer to the resulting VMCode, must be NULL.
 *
 *  @return  Exit status, 0 if no error, non-0 otherwise.
 *
 *  This function runs the compiler backend to generate the virtual machine
 *  bytecode from a syntax tree. The caller of the function is responsible for
 *  ownership of the code object. If compilation fails the code object will
 *  remain `NULL`.
 */
int compiler_backend(const SyntaxNode *const tree, VMCode **code);

#endif /* NEWTON_COMPILER_BACKEND_H */

