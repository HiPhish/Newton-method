#ifndef NEWTON_VM_CODE_H
#define NEWTON_VM_CODE_H

/** @file vm_code.h
 *
 *  This file describes the structure holding the compiled bytecode and the
 *  corresponding functions. The bytecode itself is just a sequence of bytes in
 *  an array, but we need to wrap it in a structure to know the length of the
 *  array.
 *  
 *  The way to think about this structure is like a cartridge that gets plugged
 *  into our virtual machine. The cartridge contains some information for the
 *  machine on how to read it and a tape of paper (think like a really long
 *  punch card) containing the actual code. An empty cartridge contains no
 *  tape, so the pointer to the sequence is `NULL`.
 *
 *  A cartridge can be copied as a *deep copy*: the new cartridge has its own
 *  code roll which is a copy of the original one. A shallow copy would share
 *  the code, which would only be useful for passing the same cartridge around.
 *
 *      +--[CARTRIDGE]--+
 *      |               |
 *      |  length       |
 *      |  code         |
 *      |   |           |
 *      +---+-----------+
 *          |            _ _ _ _ _     _
 *          +--[TAPE]-->|_|_|_|_|_|...|_|
 *
 *  The data that's embedded into the cartridge could contain other information
 *  as well, such as the version of the bytecode syntax for different versions,
 *  or a decompression table for compressed bytecode, but we won't be using
 *  any of that in our case.
 */

#include <stdlib.h> //size_t, uint8_t(?)

/** Structure describing the compiled bytecode of an arithmetic expression.
 *
 *  The VMCode object is the owner of the bytecode array. It must be cleared
 *  before it can be safely deleted.
 *
 *  @sa vm_code_clear
 */
typedef struct vm_code {
	size_t   length;   /**< Length of the bytecode sequence. */
	size_t   capacity; /**< Length of the bytecode array.    */
	uint8_t *code;     /**< Array of compiled bytecode.      */
} VMCode;

/** Deep-copy a VM code object.
 *
 *  @param original  The original VM code object to copy.
 *  @param error     Pointer to an integer to hold an error code.
 *
 *  @return  A new, deep-copied VMCode object.
 *
 *  This function returns a new VM code with its own code. Just copying the
 *  VMCode object the usual way would have the copy point to the same sequence
 *  of code as the original. This deep copy creates a new independent sequence.
 *
 *  If the code array for the new code object could not be allocated an object
 *  will be returned regardless. In that case the error parameter will hold an
 *  error code. If everything went right the error code will be 0.
 *
 *  It is possible to pass a NULL pointer for the error code as well, in that
 *  case it will be ignored.
 */
VMCode vm_code_copy(const VMCode original, int *const error);

/** Clears a VMCode object by freeing its bytecode array.
 *
 *  @param code  The VMCode object to clear.
 *
 *  Since each VMCode object owns its bytecode array we have to clear it first
 *  before we can throw it away. This function will free the memory, invalidate
 *  the pointer and reset the size to 0. Deleting a VMCode object without
 *  clearing it first is a memory leak.
 *
 *  Following the cartridge analogy, this is like erasing the cartridge.
 */
void vm_code_clear(VMCode *const code);

#endif /* NEWTON_VM_CODE_H */

