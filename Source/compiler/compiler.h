#ifndef NEWTON_COMILER_H
#define NEWTON_COMILER_H

#include <stdint.h>
#include "../virtual_machine/vm_code.h"
#include "../syntax_node/syntax_node.h"

/** Compiles a syntax tree into a new VMCode object.
 *
 *  @param tree  The syntax tree to compile.
 *
 *  @return A new VMCode object that owns the compiled bytecode.
 *
 *  Keep in mind that compilation can fail, in which case the VMCode object
 *  will be empty, meaning the length of the bytecode sequence will be zero and
 *  the pointer to the sequence will be *NULL*.
 */
VMCode compile_syntax_tree(struct syntax_node *tree);

#endif /* NEWTON_COMILER_H */

