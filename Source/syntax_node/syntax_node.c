#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "syntax_node.h"

/** Syntax node operation for number nodes.
 *
 *  Gets the number value from a syntax node. If the node is not a number the
 *  program will exit with a syntax error.
 *
 *  @param node The node we want the number value from.
 *
 *  @return Number value of a number node, exits with a syntax error if the
 *  node is not a number.
 */
static double syntax_node_operation_number_value(SyntaxNode *node, double value);

/** Syntax node operation for negation.
 *
 *  @param node The negation node we wont to operate on.
 *
 *  @return Result of the child's operation with inverted sign.
 */
static double syntax_node_operation_negate(SyntaxNode *node, double value);

/** Syntax node operation for addition.
 *
 *  @param node The addition node we want to operate on.
 *
 *  @return Sum of the result of the children's operations.
 */
static double syntax_node_operation_add(SyntaxNode *node, double value);

/** Syntax node operation for subtraction.
 *
 *  @param node The subtraction node we want to operate on.
 *
 *  @return Difference of the result of the children's operations.
 */
static double syntax_node_operation_subtract(SyntaxNode *node, double value);

/** Syntax node operation for multiplication.
 *
 *  @param node The multiplication node we want to operate on.
 *
 *  @return Product of the result of the children's operations.
 */
static double syntax_node_operation_multiply(SyntaxNode *node, double value);

/** Syntax node operation for division.
 *
 *  @param node The division node we want to operate on.
 *
 *  @return Quotient of the result of the children's operations.
 */
static double syntax_node_operation_divide(SyntaxNode *node, double value);

/** Syntax node operation for exponentiation.
 *
 *  Exponentiation is somewhat complicated: everything works fine as long as
 *  the base is non-negative. However, if the base is negative only an integer
 *  exponent makes sense. Similarly, if the base is 0 only non-negative
 *  exponents are valid. There is no error-checking, the responsibility is up
 *  to the user to provide sensible functions.
 *
 *  @param node The exponentiation node we want to operate on.
 *
 *  @return Exponent of the left child's operation to the power of the right
 *  child's operation.
 */
static double syntax_node_operation_power(SyntaxNode *node, double value);

/** Syntax node operation for the exponential function.
 *
 *  @param node The exponential function node we want to operate on.
 *
 *  @return Euler's number raised to the power of the result of the children's
 *  operations.
 */
static double syntax_node_operation_exponent(SyntaxNode *node, double value);

/** Syntax node operation for the natural logarithm.
 *
 *  @param node The natural logarithm node we want to operate on.
 *
 *  @return Natural logarithm of the result of the children's operations.
 */
static double syntax_node_operation_logarithmus_naturalis(SyntaxNode *node, double value);

/** Syntax node operation for the sine.
 *
 *  @param node The sine node we want to operate on.
 *
 *  @return Sine of the result of the children's operations.
 */
static double syntax_node_operation_sine(SyntaxNode *node, double value);

/** Syntax node operation for the cosine.
 *
 *  @param node The cosine node we want to operate on.
 *
 *  @return Cosine of the result of the children's operations.
 */
static double syntax_node_operation_cosine(SyntaxNode *node, double value);

/** Syntax node operation for the tangent.
 *
 *  @param node The tangent node we want to operate on.
 *
 *  @return Tangent of the result of the children's operations.
 */
static double syntax_node_operation_tangent(SyntaxNode *node, double value);

/** Syntax node operation for the x variable.
 *
 *  @param node The x variable node we want to operate on.
 *
 *  @return Value of the global x variable.
 */
static double syntax_node_operation_x(SyntaxNode *node, double value);

/** Syntax node operation for the π (pi) constant.
 *
 *  @param node The π (pi) constant node we want to operate on.
 *
 *  @return Value of the π (pi) constant.
 */
static double syntax_node_operation_pi(SyntaxNode *node, double value);

/** Syntax node operation for the e (Euler's number) constant.
 *
 *  @param node The e (Euler's number) constant node we want to operate on.
 *
 *  @return Value of the e (Euler's number) constant.
 */
static double syntax_node_operation_e(SyntaxNode *node, double value);

/** This should never happen and if it does it's a program error. */
static double syntax_node_operation_failure(SyntaxNode *node, double value);

/** Derives number or constant nodes.
 *
 *  The derivative of a number node or a constant node is always a number node
 *  with numeric value 0.
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_number(const SyntaxNode * const node);

/** Derives negation nodes.
 *
 *  The derivative of a negation node is the negation of the derivative of its
 *  child. We could also multiply the derivative with @f$ -1 @f$, but negation
 *  requires one node less.
 *
 *  @f[ (-f)' = - f' @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_negate(const SyntaxNode * const node);

/** Derives addition nodes.
 *
 *  The derivative of an addition node is an addition node adding the
 *  derivatives of its children.
 *
 *  @f[ (f + g)' = f' + g' @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_plus(const SyntaxNode * const node);

/** Derives subtraction nodes.
 *
 *  The derivative of a subtraction node is a subtraction node subtracting the
 *  derivatives of its children.
 *
 *  @f[ (f - g)' = f' - g' @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_minus(const SyntaxNode * const node);

/** Derives multiplication nodes.
 *
 *  The derivative of a multiplication node is a addition node adding the
 *  product of *node*'s left child's derivative and *node*'s right child to the
 *  product of *node*'s left child and *node*'s right child's derivative.
 *
 *
 *  @f[ (f g)' = f' g + f g' @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_times(const SyntaxNode * const node);

/** Derives division nodes.
 *
 *  The derivative of a division node is a subtraction node subtracting the
 *  product of *node*'s left child's derivative and *node*'s right child from
 *  the product of *node*'s left child and *node*'s right child's derivative
 *  and dividing the result by the square of *node*'s right child.
 *
 *
 *  @f[ \left( \frac{f}{g} \right)' = \frac{ (f' g - f g') }{ g^2 } @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_divide(const SyntaxNode * const node);

/** Derives power-of nodes.
 *
 *  The derivative of a power node is more complicate we have a generic case
 *  where the derivative of a power node is @f[ (f^g)' = \exp'(f \ln(g) ) =
 *  \left( f' \ln(g) + f \frac{g'}{g} \right) \cdot f^g @f] and a special case
 *  if the right child is an integer
 *
 *
 *  @f[ (f^n)' = n \cdot f^{n-1} \cdot f' @f]
 *
 *  in which case negative values for the child are allowed.
 *
 *  @param node  The node we derive from
 *
 *  @return  Pointer to node derived from the @a node
 */
static SyntaxNode *derive_power(const SyntaxNode * const node);

/** Derives exponential function nodes.
 *
 *  The derivative of a exponential function node is a multiplication node
 *  multiplying the derivative of *node*'s child's with a copy of *node* and
 *  all its children.
 *
 *
 *  @f[ \exp'(f) = f' \cdot \exp(f) @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_exp(const SyntaxNode * const node);

/** Derives natural logarithm nodes.
 *
 *  The derivative of a natural logarithm node is the inverse of *node*
 *  multiplied with the derivative of *node*'s child.
 *
 *  @f[ \ln'(f) = \frac{f'}{f} @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_ln(const SyntaxNode * const node);

/** Derives sine nodes.
 *
 *  The derivative of a sine node is a cosine node with *node*'s child as its
 *  child, multiplied with the derivative of *node*'s child.
 *
 *
 *  @f[ \sin'(f) = f' \cdot \cos(f) @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_sine(const SyntaxNode * const node);

/** Derives cosine nodes.
 *
 *  The derivative of a cosine node is a negative sine node with
 *  *node*'s child as its child, multiplied with the derivative of
 *  *node*'s child.
 *
 *  @f[ \cos'(f) = -f' \cdot \sin(f) @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_cosine(const SyntaxNode * const node);

/** Derives tangent nodes.
 *
 *  The derivative of a tangent node is a multiplication node multiplying the
 *  quotient of the sine and cosine of *node*'s child with the derivative of
 *  *node*'s child.
 *
 *  @f[ \tan'(f) = f' \cdot \frac{ \sin(f) }{ \cos(f) } @f]
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_tangent(const SyntaxNode * const node);

/** Derives variable nodes.
 *
 *  The derivative of a variable node is always a number node with value 1.
 *
 *  @param node  The node we derive from.
 *
 *  @return  Pointer to node derived from the @a node.
 */
static SyntaxNode *derive_x(const SyntaxNode * const node);

/** This should never happen and if it does it's a program error. */
static SyntaxNode *derive_failure(const SyntaxNode * const node);


/** Array holding the arity for each operator.
 *
 *  *NUMBER_OP*, constants and variables have arity 0, *NEGATE_OP* and
 *  functions have arity 1 and the rest have arity 2. To get the arity of an
 *  operator simply pass the operator as the index, like
 *  `operator_arity[TIMES_OP]`.
 */
static int operator_arity[NUMBER_OF_OPERATORS] = {
	[ OP_UNKNOWN     ] = 0,
	[ OP_NUMBER      ] = 0,
	[ OP_NEGATE      ] = 1,
	[ OP_PLUS        ] = 2,
	[ OP_MINUS       ] = 2,
	[ OP_TIMES       ] = 2,
	[ OP_DIVIDE      ] = 2,
	[ OP_POWER       ] = 2,
	[ OP_EXP         ] = 1,
	[ OP_LN          ] = 1,
	[ OP_SIN         ] = 1,
	[ OP_COS         ] = 1,
	[ OP_TAN         ] = 1,
	[ OP_X_VAR       ] = 0,
	[ OP_PI          ] = 0,
	[ OP_E           ] = 0,
	[ OP_LEFT_BRACE  ] = 0,
	[ OP_RIGHT_BRACE ] = 0,
};

/** Array holding function pointers to the operations for each operator.
 *
 *  For example, typing `operation_list[my_node->operator_value](my_node)` will
 *  call the  appropriate function to operate on the node's children. The type
 *  of this is *array of pointers to functions that take in a pointer to a
 *  syntax node and return a double*. The amount of elements is two less than
 *  the number of operators, because we don't need any operation for
 *  parentheses.
 */
static double (*operation_list[NUMBER_OF_OPERATORS])(SyntaxNode *node, double value) = {
	[ OP_UNKNOWN     ] = syntax_node_operation_failure               ,
	[ OP_NUMBER      ] = syntax_node_operation_number_value          ,
	[ OP_NEGATE      ] = syntax_node_operation_negate                ,
	[ OP_PLUS        ] = syntax_node_operation_add                   ,
	[ OP_MINUS       ] = syntax_node_operation_subtract              ,
	[ OP_TIMES       ] = syntax_node_operation_multiply              ,
	[ OP_DIVIDE      ] = syntax_node_operation_divide                ,
	[ OP_POWER       ] = syntax_node_operation_power                 ,
	[ OP_EXP         ] = syntax_node_operation_exponent              ,
	[ OP_LN          ] = syntax_node_operation_logarithmus_naturalis ,
	[ OP_SIN         ] = syntax_node_operation_sine                  ,
	[ OP_COS         ] = syntax_node_operation_cosine                ,
	[ OP_TAN         ] = syntax_node_operation_tangent               ,
	[ OP_X_VAR       ] = syntax_node_operation_x                     ,
	[ OP_PI          ] = syntax_node_operation_pi                    ,
	[ OP_E           ] = syntax_node_operation_e                     ,
	[ OP_LEFT_BRACE  ] = syntax_node_operation_failure               ,
	[ OP_RIGHT_BRACE ] = syntax_node_operation_failure               ,
};

unsigned int operator_precedence[NUMBER_OF_OPERATORS] = {
	[ OP_NUMBER      ] = 0, /* error if this occurs */
	[ OP_NEGATE      ] = 3,
	[ OP_PLUS        ] = 1,
	[ OP_MINUS       ] = 1,
	[ OP_TIMES       ] = 2,
	[ OP_DIVIDE      ] = 2,
	[ OP_POWER       ] = 4,
	[ OP_EXP         ] = 4,
	[ OP_LN          ] = 4,
	[ OP_SIN         ] = 4,
	[ OP_COS         ] = 4,
	[ OP_TAN         ] = 4,
	[ OP_X_VAR       ] = 0, /* error if this occurs */
	[ OP_PI          ] = 0, /* error if this occurs */
	[ OP_E           ] = 0, /* error if this occurs */
	[ OP_LEFT_BRACE  ] = 0, /* error if this occurs */
	[ OP_RIGHT_BRACE ] = 0, /* error if this occurs */
	[ OP_UNKNOWN     ] = 0, /* error if this occurs */
};

/** Array of function pointers to derivation functions.
 *
 *  This array maps an operator to a corresponding derivation method. The
 *  method takes a syntax node as its argument and returns a pointer to the
 *  derived syntax node. The number of elements is two less than the number of
 *  operators, because we don't need to derive parentheses. The type of the
 *  array is *array of pointers to functions that take a pointer to a syntax
 *  node as the argument and return a pointer to another syntax node*.
 */
static SyntaxNode * (*derivation_table[NUMBER_OF_OPERATORS])(const SyntaxNode * const node) = {
	[ OP_NUMBER      ] = derive_number  ,
	[ OP_NEGATE      ] = derive_negate  ,
	[ OP_PLUS        ] = derive_plus    ,
	[ OP_MINUS       ] = derive_minus   ,
	[ OP_TIMES       ] = derive_times   ,
	[ OP_DIVIDE      ] = derive_divide  ,
	[ OP_POWER       ] = derive_power   ,
	[ OP_EXP         ] = derive_exp     ,
	[ OP_LN          ] = derive_ln      ,
	[ OP_SIN         ] = derive_sine    ,
	[ OP_COS         ] = derive_cosine  ,
	[ OP_TAN         ] = derive_tangent ,
	[ OP_X_VAR       ] = derive_x       ,
	[ OP_PI          ] = derive_number  ,
	[ OP_E           ] = derive_number  ,
	[ OP_LEFT_BRACE  ] = derive_failure ,
	[ OP_RIGHT_BRACE ] = derive_failure ,
	[ OP_UNKNOWN     ] = derive_failure ,
};


SyntaxNode* syntax_node_construct(Operator op, double number) {
	SyntaxNode *ptr = (SyntaxNode *)malloc(sizeof(SyntaxNode));
	if (!ptr) {
		fprintf(stderr, "Memory error: Could not allocate memory for syntax node.\n");
		return NULL;
	}
	*ptr = (SyntaxNode){op, number, operator_arity[op]};
	return ptr;
}

SyntaxNode *syntax_node_copy(const SyntaxNode * const original){
	SyntaxNode * const copy = syntax_node_construct(original->operator_value, original->numeric_value);
	if (!copy) {
		fprintf(stderr, "Memory error: Could not copy syntax node.\n");
		return NULL;
	}

	/* recursively copy and add the child nodes of the original to the copy */
	for (int i = 0; i < copy->arity; i++) {
		copy->operand[i] = syntax_node_copy(original->operand[i]);
		if (!copy->operand[i]) {
			/* Free previously allocated children */
			for (int j = i-1; j >= 0; --j) {
				syntax_node_destroy(copy->operand[j]);
			}
			/* Free this copy */
			free(copy);
			return NULL;
		}
	}

	return copy;
}

void syntax_node_destroy(SyntaxNode *node) {
	assert (node != NULL);

	/* destroy all child nodes first or else we get a memory leak */
	for (int i = 0; i < node->arity; i++) {
		syntax_node_destroy(node->operand[i]);
		node->operand[i] = NULL;
	}

	/* free the memory */
	free(node);
}

int syntax_node_condense(SyntaxNode *node) {
	// a variable can never be condensed
	if (node->operator_value == OP_X_VAR) {
		return 0;
	}
	
	// this will tell us whether the operands are condensable (defaults to 1
	// because numbers and constants can be condensed)
	int condensable = 1;
	// now check if the operands can all be condensed by recursively condensing them.
	for (int i = 0; i < node->arity; i++) {
		// if at least one operand is not condensable the value will be 0
		condensable *= syntax_node_condense(node->operand[i]);
	}
	
	// if we can condense we should do so now
	if (condensable) {
		// The number value of the node will be the value of its operation...
		node->numeric_value = syntax_node_operate(node, 0.0); //value parameter does not matter
		// ... and its type will be that of a number node (don't reverse the order of these two!)
		node->operator_value = OP_NUMBER;
		// free all the child nodes, set their pointers to NULL...
		for (int i = 0; i < node->arity; ++i) {
			syntax_node_destroy(node->operand[i]);
			node->operand[i] = NULL;
		}
		// ... and set he node's arity to 0.
		node->arity = 0;
	}
	
	return condensable;
}

int syntax_node_is_constant(SyntaxNode *node) {
	/* Variables are never constant. */
	if (node->operator_value == OP_X_VAR) {
		return 0;
	}

	int is_constant = 1; /* True */
	for (int i = 0; i < node->arity; i++) {
		/* If any child is not constant the result will be 0. */
		is_constant *= syntax_node_is_constant(node->operand[0]);
		if (!is_constant) {break;}
	}

	return is_constant;
}

/*--[ Operations ]------------------------------------------------------------*/

double syntax_node_operate(SyntaxNode *node, double value) {
	return operation_list[node->operator_value](node, value);
}

static double syntax_node_operation_number_value(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_NUMBER);
	return node->numeric_value;
}

static double syntax_node_operation_negate(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_NEGATE);
	return -1.0 * syntax_node_operate(node->operand[0], value);
}

static double syntax_node_operation_add(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_PLUS);
	return syntax_node_operate(node->operand[0], value) + syntax_node_operate(node->operand[1], value);
}

static double syntax_node_operation_subtract(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_MINUS);
	return syntax_node_operate(node->operand[0], value) - syntax_node_operate(node->operand[1], value);
}

static double syntax_node_operation_multiply(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_TIMES);
	return syntax_node_operate(node->operand[0], value) * syntax_node_operate(node->operand[1], value);
}

static double syntax_node_operation_divide(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_DIVIDE);
	double divisor = syntax_node_operate(node->operand[1], value);
	if (divisor == 0) {
		fprintf(stderr, "Method error: trying to divide by zero. Try another guess value.\n");
		exit(1);
	}
	return syntax_node_operate(node->operand[0], value) / divisor;
}

static double syntax_node_operation_power(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_POWER);
	return pow(syntax_node_operate(node->operand[0], value), syntax_node_operate(node->operand[1], value));
}

static double syntax_node_operation_exponent(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_EXP);
	return exp(syntax_node_operate(node->operand[0], value));
}

static double syntax_node_operation_logarithmus_naturalis(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_LN);
	return log(syntax_node_operate(node->operand[0], value));
}

static double syntax_node_operation_sine(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_SIN);
	return sin(syntax_node_operate(node->operand[0], value));
}

static double syntax_node_operation_cosine(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_COS);
	return cos(syntax_node_operate(node->operand[0], value));
}

static double syntax_node_operation_tangent(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_TAN);
	return tan(syntax_node_operate(node->operand[0], value));
}

static double syntax_node_operation_x(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_X_VAR);
	return value;
}

static double syntax_node_operation_pi(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_PI);
	return PI;
}

static double syntax_node_operation_e(SyntaxNode *node, double value) {
	assert(node->operator_value == OP_E);
	return E;
}

static double syntax_node_operation_failure(SyntaxNode *node, double value) {
	assert(0); /* Fails always */
}

SyntaxNode *syntax_node_derive(SyntaxNode *node) {
	return derivation_table[node->operator_value](node);
}

static SyntaxNode *derive_number(const SyntaxNode * const node) {
	assert(node->operator_value == OP_NUMBER);
	return syntax_node_construct(OP_NUMBER, 0.0);
}

static SyntaxNode *derive_negate(const SyntaxNode * const node) {
	assert(node->operator_value == OP_NEGATE);
	SyntaxNode *new_node = syntax_node_construct(OP_NEGATE, 0.0);
	new_node->operand[0] = syntax_node_derive(node->operand[0]);
	return new_node;
}

static SyntaxNode *derive_plus(const SyntaxNode * const node) {
	assert(node->operator_value == OP_PLUS);
	SyntaxNode *new_node = syntax_node_construct(OP_PLUS, 0.0);

	new_node->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[1] = syntax_node_derive(node->operand[1]);

	return new_node;
}

static SyntaxNode *derive_minus(const SyntaxNode * const node) {
	assert(node->operator_value == OP_MINUS);
	SyntaxNode *new_node = syntax_node_construct(OP_MINUS, 0.0);

	new_node->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[1] = syntax_node_derive(node->operand[1]);

	return new_node;
}

static SyntaxNode *derive_times(const SyntaxNode * const node) {
	assert(node->operator_value == OP_TIMES);
	SyntaxNode *new_node = syntax_node_construct(OP_PLUS, 0.0);

	new_node->operand[0] = syntax_node_construct(OP_TIMES, 0.0);
	new_node->operand[1] = syntax_node_construct(OP_TIMES, 0.0);
	
	new_node->operand[0]->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[0]->operand[1] = syntax_node_copy(node->operand[1]);
	
	new_node->operand[1]->operand[0] = syntax_node_copy(node->operand[0]);
	new_node->operand[1]->operand[1] = syntax_node_derive(node->operand[1]);
	
	return new_node;
}

static SyntaxNode *derive_divide(const SyntaxNode * const node) {
	assert(node->operator_value == OP_DIVIDE);
	SyntaxNode *new_node = syntax_node_construct(OP_DIVIDE, 0.0);

	new_node->operand[0] = syntax_node_construct(OP_MINUS, 0.0);
	new_node->operand[1] = syntax_node_construct(OP_TIMES, 0.0);
	
	new_node->operand[0]->operand[0] = syntax_node_construct(OP_TIMES, 0.0);
	new_node->operand[0]->operand[1] = syntax_node_construct(OP_TIMES, 0.0);
	
	new_node->operand[0]->operand[0]->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[0]->operand[0]->operand[1] = syntax_node_copy(node->operand[1]);
	
	new_node->operand[0]->operand[1]->operand[0] = syntax_node_copy(node->operand[0]);
	new_node->operand[0]->operand[1]->operand[1] = syntax_node_derive(node->operand[1]);
	
	new_node->operand[1]->operand[0] = syntax_node_copy(node->operand[1]);
	new_node->operand[1]->operand[1] = syntax_node_copy(node->operand[1]);
	
	return new_node;
}

static SyntaxNode *derive_power(const SyntaxNode * const node) {
	assert(node->operator_value == OP_POWER);
	SyntaxNode *new_node = syntax_node_construct(OP_TIMES, 0.0);
	
	/* A ^ B = exp(ln(A) * B)              */
	/* A ^ B * ((A' / A) * B + ln(A) * B') */

	new_node->operand[0] = syntax_node_construct(OP_POWER, 0.0);
	new_node->operand[1] = syntax_node_construct(OP_PLUS , 0.0);

	// A ^ B
	new_node->operand[0]->operand[0] = syntax_node_copy(node->operand[0]);
	new_node->operand[0]->operand[1] = syntax_node_copy(node->operand[1]);

	// [A' / A] * [B] + [ln(A)] * [B']
	new_node->operand[1]->operand[0] = syntax_node_construct(OP_TIMES, 0.0);
	new_node->operand[1]->operand[1] = syntax_node_construct(OP_TIMES, 0.0);

	// [A' / A] * [B]
	new_node->operand[1]->operand[0]->operand[0] = syntax_node_construct(OP_DIVIDE, 0.0);
	new_node->operand[1]->operand[0]->operand[1] = syntax_node_copy(node->operand[1]);

	// A' / A
	new_node->operand[1]->operand[0]->operand[0]->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[1]->operand[0]->operand[0]->operand[1] = syntax_node_copy(  node->operand[0]);

	// [ln(A)] * B'
	new_node->operand[1]->operand[1]->operand[0] = syntax_node_construct(OP_LN, 0.0);
	new_node->operand[1]->operand[1]->operand[1] = syntax_node_derive(node->operand[1]);

	new_node->operand[1]->operand[1]->operand[0]->operand[0] = syntax_node_copy(node->operand[0]);
	
	// If the exponent is a real number, its derivative will be 0. Remove that
	// part from the sum to remove the ln; we need to to this, because ln(x) is
	// forbidden for negative numbers, but pow(x, y) is allowed for negative x and
	// integer y.
	if (syntax_node_is_constant(node->operand[1])) { // if the B can be condensed into a number
		// destroy it the OP_TIMES from (ln(A) * B')
		syntax_node_destroy(new_node->operand[1]->operand[1]);
		// and create new node with a value of 0 in its place
		new_node->operand[1]->operand[1] = syntax_node_construct(OP_NUMBER, 0.0);
	}
	
	return new_node;
}

static SyntaxNode *derive_exp(const SyntaxNode * const node) {
	assert(node->operator_value == OP_EXP);
	SyntaxNode *new_node = syntax_node_construct(OP_TIMES, 0.0);

	new_node->operand[0] = syntax_node_derive(syntax_node_copy(node->operand[0]));
	new_node->operand[1] = syntax_node_copy(node->operand[0]);

	return new_node;
}

static SyntaxNode *derive_ln(const SyntaxNode * const node) {
	assert(node->operator_value == OP_LN);
	SyntaxNode *new_node = syntax_node_construct(OP_TIMES, 0.0);

	new_node->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[1] = syntax_node_construct(OP_DIVIDE, 0.0);

	new_node->operand[1]->operand[0] = syntax_node_construct(OP_NUMBER, 1.0);
	new_node->operand[1]->operand[1] = syntax_node_copy(node->operand[0]);

	return new_node;
}

static SyntaxNode *derive_sine(const SyntaxNode * const node) {
	assert(node->operator_value == OP_SIN);
	SyntaxNode *new_node = syntax_node_construct(OP_TIMES, 0.0);

	new_node->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[1] = syntax_node_construct(OP_COS, 0.0);

	new_node->operand[1]->operand[0] = syntax_node_copy(node->operand[0]);

	return new_node;
}

static SyntaxNode *derive_cosine(const SyntaxNode * const node) {
	assert(node->operator_value == OP_COS);
	SyntaxNode *new_node = syntax_node_construct(OP_TIMES, 0.0);

	new_node->operand[0] = syntax_node_construct(OP_NUMBER, -1.0);
	new_node->operand[1] = syntax_node_construct(OP_TIMES, 0);

	new_node->operand[1]->operand[0] = syntax_node_construct(OP_SIN, 0);
	new_node->operand[1]->operand[1] = syntax_node_derive(node->operand[0]);

	new_node->operand[1]->operand[0]->operand[0] = syntax_node_copy(node->operand[0]);

	return new_node;
}

static SyntaxNode *derive_tangent(const SyntaxNode * const node) {
	assert(node->operator_value == OP_TAN);
	SyntaxNode *new_node = syntax_node_construct(OP_DIVIDE, 0.0);

	new_node->operand[0] = syntax_node_derive(node->operand[0]);
	new_node->operand[1] = syntax_node_construct(OP_POWER, 1.0);

	new_node->operand[1]->operand[0] = syntax_node_construct(OP_COS, 1.0);
	new_node->operand[1]->operand[1] = syntax_node_construct(OP_NUMBER, 2.0);

	new_node->operand[1]->operand[0]->operand[0] = syntax_node_copy(node->operand[0]);
	return new_node;
}

static SyntaxNode *derive_x(const SyntaxNode * const node) {
	assert(node->operator_value == OP_X_VAR);
	return syntax_node_construct(OP_NUMBER, 1.0);
}

static SyntaxNode *derive_failure(const SyntaxNode * const node) {
	assert(0); /* Fails always. */
}

Operator char_to_operator(char *c) {
	Operator o = OP_UNKNOWN;

	switch (*c) {
		case '+' : o = OP_PLUS        ; break;
		case '-' : o = OP_MINUS       ; break;
		case '*' : o = OP_TIMES       ; break;
		case '/' : o = OP_DIVIDE      ; break;
		case '^' : o = OP_POWER       ; break;
		case '(' :
		case '[' : o = OP_LEFT_BRACE  ; break;
		case ')' :
		case ']' : o = OP_RIGHT_BRACE ; break;
	}

	return o;
}

Operator string_to_operator(char *s) {
	/* exp, ln, sin, cos, tan, x, pi, e */
	if (strcmp(s, "exp") == 0) {
		return OP_EXP;
	}
	if (strcmp(s, "ln") == 0) {
		return OP_LN;
	}
	if (strcmp(s, "sin") == 0) {
		return OP_SIN;
	}
	if (strcmp(s, "cos") == 0) {
		return OP_COS;
	}
	if (strcmp(s, "tan") == 0) {
		return OP_TAN;
	}
	if (strcmp(s, "x") == 0 || strcmp(s, "X") == 0) {
		return OP_X_VAR;
	}
	if (strcmp(s, "pi") == 0 || strcmp(s, "PI") == 0 || strcmp(s, "Pi") == 0) {
		return OP_PI;
	}
	if (strcmp(s, "e") == 0 || strcmp(s, "E") == 0) {
		return OP_E;
	}

	/* If everything else fails. */
	return OP_UNKNOWN;
}

