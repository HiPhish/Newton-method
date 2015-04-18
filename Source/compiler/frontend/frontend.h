#ifndef NEWTON_COMPILER_FRONTEND_H
#define NEWTON_COMPILER_FRONTEND_H

#include "../../syntax_node/syntax_node.h"

/** Run the compiler frontend.
 *
 *  @param source  Source code string.
 *  @param tree    Pointer to the root of the resulting tree.
 *
 *  @return  Exit status, 0 if no error, non-0 otherwise.
 *
 *  This function runs the compiler frontend to compile the source code string
 *  into a syntax tree. The caller is responsible for ownership of the
 *  resulting syntax tree. If compilation fails the code object will remain
 *  `NULL`.
 */
int compiler_frontend(const char *const source, SyntaxNode **tree);

#endif /* NEWTON_COMPILER_FRONTEND_H */

