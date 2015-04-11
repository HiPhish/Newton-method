#ifndef NEWTON_LEXER_H
#define NEWTON_LEXER_H

#include "../syntax_node/syntax_node.h"

/** Structure of a function string lexer machine. */
struct lexer;

typedef struct lexer Lexer;

/** Initializes the parsing machine over a given string.
 *
 *  Call this function to start lexing a string; a string is immutable and
 *  NUL-terminated. The machine will run ons its own and if it encounters an
 *  error it will exit the application. For real-world use it would be better
 *  to have the machine return some sort of failure message, allowing the
 *  program to decide what to do next instead of just exiting.
 *
 *  If the lexer cannot be allocated the function will return `Null`, so check
 *  the return value.
 *
 *  @param string  Read-only sequence of characters we want to parse.
 */
Lexer *init_lexer(const char *string);

/** Continually runs the lexing machine.
 *
 *  This function keeps running the machine by parsing characters as longs as
 *  the machine has not reached the accept or error state. Once one of these
 *  states has been reached it will destroy the machine by calling
 *  `destroy_lexer()`. Since the input string is finite and the machine is
 *  strictly right-moving it is guaranteed that it will eventually terminate.
 *
 *  @param l  The lexer to run.
 */
void run_lexer(Lexer *l);

/** Destroys the lexing machine.
 *  
 *  Once the lexing machine has terminated we can safely destroy it. This
 *  function resets all variables and sets pointers to NULL. Furthermore, it
 *  checks if the string it has parsed is even valid by comparing the state to
 *  `ERROR_ST`. If everything went right it tells the syntax tree builder to
 *  finish building the syntax tree.
 *
 *  @param l  The lexer to destroy.
 *
 *  @note  This function does not invalidate the pointer, you have to set it to
 *         `NULL` yourself.
 *
 *  @sa syntax_tree_builder_finish()
 */
struct syntax_node *destroy_lexer(Lexer *l);

#endif /* NEWTON_LEXER_H */

