#ifndef NEWTON_PARSER_H
#define NEWTON_PARSER_H

#include "../syntax_node/syntax_node.h"

/** Structure of a parser to build the function from tokens. */
struct parser {
	/** Stack of root nodes for output.
	 *
	 *  By the time the parser is done there should be only one node remaining on
	 *  the stack: the root node of the function. All other nodes are its children.
	 *  If there are more nodes on the stack we have a syntax error.
	 */
	struct syntax_node_stack *operand_stack;

	/** Stack of operator nodes waiting to be integrated into the function. */
	struct syntax_node_stack *operator_stack;

	/** Previously parsed node.
	 *
	 *  We need to know what node we previously parsed in order to be able to
	 *  substitute the binary minus for a unary negate.
	 */
	struct syntax_node *previous_node;
};


/** Initialize a parser for building the syntax tree.
 *
 *  Allocate the parser and initialize the output- and operator stack as empty
 *  stacks with default size.
 */
struct parser *parser_init(void);

/** Finishes building the syntax tree after the lexing machine is done.
 *
 *  After the last token has been read (i.e. the lexing machine has terminated)
 *  finish off building the syntax tree by processing the remaining operators
 *  on the operator stack. The function should be called by the lexer after it
 *  has terminated to finish the tree construction.
 *
 *  This does not free the parser, you will have to free the memory yourself.
 *
 *  @param p  Parser to finish.
 *
 *  @return  Pointer to the root of the syntax tree, which is `NULL` if a
 *           syntax error occurred.
 */
struct syntax_node *parser_finish(struct parser *p);

/** Parses a syntax node into the syntax tree.
 *
 *  This is the true heart of the parser. It implements the shunting-yard
 *  algorithm and is called when a node is passed to the parser.
 *
 *  @param p     Parser to parse the node.
 *  @param node  The syntax node that will be "digested" and parsed into the
 *               syntax tree.
 */
void parser_parse_node(struct parser *p, struct syntax_node *node);

#endif /* NEWTON_PARSER_H */

