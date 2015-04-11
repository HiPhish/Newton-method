#include <stdlib.h>
#include <assert.h>

#include "parser.h"

/** Default starting size for the operator- and operand stack. */
#define STACK_SIZE_DEFAULT  7

/** Associativity of an operator
 *
 *  Most operator are left-associative, only functions and the _power_ operator
 *  are right-associative. The shunting-yard algorithm needs this information.
 */
enum operator_associativity {
	LEFT_ASSOC,
	RIGHT_ASSOC
};

/** Stack containing pointers to syntax nodes. */
struct syntax_node_stack {
	/** Store pointer to pointer of nodes, because the stack will be thrown away later. **/
	struct syntax_node **stack;

	/** The maximum size of the stack.
 	 *
 	 *  needed in order to decide how much memory to allocate per stack. This
 	 *  value can grow and shrink as the program is running.
 	 */
	int capacity;

	/** Index of the topmost element, equals -1 for empty stacks. */
	int top;
};

/** Allocate and initialise a new syntax node stack.
 *
 *  @return  Pointer to the new stack.
 */
static struct syntax_node_stack * syntax_node_stack_init(int maxSize );

/** Deallocate a syntax node stack.
 *
 *  This method deallocates the stack but does not invalidate any pointers to
 *  it.
 *
 *  @param stack  The stack to destroy.
 */
static void syntax_node_stack_destroy(struct syntax_node_stack *stack );

/** Push a new pointer onto the stack.
 *
 *  @param stack  The stack to push onto.
 *  @param node   Pointer to the node to push.
 */
static void syntax_node_stack_push(struct syntax_node_stack *stack, struct syntax_node *node );

/** Pop a pointer off the stack.
 *
 *  @param stack  Stack to pop from.
 *
 *  @return  Pointer to the node that has been popped off the stack.
 */
static struct syntax_node *syntax_node_stack_pop(struct syntax_node_stack *stack );

/** Returns the top element of the stack without actually popping it.
 *
 *  This is handy if all you want is to inspect the top element.
 *
 *  @param stack  The stack whose top element we want.
 *
 *  @return  Pointer to the topmost node of the stack.
 */
static struct syntax_node *syntax_node_stack_top(struct syntax_node_stack *stack);

/** Whether the stack is empty.
 *
 *  @param stack  Stack to inspect.
 *
 *  @return  1 if the stack is empty, 0 otherwise.
 */
static int syntax_node_stack_is_empty(struct syntax_node_stack *stack);

/** Whether the stack is full.
 *
 *  @param stack  Stack to inspect.
 *
 *  @return  1 if the stack is full, 0 otherwise.
 */
static int syntax_node_stack_is_full(struct syntax_node_stack *stack);

/** Pops all operators off the operator stack.
 *
 *  This function will pop one operator after the other as long as the stack is
 *  not empty.  It is called automatically when the parser is being finished.
 *
 *  @param p  Parser to work with.
 */
static void pop_operator_stack(struct parser *p);

/** Pops a single operator off the operator stack.
 *
 *  This function will pop one operator off the stack and then pops as many
 *  operands form the operand stack as the arity of the operator is and adds
 *  them as children to the operator. The operator is then pushed onto the
 *  operand stack as the root of a new tree. This tree might later become and
 *  operand of another operator that got popped as well.
 *
 *  @param p  Parser to work with.
 */
static void pop_operator(struct parser *p);

/** Checks if an operator token represents a function.
 *
 *  @param t  The token to check.
 *
 *  @return  Returns 1 if the token is a function, 0 otherwise.
 */
static int token_is_function(enum operator_type t);

static int token_is_operator(enum operator_type t);
static int token_is_constant(enum operator_type t);

static int token_is_variable(enum operator_type t);
static enum operator_associativity op_assoc(enum operator_type t);

static struct syntax_node_stack *syntax_node_stack_init(int capacity) {
	struct syntax_node_stack *stack = malloc(sizeof(struct syntax_node_stack));
	struct syntax_node **new_contents = NULL; /* pointer to the stack elements */
	new_contents = (struct syntax_node **)malloc(capacity * sizeof(struct syntax_node *));
	
	if (new_contents == NULL) {
		fprintf(stderr, "Insufficient memory to initialize stack.\n");
		exit(1); /* Exit, returning error code. */
	}
	
	stack->stack    = new_contents;
	stack->capacity = capacity;
	stack->top      = -1; /* empty */
	
	return stack;
}

static void syntax_node_stack_destroy(struct syntax_node_stack *stack) {
	free(stack->stack); // free the memory occupied by the array

	stack->stack = NULL;
	stack->capacity = 0;
	stack->top = -1;
	
	free(stack);
}

static void syntax_node_stack_push(struct syntax_node_stack *stack,  struct syntax_node *node ) {
	if (syntax_node_stack_is_full(stack)) {
		/* if the stack is full try to reallocate it with twice the size */
		if (!(stack->stack = (struct syntax_node **)realloc(stack->stack, sizeof(struct syntax_node *) * (stack->capacity * 2)))) {
			fprintf(stderr, "Can't push element on stack: stack is full and cannot be enlarged.\n");
			exit(1);
		}
		stack->capacity *= 2;
	}
	
	stack->stack[++(stack->top)] = node;
}

static struct syntax_node *syntax_node_stack_pop(struct syntax_node_stack *stack) {
	if (syntax_node_stack_is_empty(stack)) {
		fprintf(stderr, "Error: stack underflow.\n");
		exit(1);
	}
	/* if the stack is only one quarter full cut it in half */
	if (4 * stack->top <= stack->capacity) {
		if (!(stack->stack = (struct syntax_node **)realloc(stack->stack, sizeof(struct syntax_node *) * (2 * stack->top + 1)))) {
			fprintf(stderr, "Can't trim stack.\n");
			exit(1);
		}
		stack->capacity = 2 * stack->top + 1;
	}
	struct syntax_node *ptr = stack->stack[stack->top];
	stack->stack[(stack->top)--] = NULL;
	return ptr;
}

static struct syntax_node *syntax_node_stack_top(struct syntax_node_stack *stack) {
	if (syntax_node_stack_is_empty(stack)) {
		return NULL;
	}
	return stack->stack[stack->top];
}

static int syntax_node_stack_is_empty(struct syntax_node_stack * stack ) {
	return stack->top < 0 ? 1 : 0; /* if the top is negative, the stack is empty */
}

static int syntax_node_stack_is_full(struct syntax_node_stack * stack ) {
	return stack->top >= (stack->capacity - 1);
}


struct parser *parser_init() {
	struct parser *p = malloc(sizeof(struct parser));
	p->operand_stack   = syntax_node_stack_init(STACK_SIZE_DEFAULT);
	p->operator_stack = syntax_node_stack_init(STACK_SIZE_DEFAULT);

	return p;
}

struct syntax_node *parser_finish(struct parser *p) {
	struct syntax_node *result = NULL;

	/* pop all the operators from the operator stack onto the operand stack */
	pop_operator_stack(p);
	
	/* destroy the operator stack */
	syntax_node_stack_destroy(p->operator_stack);
	
	/* if there is more than one tree on the output stack we have a syntax error */
	if (p->operand_stack->top > 1) {
		fprintf(stderr, "Syntax Error: More operands than operators");
		return result; /* NULL */
	}
	
	/* save the syntax tree of the function */
	result = syntax_node_stack_top(p->operand_stack);
	/* finally, destroy the operand stack as well */
	syntax_node_stack_destroy(p->operand_stack);

	return result;
}

void parser_parse_node(struct parser *p, struct syntax_node *node) {
	p->previous_node = node;
	enum operator_type token = node->operator_value;

	/* Numbers, constants and variables always on the operand stack. */
	if (token == OP_NUMBER || token_is_constant(token) || token_is_variable(token)) {
		syntax_node_stack_push(p->operand_stack, node);
		return;
	}

	/* Functions always go on the operator stack. */
	if (token_is_function(token)) {
		syntax_node_stack_push(p->operator_stack, node);
		return;
	}
	
	/* If the token is an operator. */
	if (token_is_operator(token)) {
		/* As long as there is an operator on the stack... */
		while (!syntax_node_stack_is_empty(p->operator_stack)) {
			/* Operator on top of the operator stack. */
			#define STACK_TOP syntax_node_stack_top(p->operator_stack)->operator_value
			/* Precedences, and asserting they are not 0 */
			unsigned int precedence_current = operator_precedence[token ];
			unsigned int precedence_top     = operator_precedence[STACK_TOP];
			assert(precedence_current != 0);
			assert(precedence_top     != 0);

			/* ...and this operator is either
 			 * a) left-associative and has less or equal precedence
 			 *    than the top of the operator stack
 			 * or
 			 * b) right-associative and has less precedence than the
 			 *    top of the operator stack
 			 */

			#define CONDITION_A ( \
				op_assoc(token) == LEFT_ASSOC \
				&& \
				precedence_current \
				<= \
				precedence_top \
				)

			#define CONDITION_B ( \
				op_assoc(token) == RIGHT_ASSOC \
				&& \
				precedence_current \
				< \
				precedence_top \
				)
			if (CONDITION_A || CONDITION_B) {
				pop_operator(p);
			} else {
				break;
			}
			#undef CONDITION_A
			#undef CONDITION_B
			#undef TOP_OP
		}
		syntax_node_stack_push(p->operator_stack, node);
		return;
	}
	
	/* If the token is a left parenthesis push it onto the operator stack */
	if (token == OP_LEFT_BRACE) {
		syntax_node_stack_push(p->operator_stack, node);
		return;
	}
	
	/* If the token is a right parenthesis */
	if (token == OP_RIGHT_BRACE) {
		while (!syntax_node_stack_is_empty(p->operator_stack)) {
			/* If we find a matching closing brace... */
			if (syntax_node_stack_top(p->operator_stack)->operator_value == OP_LEFT_BRACE) {
				/* pop the left brace and free its memory, we don't need it anymore */
				free(syntax_node_stack_pop(p->operator_stack));
				/* If the top of the operator stack is a function pop it as well. */
				#define STACK_TOP syntax_node_stack_top(p->operator_stack)->operator_value
				if (!syntax_node_stack_is_empty(p->operator_stack) && token_is_function(STACK_TOP)) {
					pop_operator(p);
				}
				#undef TOP_OP
				/* we have our closing brace, so break out of the function */
				return;
			}
			/* Otherwise pop the next operator from the stack */
			pop_operator(p);
		}

		/* If we made it to here it means we have emptied the stack  *
		 * without finding the closing brace. That's a syntax error. */
		fprintf(stderr, "Syntax error: no matching opening parenthesis found.\n");
		exit(1);
	}

	/* If the token none of these types, then we have an undefined type */
	fprintf(stderr, "Syntax error: unknown type of token %i.\n", token);
	exit(1);
}

static void pop_operator_stack(struct parser *p) {
	while (!syntax_node_stack_is_empty(p->operator_stack)) {
		/* By this point there should be no opening parentheses      *
		 * on the stack anymore. If there are we have a syntax error */
		if(syntax_node_stack_top(p->operator_stack)->operator_value == OP_LEFT_BRACE){
			fprintf(stderr, "Syntax error: no matching closing parenthesis found.\n");
			exit(1);
		}
		pop_operator(p);
	}
}

static int token_is_function(enum operator_type t) {
	return (t == OP_EXP || t == OP_LN || t == OP_SIN || t == OP_COS || t == OP_TAN) ? 1 : 0;
}

static int token_is_operator(enum operator_type t) {
	return (t == OP_NEGATE || t == OP_PLUS || t == OP_MINUS || t == OP_TIMES || t == OP_DIVIDE || t == OP_POWER) ? 1 : 0;
}

static int token_is_constant(enum operator_type t) {
	return (t == OP_PI || t == OP_E) ? 1 : 0;
}

static int token_is_variable(enum operator_type t) {
	return t == OP_X_VAR ? 1 : 0;
}

enum operator_associativity op_assoc(enum operator_type t) {
	return (t == OP_POWER || t == OP_NEGATE|| token_is_function(t)) ? RIGHT_ASSOC : LEFT_ASSOC;
}

void pop_operator(struct parser *p) {
	/* pop operators off the operator stack, onto the output stack */
	struct syntax_node *op = syntax_node_stack_pop(p->operator_stack);
	/* pop the right amount of operands off the output stack  *
	 * and set them as operands to this node (back to front). */
	for (int i = op->arity; i > 0; --i) {
		if (syntax_node_stack_is_empty(p->operand_stack)) {
			fprintf(stderr, "Syntax error: operator has not enough operands");
			exit(1);
		}
		op->operand[i - 1] = syntax_node_stack_pop(p->operand_stack);
	}
	
	/* add this operator node to the output stack */
	syntax_node_stack_push(p->operand_stack, op);
}

