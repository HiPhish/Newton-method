#ifndef NEWTON_SYNTAX_NODE_H
#define NEWTON_SYNTAX_NODE_H

/** @file syntax_node.h
 *
 *  Public header file for syntax nodes.
 *
 *  This file contains all the publicly available declarations for syntax nodes,
 *  i.e. the nodes themselves, functions to manipulate them and an operator
 *  enum.
 */

#include <stdio.h>
#include "../newton.h"

/** Enumeration for all the possible types of operators.
 *
 *  The last item, *NUMBER_OF_OPERATORS*, gives us the amount of operators in
 *  the enum, which can be used for the size of an array.  It is used in the
 *  arrays *operator_arity* and *operation_list*. If entries are added those
 *  arrays must be adjusted as well.
 *
 *  The unknown operator is guaranteed to have the number 0. This can be used
 *  when error-checking.
 */
typedef enum operator_type{
	OP_UNKNOWN          , /**< Unknown/undefined operator.    */
	OP_NUMBER           , /**< Numbers literal.               */
	OP_NEGATE           , /**< Unary minus sign, subtraction. */
	OP_PLUS             , /**< Addition operator, not a sign. */
	OP_MINUS            , /**< Binary subtraction operator.   */
	OP_TIMES            , /**< Multiplication operator.       */
	OP_DIVIDE           , /**< Division operator.             */
	OP_POWER            , /**< Power-of operator.             */
	OP_EXP              , /**< Exponential function.          */
	OP_LN               , /**< Natural logarithm.             */
	OP_SIN              , /**< Sine function.                 */
	OP_COS              , /**< Cosine function.               */
	OP_TAN              , /**< Tangent function.              */
	OP_X_VAR            , /**< X variable.                    */
	OP_PI               , /**< Constant π (pi).               */
	OP_E                , /**< Constant e (Euler's number).   */
	OP_LEFT_BRACE       , /**< Left parenthesis.              */
	OP_RIGHT_BRACE      , /**< Right parenthesis.             */
	NUMBER_OF_OPERATORS , /**< Total number or operators.     */
} Operator;

/** Precedence of the node's operator.
 *
 *  Higher number means higher precedence. Functions are the highest, followed
 *  by negation, then multiplication and division and finally addition and
 *  subtraction. For anything else the precedence is 0 because it should not
 *  appear in a precedence comparison, i.e.  numbers, constants, variables and
 *  undefined operators. If they do it is an error.
 */
unsigned int operator_precedence[NUMBER_OF_OPERATORS];

/** Maps an operator type to a string. */
char *operator_to_string[NUMBER_OF_OPERATORS];

/** Struct representing a syntax tree node.
 *
 *  This is the most important data type in the entire program. Each node
 *  represents either an operator (symbol, function) or an operand (number,
 *  constant, variable), but for the sake of simplicity both are sometimes
 *  called operators in general. Each node can have a certain number of child
 *  nodes, defined by the nodes @a arity.
 */
typedef struct syntax_node {
	/** The type of operator the node represents. */
	Operator operator_value;

	/** Numeric value for number nodes.
	 *
	 *  If a node is not a number node this value will be ignored and is best
	 *  set to 0. It might be possible to use the numeric value to store
	 *  results of child operations for operator nodes, but such a feature is
	 *  not planned and I am not certain if there would be any benefit to it.
	 */
	double numeric_value;

	/** The arity of the operator node.
	 *
	 *  This is always 0 for numbers, variables and constants, 1 for functions
	 *  and negation (negative sign) and 2 for every other operator. The
	 *  program could be extended to support functions with multiple arguments,
	 *  such as a logarithm to any base.
	 */
	unsigned int arity;

	/** Array of child nodes.
	 *
	 *  The size of this array is given by the node's @a arity; when allocating
	 *  memory for a new node always use the arity for the array's size.
	 */
	struct syntax_node *operand[MAX_ARITY];
} SyntaxNode;

/** Creates a syntax node by allocation the memory, filling in the values and
 *  returning a pointer to it.
 *
 *  @param op      The operator token, `NUMBER_OP` if it's a number.
 *  @param number  The number value for the node if it's a number node. Set to
 *                 *0* for operator nodes.
 *
 *  @return  Pointer to the created syntax node. The result is `NULL` on
 *  failure.
 */
SyntaxNode* syntax_node_construct(Operator op, double number);

/** Copies an existing node to create an exact duplicate and returns a pointer
 *  to the duplicate. This function does not alter the original.
 *
 *  @param original  The original node to copy (read-only).
 *
 *  @return  Pointer to a new node, which is a duplicate of the supplied node.
 */
SyntaxNode *syntax_node_copy(const SyntaxNode *const original);

/** Destroys an existing syntax node recursively, freeing its memory.
 *
 *  Before the node gets freed its child nodes are freed recursively first, or
 *  else they would be out of reach for the application and would cause a
 *  memory leak. Note that this function will not set your existing pointers to
 *  `NULL`, you will have to do that yourself.
 *
 *  @param node  The node we want to destroy.
 */
void syntax_node_destroy(SyntaxNode *node);

/** Performs the operation of the node and returns the result.
 *
 *  This function will recursively operate on the node and all its children
 *  until it returns the resulting number or encounter an error.
 *
 *  @param node  Node to perform the operation of.
 *
 *  @return  Number resulting from operating on the node. In case of number
 *           nodes the return value is the number itself. In case of variable
 *           nodes it is the variable value.
 */
double syntax_node_operate(SyntaxNode *node, double value);

/** Condenses a (sub-)tree of syntax nodes into one number node, as long as the
 *  result can be computed into a constant.
 *
 *  If successful the original node will be turned into a number node with the
 *  result of its operation as its number value. The operand nodes will be
 *  destroyed in the process, irreversibly losing information. If you have a
 *  term like @f$ \sin(\pi / 6) @f$ it will become @f$ 0.5 @f$ with no way to
 *  distinguish it from a @f$ 0.5 @f$ that came from @f$ 2/4 @f$. The point of
 *  condensing nodes is to reduce the size and computational overhead by
 *  keeping the tree as small as possible. This function works recursively, so
 *  applying it once to the root of a (sub-)tree will condense the entire tree
 *  as much as possible.
 *
 *  @param node  The node we want to condense.
 *
 *  @return  1 if the condensing was successful, 0 if not.
 */
int syntax_node_condense(SyntaxNode *node);

/** Whether a syntax node, or rather its sub-tree, is a constant number or not.
 *
 *  Any node that does not contain any variable nodes in its sub-tree is a
 *  constant and could be expressed as a constant number.
 *
 *  @param node  The root of the sub-tree we want to check.
 *
 *  @return  Returns 1 if the node's sub-tree is constant and 0 otherwise.
 */
int syntax_node_is_constant(SyntaxNode *node);

/** Takes in the root of a syntax tree and returns its derivative recursively.
 *
 *  This function works recursively: first it uses the node's operator to
 *  determine how exactly to derive the node, then it builds a new node and
 *  adds child nodes to it. These child nodes in turn can return be the result
 *  of some other derivation, thus continuing the derivation process
 *  recursively.  Therefore it is enough to call this function once on the root
 *  node of the tree to perform a complete derivation of the tree.
 *
 *  @param node  The root note of the tree or sub-tree to derive.
 *
 *  @return  The root node of the derived tree or sub-tree.
 */
SyntaxNode *syntax_node_derive(SyntaxNode *node);

/** Returns an operator based on the passed char.
 *
 *  @param c  pointer to a character to turn into operator.
 *
 *  @return  Operator value of the char, e.g. PLUS_OP for '+'. Returns NO_OP for
 *           unknown characters like '€'.
 */
Operator char_to_operator(char *c);

/** Returns an operator based on the passed string.
 *
 *  @param s  String to turn into operator.
 *
 *  @return  Operator value of the passed string, e.g. EXP_OP for "exp". Returns
 *           NO_OP for unknown strings like "abc".
 */
Operator string_to_operator(char *s);

#endif /* NEWTON_SYNTAX_NODE_H */

