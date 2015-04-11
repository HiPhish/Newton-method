#ifndef NEWTON_VM_H
#define NEWTON_VM_H

#include <stdint.h>
#include "../newton.h"
#include "vm_code.h"

/** @file vm.h
 *
 *  Public interface for the virtual machine.
 *
 *  How the virtual machine works
 *  -----------------------------
 *  The VM reads the code backwards, and every time it reads an opcode it
 *  carries out that action. If the action is a number literal instruction the
 *  next sequence of bytes is the number literal. Numbers, be they literals or
 *  results of a previous computation, are stored in registers.
 *
 *  The machine has only one register holding the variable X. There are no
 *  registers for processing the arithmetic expression, a stack is used
 *  instead.  We could use temporary registers for the operands of an operator
 *  like this:
 *
 *      POP  tmp_1       ; pop number from stack into register tmp_1
 *      POP  tmp_2       ; pop number from stack into register tmp_2
 *      ADD  tmp_1 tmp_2 ; add tmp_2 to tmp_1
 *      PUSH tmp_1       ; push tmp_2 back to the stack
 *
 *  However, it is easier to just do it in one go C-style instead of emulating
 *  assembly.
 *
 *      PUSH(POP + POP)
 *      // expands to
 *      push(pop(vm) + pop(vm), vm)
 *      // where `vm` is a pointer to the virtual machine
 *      
 *  The stack does not have to grow, it has a fixed size that matches the
 *  maximum arity of any operator (2 in this case, but it could be larger).
 */

/** Structure of a virtual machine to run the bytecode of an arithmetic expression. */
typedef struct virtual_machine {
	VMCode code;  /**< Compiled bytecode of an arithmetic expression.       */
	double reg_x; /**< Value of the variable for the arithmetic expression. */
} VirtualMachine; 


/** Loads new code into the machine.
 *
 *  Replaces the machine's old code with the new one. The old code is returned.
 *  After the code has been loaded it is owned by the machine.
 *  
 *  @param machine  The machine to load the new code.
 *  @param code     Structure of code to load.
 *
 *  @return  Array of old machine code.
 */
VMCode machine_load_code(VirtualMachine *machine, VMCode code);

/** Execute the bytecode that has been loaded into the machine.
 *
 *  The value of *result* will be overwritten.
 *
 *  @param machine  The machine to execute the code.
 *  @param result   Pointer to a variable to store the result in, must not be
 *                  NULL.
 *
 *  @return  0 if no error occurred, non-0 otherwise.
 *
 *  The machine will be run using the current value of the `reg_x` register and
 *  if it successfully terminates the register will be set to the result of the
 *  expression. If the machine aborts prematurely the register will remain
 *  unchanged. The machine can abort if the internal stack is unable to grow to
 *  accommodate for the numbers to store.
 *
 *  To use a different starting value set the register manually before
 *  executing code.
 */
int machine_execute(VirtualMachine *machine, double *result);

#endif /* NEWTON_VM_H */

