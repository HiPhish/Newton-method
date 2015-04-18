#ifndef NEWTON_OPCODES_H
#define NEWTON_OPCODES_H

/** Enumeration of opcodes for the virtual machine.
 *
 *  We skip over the number 0x00 because it's easier to debug. The type used
 *  for opcodes should be uint8_t.
 */
enum vm_opcodes {
	OPC_NUM   = 0x01, /**< Number literal.       */
	OPC_NEG   = 0x02, /**< Negation.             */
	OPC_ADD   = 0x03, /**< Addition.             */
	OPC_SUB   = 0x04, /**< Subtraction.          */
	OPC_MULT  = 0x05, /**< Multiplication.       */
	OPC_DIV   = 0x06, /**< Division.             */
	OPC_POW   = 0x07, /**< Power.                */
	OPC_EXP   = 0x08, /**< Exponential function. */
	OPC_LN    = 0x09, /**< Natural logarithm.    */
	OPC_SIN   = 0x0a, /**< Sine.                 */
	OPC_COS   = 0x0b, /**< Cosine.               */
	OPC_TAN   = 0x0c, /**< Tangent.              */
	OPC_VAR_X = 0x0d, /**< Variable X.           */
	OPC_PI    = 0x0e, /**< Pi constant.          */
	OPC_E     = 0x0f, /**< E constant.           */
};

#endif /* NEWTON_OPCODES_H */

