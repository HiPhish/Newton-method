#ifndef NEWTON_NEWTON_H
#define NEWTON_NEWTON_H

/** @fine newton.h
 *
 *  Global definitions for the entire program.
 */

/** Maximum length of a function string. */
#define MAX_FUNCTION_LENGTH  3

/** Floating point value of the Greek letter pi.
 *
 *  This value has been taken from `M_PI`, part of the POSIX/UNIX extensions to
 *  C's *math.h* header file; it has been re-declared here for portability.
 */
#define PI  3.14159265358979323846264338327950288

/** Floating point value of Euler's number e.
 *
 *  This value has been taken from `M_E`, part of the POSIX/UNIX extensions to
 *  C's *math.h* header file; it has been re-declared here for portability.
 */
#define E   2.71828182845904523536028747135266250

/** Maximum arity of any operator */
#define MAX_ARITY  2

#endif /* NEWTON_NEWTON_H */

