#include <stdlib.h>
#include <assert.h>

#include "backend.h"
#include "../../virtual_machine/opcodes.h"

/* Compilation:
 *
 * To compile a tree start with the root and compile recursively every node. If
 * the node is a number literal write the number literal opcode followed by the
 * literal representation of the number. If it's an operator write the opcode
 * of the operator and then compile recursively all children. Children are
 * compiled depth-first. The resulting sequence is the arithmetic expression in
 * Polish notation.
 *
 * Number literals are written in front of their opcode because the bytecode
 * will be read in reverse order.
 */

/** Maps a syntax node operator to a VM bytecode.
 *
 *  Some operators are not mapped to opcodes because they don't have any. In
 *  particular those operators are the constants and braces.
 */
static uint8_t operator_to_opcode[NUMBER_OF_OPERATORS] = {
	[ OP_NUMBER ] = OPC_NUM   ,
	[ OP_NEGATE ] = OPC_NEG   ,
	[ OP_PLUS   ] = OPC_ADD   ,
	[ OP_MINUS  ] = OPC_SUB   ,
	[ OP_TIMES  ] = OPC_MULT  ,
	[ OP_DIVIDE ] = OPC_DIV   ,
	[ OP_POWER  ] = OPC_POW   ,
	[ OP_EXP    ] = OPC_EXP   ,
	[ OP_LN     ] = OPC_LN    ,
	[ OP_SIN    ] = OPC_SIN   ,
	[ OP_COS    ] = OPC_COS   ,
	[ OP_TAN    ] = OPC_TAN   ,
	[ OP_X_VAR  ] = OPC_VAR_X ,
};

/** Compile an individual syntax node to bytecode.
 *
 *  @param node  The node to compile.
 *  @param code  The code object to compile to.
 *  @param tail  Index into the bytecode sequence.
 *
 *  @return 0 on success, non-0 on error.
 */
static int compile_syntax_node(const SyntaxNode *const node, VMCode *code);

/** Writes an opcode byte to the bytecode.
 *
 *  @param opcode  The opcode to write.
 *  @param code    code object to write to.
 *  @param position  Position in the bytecode sequence.
 *
 *  @return 0 on success, non-0 on error.
 *
 *  If the bytecode sequence is too short to hold the opcode it will grow. The
 *  reallocation can fail, so check the return value of the function.
 */
static int write_opcode(uint8_t opcode, VMCode *code);

/** Writes the naked bytes of a number to the bytecode.
 *
 *  @param number    The number to write.
 *  @param code      The VMCode object to write to.
 *  @param position  Index of the bytecode sequence tail.
 *
 *  @return  0 if everything went right, non-0 otherwise.
 *
 *  The exact format of the number is specific to the machine the program has
 *  been compiled for (the physical machine the user is running this on, not
 *  the VM).
 */
static int write_number(double number, VMCode *code);

/** Grows the bytecode array to fit in more bytes.
 *
 *  @param code      Pointer to the VMCode object to grow.
 *  @param position  Position in the bytecode where the new opcode is to be
 *                   written to.
 *
 *  return  0 if everything went right, non-0 otherwise.
 */
static int grow_bytecode(VMCode *code, size_t by);


int compiler_backend(const SyntaxNode *const tree, VMCode **code) {
	assert(*code == NULL); /* The code object must be NULL */

	int error = 0; /* No error */

	#define CODE_LENGTH  64 /**< Default size of the code array. */
	*code = malloc(sizeof(VMCode));
	if (!*code) {error = 1; goto end;}
	(*code)->length   = 0;
	(*code)->capacity = CODE_LENGTH;
	(*code)->code     = malloc(CODE_LENGTH * sizeof(uint8_t));
	if (!(*code)->code) {free(*code); *code = NULL; error = 1; goto end;}

	// If compilation fails at any point the bytecode is invalid, so delete it.
	if (compile_syntax_node(tree, *code) != 0) {
		error = 1;
		free((*code)->code);
		free(*code);
		*code = NULL;
		goto end;
	}

end:
	return error;
	#undef CODE_LENGTH
}

static int compile_syntax_node(const SyntaxNode *const node, VMCode *code) {
	#define CHECK_EXIT_STATUS  if (error != 0) {goto end;}
	int     error = 0; /**< Indicate success or failure.     */
	uint8_t opcode;    /**< Opcode to write to the bytecode. */

	/* Opcode of this node, get it, don't write it yet */
	opcode = operator_to_opcode[node->operator_value];

	/* If the node is a number write the number to the bytecode. */
	if (opcode == OPC_NUM) {
		error = write_number(node->numeric_value, code);
		CHECK_EXIT_STATUS
	}

	error = write_opcode(opcode, code);
	CHECK_EXIT_STATUS
	for (int i = 0; i < node->arity; ++i) {
		error = compile_syntax_node(node->operand[i], code);
		CHECK_EXIT_STATUS
	}

end:
	return error;
	#undef CHECK_EXIT_STATUS
}

static int write_opcode(uint8_t opcode, VMCode *code) {
	/* If the opcode does not fit reallocate */
	if (grow_bytecode(code, sizeof(uint8_t)) != 0) {return 1;}

	code->code[code->length++] = opcode;

	return 0;
}

static int write_number(double number, VMCode *code) {
	assert(code->length > 0);
	/* If the number literal does not fit reallocate */
	if (grow_bytecode(code, sizeof(double)) != 0) {return 1;}

	/* Cast the number into a byte sequence */
	uint8_t *array = (uint8_t *)&number;

	/* Write the bytes. */
	for (size_t i = 0; i < sizeof(double) / sizeof(uint8_t); ++i) {
		code->code[code->length++] = array[i];
	}

	return 0;
}

static int grow_bytecode(VMCode *code, size_t by) {
	#define GROW_BY 64 /**< By how many bytes to grow the sequence. */

	if (code->length + by > code->capacity) {
		uint8_t *new_code = realloc(code->code, (code->capacity + GROW_BY) * sizeof(uint8_t));
		if (new_code == NULL) {
			fprintf(stderr, "Memory error: could no grow bytecode sequence.\n");
			return 1;
		}
		code->code      = new_code;
		code->capacity += GROW_BY;
	}

	return 0;
	#undef GROW_BY
}

