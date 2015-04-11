#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vm.h"
#include "opcodes.h"

/** Stack of double floating point numbers.
 *
 *  We use a *count* member rather than a *top* because the type is *size_t*,
 *  which is an unsigned integer type. An empty stack has a *count* of *0*,
 *  whereas the top would have to be an impossible number like *-1*. Since the
 *  type is unsigned this would wrap around be the highest possible unsigned
 *  number, breaking any comparison with the capacity in the process.
 */
typedef struct number_stack {
	double *stack;    /**< Underlying array of numbers.     */
	size_t  count;    /**< Number of elements on the stack. */
	size_t  capacity; /**< Length of the stack array.       */
} NumberStack;

/** Push a number onto the stack.
 *
 *  @param stack  Stack to push the value on.
 *  @param value  Value to push on the stack.
 *  
 *  @return  0 is everything went right, non-0 otherwise.
 *  
 *  If the stack is too short to hold the new value it will double in size. If
 *  an error occurs during the re-allocation an error code will be returned.
 */
static int number_stack_pop(NumberStack *stack, double *value);

/** Pop a number off the stack.
 *
 *  @param stack  Stack to pop the value off.
 *  @param value  Value to pop off the stack.
 *  
 *  @return  0 is everything went right, non-0 otherwise.
 *  
 *  If the stack is only one quarter full after popping it will half in size.
 *  If an error occurs during the re-allocation an error code will be returned.
 */
static int number_stack_push(NumberStack *stack, double value);

static int number_stack_pop(NumberStack *stack, double *value) {
	assert(0 < stack->count && stack->count <= stack->capacity);

	int error = 0; /* No error. */
	*value = stack->stack[--stack->count];

	/* Stack will shrink by half if it is only one quarter full, but avoid a
 	 * stack with no capacity.
 	 *
 	 * Example: capacity = 8, count = 2
 	 *          => 4*count = 4*2 = 8 = capacity
 	 *          => capacity shrunk to 2*2 = 4
 	 */
	if (4 * stack->count <= stack->capacity && stack->count > 0) {
		double *new_stack = realloc(stack->stack, (2 * stack->count) * sizeof(double));
		if (!new_stack) {error = 1; goto end;}
		stack->stack = new_stack;
		stack->capacity = 2 * stack->count;
	}

end:
	assert(0 <= stack->count && stack->count < stack->capacity);
	return error;
}

static int number_stack_push(NumberStack *stack, double value) {
	assert(0 <= stack->count && stack->count <= stack->capacity);

	int error = 0; /* No error. */

	/* Stack will grow if it is full (top == capacity - 1)
 	 *
	 * Example: capacity = 4, top = 3
 	 *          => Stack full; after increment: top == capacity
 	 */
	if(stack->count == stack->capacity) {
		double *new_stack = realloc(stack->stack, 2 * stack->count * sizeof(double));
		if (!new_stack) {error = 1; goto end;}
		stack->stack = new_stack;
		stack->capacity = 2 * stack->count;
	}
	stack->stack[stack->count++] = value;

end:
	assert(0 < stack->count && stack->count <= stack->capacity);
	return error;
}


VMCode machine_load_code(VirtualMachine *machine, VMCode code) {
	VMCode old_code = machine->code;
	machine->code   = code;

	return old_code;
}

int machine_execute(VirtualMachine *machine, double *result) {
	int error = 0;                      /**< Exit status, 0 mean no error.    */
	int index = machine->code.length-1; /**< Current index into the bytecode. */

	double tmp[MAX_ARITY]; /**< Temporary registers.       */
	int    tmp_i = 0;      /**< Index of current register. */

	/** Stack of intermediate numbers. */
	NumberStack stack = {
		.stack    = malloc(2 * sizeof(double)),
		.count    = 0,
		.capacity = 2,
	};

	if (!stack.stack) {
		error = 1;
		goto end;
	}

	/* Macros to save repetitious typing and for readability. */

	/** Pop a number off this function's stack. */
	#define POP  { \
		assert(tmp_i < MAX_ARITY); \
		if(number_stack_pop(&stack, &tmp[tmp_i++]) != 0) { \
			error = 1; \
			goto end; \
		}; \
	}

	/** Push a number to function's stack. */
	#define PUSH(value) { \
		if (number_stack_push(&stack, value) != 0) { \
			error = 1; \
			goto end; \
		} \
		tmp_i = 0; \
	}


	while (index >= 0) {
		if (error != 0) {goto end;}
		uint8_t opcode = machine->code.code[index--];

		switch (opcode) {
		case OPC_NUM: {
			uint8_t *ptr   = machine->code.code + index - sizeof(double) + 1;
			double  number = *((double*)ptr);
			PUSH(number)
			index -= sizeof(double);
			break;
		}
		case OPC_NEG   :  POP       PUSH(    -tmp[0]           )  break;
		case OPC_ADD   :  POP  POP  PUSH(     tmp[0] + tmp[1]  )  break;
		case OPC_SUB   :  POP  POP  PUSH(     tmp[0] - tmp[1]  )  break;
		case OPC_MULT  :  POP  POP  PUSH(     tmp[0] * tmp[1]  )  break;
		case OPC_DIV   :  POP  POP  PUSH(     tmp[0] / tmp[1]  )  break;
		case OPC_POW   :  POP  POP  PUSH( pow(tmp[0] , tmp[1]) )  break;
		case OPC_EXP   :  POP       PUSH( exp(tmp[0])          )  break;
		case OPC_LN    :  POP       PUSH( log(tmp[0])          )  break;
		case OPC_SIN   :  POP       PUSH( sin(tmp[0])          )  break;
		case OPC_COS   :  POP       PUSH( cos(tmp[0])          )  break;
		case OPC_TAN   :  POP       PUSH( tan(tmp[0])          )  break;
		case OPC_VAR_X :            PUSH( machine->reg_x       )  break;
		case OPC_PI    :            PUSH( PI                   )  break;
		case OPC_E     :            PUSH( E                    )  break;

		default: error = 1; break; /* Unknown opcode. */
		}
	}
	assert(stack.count == 1); /* Only only one number left on the stack. */
	*result = stack.stack[0];

end:
	if (stack.stack) {free(stack.stack);}
	return error;

	#undef POP
	#undef PUSH
}

