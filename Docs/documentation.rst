.. This file is written in reStructuredText markup syntax.


################################
Newton's method implemented in C
################################

This project is an implementation of Newton's method in the C programming
language using only the C standard library. It features a complete compiler and
a virtual machine for interpreting the user's input.

.. contents::



Overview and goals
##################

The program is a command-line application that takes a function string and a
starting value from the user as input and returns the approximated root as the
output. The string has to be analysed lexically first to produce tokens, then
those tokens are used to construct a syntax tree that represents the function
in a form that can be understood by the computer. The tree can then be
optimised, the function derived and we can operate on it. From there we can
then compile the tree into a bytecode and run the code on a virtual machine. We
will discuss all these steps in detail later.

The program is a batch-program, meaning it takes in some input and then returns
the output, the user has no control over it during execution. All instructions
must be passed as command-line parameters. However, the design of the program
makes it easy to take parts of it out of context and use them in an interactive
program.



Structure of the project
************************

The project is structured as a tree of directories. Here is a rough overview::

    Newton (root directory)
    |
    |- makefile
    |
    |- Docs (documentation)
    |  |- ...
    |
    |- Source (source code)
       |- makefile
       |- newton.c
       |- newton.h
       |- module_1
       |  |- ...
       |
       |- module_2
       |  |- ...
       |
       |- module_3
          |- ...

The modules have proper names, but they are not relevant for now. The real
source code is in the *Source* directory, there you will find the main source
file with the main function and a header file with global constant definitions.
The other sub-directories contain the modules.



Modules
=======

A module is a standalone library that encapsulates a very specialised part of
the program.  Some modules depend on other modules, but for the most part every
module can be looked at individually. Of course a module on its own is entirely
useless, they are just tools that need to be used by the program through the
main function. Over the course of this document we will examine every module in
detail.



Building the project
********************

The root directory has a makefile, running the standard target will produce a
complete binary in the root directory. There is also a makefile in the *Source*
directory which should be used during development, because it does not clean up
the object files.

The project has no dependencies other than a fully C11-standard compliant
compiler with support for variable-sized arrays. I use clang from the LLVM_
project, but any compiler should work.




Source code review
##################

In this chapter we will examine how the implementation works. There will be
less code than prose, since we will discuss the ideas rather than the concrete
implementation. For the exact code refer to the source files.



Newton's method
***************

Given a function :math:`f: \mathbb{R} \rightarrow \mathbb{R}`, its derivative
:math:`f'` and a start value :math:`x_0 \in \mathbb{R}` we can approximate a
root of the function via the following recursion:

.. math::
    x_{n+1} = x_n - \frac{f(x_n)}{f'(x_n)}

Newton's method does not terminate, instead we will accept an answer if the
discrepancy between iterations becomes small enough. Note that the method is not
guaranteed to find a root, but for the sake of simplicity we will ignore this,
we are only interested in implementing it, not improving it.

This gives use the following list of steps to carry out:

1. Read the input string from the user.
2. Analyse if the string is even valid format, i.e. ``sin`` is something that
   does have a meaning, but ``abc`` does not. We will call these building
   blocks *tokens*.
3. Analyse if the token sequence makes syntactic sense, i.e. ``5 + 4`` does,
   but ``5 + * 4`` does not.
4. Arrange the tokes into a syntax tree of syntax nodes.
5. Derive the function tree.
6. Run Newton's method using the function tree and derivative tree.
7. Print the result and terminate the program.

In reality there is more between steps 5 and 6, but the above steps are
sufficient to produce a correctly working program. We will discuss the extra
steps when we get to the virtual machine.



Syntax nodes and the syntax tree
********************************

Mathematical expressions can be written as syntax trees where every node is an
*operator* and its children are its *operands*. Leaf nodes are either numbers,
constants or variables, i.e. operators with zero arity. Syntax trees are
unambiguous and therefore don't require parentheses.

Let's look at an example::

    5 * sin(2.5 + x * pi/2)
                                |-[2]
                          |-[/]-|
                          |     |-[pi]
                    |-[*]-|
                    |     |-[x]
        |-[sin]-[+]-|
    [*]-|           |-[2.5]
        |-[5]

This tree can be optimised by condensing all sub-trees without any variables
into a single number node.::

    5 * sin(2.5 + x * pi/2)
                          |-[pi/2]
                          |
                    |-[*]-|
                    |     |-[x]
        |-[sin]-[+]-|
    [*]-|           |-[2.5]
        |-[5]

To operate on a tree start evaluating the root node, and recursively evaluate
all children.



Syntax nodes in code
====================

A syntax node is a structure holding the node's operator, numeric value and
pointers to child nodes. Not all of these are used by all nodes: only number
nodes need a numeric value and not all nodes have the maximum number of
children. This makes all nodes bloated, but it does not matter because we will
compile the tree to bytecode anyway.

+--------------------------------------------------+
|              struct syntax_node                  |
+================+=================================+
| operator_value | enum operator_type              |
+----------------+---------------------------------+
| numeric_value  | double                          |
+----------------+---------------------------------+
| arity          | unsigned int                    |
+----------------+---------------------------------+
| operand        | struct syntax_node[MAX_ARITY] * |
+----------------+---------------------------------+



Operating on syntax nodes
=========================

To operate on a node look up the operation function that belongs to the node's
operator.  Each of the operating functions returns a double-precision number and
takes in the pointer to the node we operate on as the argument. The operation
function performs the arithmetic operation on the operation results of the
node's operands, so the whole procedure travels recursively down the tree. Here
is the function for addition

.. code:: cpp

    static double add(struct syntax_node *node) {
        double lhs = syntax_node_operate(node->operand[0]);
        double rhs = syntax_node_operate(node->operand[1]);
        return lhs + rhs;
    }

To operate on the entire tree start with the root.



Deriving a syntax tree
======================

Much like operating, deriving is done recursively by starting out with the root
of the tree. The difference now is that instead of producing numbers we are
creating whole new syntax nodes and assembling them into a tree.


For the most part the rules are exactly the same as you learned them at school,
but taking into account the possibility that the operands themselves might have
to be derived. For example, you might have learned in school that

.. math::
    \frac{\text{d}}{\text{d} x} (\ln x) = \frac{1}{x}

which is correct, and you might feel compelled to write the derivation function
like this

.. code:: cpp

    static struct syntax_node *derive_ln(const struct syntax_node * const node) {
        struct syntax_node *new_node = syntax_node_construct(DIVIDE_OP, 0.0);
        new_node->operand[0] = syntax_node_construct(NUMBER_OP, 1.0);
        new_node->operand[1] = syntax_node_copy(node->operand[0]);
        return new_node;
    }

This is not enough however. We must take into account the possibility that the
operand of ``ln`` is not just a variable but an entire function on its own.
According to the chain rule the complete formula is

.. math::
    \frac{\text{d}}{\text{d} x} \left( \ln (f(x)) \right)
    = \left( \frac{\text{d}}{\text{d} x} f(x) \right) \frac{1}{f(x)}

which results in the following code

.. code:: cpp

    while (iteration_counter < MAX_ITERATIONS && fabs(result) >= EPSILON)

    static struct syntax_node *derive_ln(const struct syntax_node * const node) {
        struct syntax_node *new_node = syntax_node_construct(TIMES_OP, 0.0);
        new_node->operand[0] = syntax_node_derive(node->operand[0]);
        new_node->operand[1] = syntax_node_construct(DIVIDE_OP, 0.0);

        new_node->operand[1]->operand[0] = syntax_node_construct(NUMBER_OP, 1.0);
        new_node->operand[1]->operand[1] = syntax_node_copy(node->operand[0]);

        return new_node;
    }

The same reasoning applies to all other operators. Numbers and constants are
always derived to 0 and variables to 1.



Deriving exponents
==================

Here is where it gets murky. Let's first take a look at an alternate way of
expressing as exponent:

.. math::
    a^b = \exp\big(\ln(a^b)\big) = \exp\big(b \ln(a)\big)

Here is the problem: *ln* is undefined for arguments less or equal to zero.
Let's focus on expressions with positive basis for now. The derivative is

.. math::
    (a^b)' = \left(\exp\big(b \ln(a)\big)\right)'
           = \exp\big(b \ln(a)\big) \left( b' \ln(a) + b a' \frac{1}{a} \right)
           = a^b \left( b' \ln(a) + b a' \frac{1}{a} \right)

This is the general derivative for any *a* and *b*. Let's now assume that *b* is
a constant number and *a* equal to *cx* for a constant *c*. then the derivative
of *b* is *0* and it follows

.. math::
      \left( b' \ln(a) + b a' \frac{1}{a} \right) a^b
    = \left( b a' \frac{1}{a} \right) a^b = b (cx)' \frac{1}{cx} (cx)^b
    = b c \frac{1}{cx} (cx)^b = b c^b x^{b-1}

which is exactly the derivation rule

.. math::
    (x^n)' = n x^{n-1}

you know from school. This way we can avoid the logarithm of a negative
argument, as long as the exponent is a constant number, so we should check for
this special case in our code and replace nodes if necessary. This still does
not solve all problems though, only integer exponents are legal, fractional
exponents mean root operations, which are undefined for negative operands
anyway. We will leave this part to the user though, it's the user'
responsibility to enter reasonable functions.

Complex numbers don't suffer from this limitation, but our implementation of
Newton's method is intended for real numbers only.

The size of this derivation rule and the special case we need to accommodate for
make this the largest rule of all, but it is still preferable over a series of
if-else instructions. We can easily decide if a node is a constant the same way
we contract nodes; simply checking if it is a number node is not enough, we have
to go through the entire sub-tree and see if we can find any variable nodes.



Lexing and parsing
******************

Lexical analysis (*lexing*) and parsing are generally two different steps, but
in this example they are working in tandem. The lexer initiates the parser and
whenever it produces a token it is passed immediately to the parsed to build the
tree. This has the advantage that we don't need to first store the tokens
somewhere and then process it. For the sake of clarity we will ignore this tight
coupling for now.

The task of the lexer is to look at the characters that form the string one at a
time and recognise what belongs together to form a syntax unit (a *token*).
Take the following string::

    5 * sin(2.5 + x * pi/2)

The character sequences ``5``, ``*``, ``sin``, ``(``, ``2.5``, ``+``, ``x``,
``*``, ``pi``, ``/``, ``2`` and ``)`` form tokens. And yes, some of these
sequences are only one character long, but that is irrelevant.

How do we find out where to split the string? The solution is a simplified
Turing machine. The machine will be read-only and it will only be moving to the
right, it will never move to the left or stand still.

The lexer structure has the following members:

+--------------------------------------------------+
|                struct lexer                      |
+=================+================================+
| parser          | struct parser *                |
+-----------------+--------------------------------+
| string_to_parse | const char *                   |
+-----------------+--------------------------------+
| read_head       | const char *                   |
+-----------------+--------------------------------+
| current_state   | enum lexer_state               |
+-----------------+--------------------------------+
| previous_state  | enum lexer_state               |
+-----------------+--------------------------------+
| number_buffer   | double                         |
+-----------------+--------------------------------+
| power           | double                         |
+-----------------+--------------------------------+
| string_buf      | char[STRING_BUFFER_LENGTH + 1] |
+-----------------+--------------------------------+
| string_ptr      | char *                         |
+-----------------+--------------------------------+
| symb            | char                           |
+-----------------+--------------------------------+
| previous_node   | struct syntax_node *           |
+-----------------+--------------------------------+

The parser is owned by the lexer because the two work together hand in hand.
There are two pointers to the input, one is the pointer to the head to the
string to parse, and the other is the current position of the read head of the
machine. We also keep track of the current state and the previous state, we
need the latter to make decisions for implicit operators. The rest are buffers
for collecint numbers and characters of multi-character operators like numbers
and functions.



Implementing a Turing machine
=============================

The function string will serve as our input tape and a pointer to ``char`` will
be the machine's read-head. We don't need to write anything to the tape and we
are only concerned with the next character, not the previous one. The purpose of
our machine is to verify that all sub-sequences represent sensible instructions;
if everything checks out the machine will terminate in the success state,
otherwise it will enter the error state. Each time the machine transitions
states, even if it transitions to the same state, an action can be performed.
This action is a side effect that makes our machine produce something useful:
syntax nodes.

In code the lexer is a structure that holds a reference to the input string, a
pointer to the current character on the string, the value of the previous state,
and a few buffers. The number buffer is used to collect individual digits into a
number. The string buffer concatenates letters to identify functions.

During every iteration we read the current character and depending on what kind
it is (letter, digit, decimal point, symbol) we transition to another state and
perform an action. Some actions, in particular those after reading a different
type of character, produce tokens as a side effect, these tokens are then passed
to the parser.

Once the last character of the string has been reached and if the machine's
final state is the accepting state the lexer can safely terminate, freeing all
its pointers. If at any point the lexer enters the error state we have a format
error. Such an string might look like this:

.. code::

    5 * sun(2..5 + x * pi?2)

There are several format errors here: the string ``sun`` is not a known function
the decimal point is followed by a non-digit, and the question mark is not a
know operator.

The lexer is fairly tolerant when it comes to whitespace, a whitespace
character terminates a function or number, but they are not required when it is
clear where one ends and the other begins. The following variants are both
legal:

.. code::

    5*sin(2.5+x*pi/2)
    5 * sin ( 2.5 + x * pi / 2 )



Lexer states
============

Here is a list of machine states and their description:

ST_START
   This is the starting state of the machine, and also the state after a
   whitespace character has been read.
ST_ERROR
   If the sequence read cannot be associated with any known instruction we end
   up here.
ST_ACCEPT
   After the machine has read the terminating `NUL` character and is not in
   the error state it will transition to this state.
ST_LETTER
   Reading a letter will bring us to this state and as long as we remain here
   we concatenate characters into a string, which will then be compared to
   existing functions once we transition to another state
ST_NUMBER
   As long as we are reading digits they get accumulated.
ST_DECIMAL_POINT
   If we read a decimal point we must transition to this state.
ST_DECIMAL_NUM
   Reading a digit after a decimal point leads to a decimal number.
ST_SYMBOL
    When we read a symbol we are in the symbol state. Symbols are different
    from letters because it is legal to write an expression like `5 +sin(0)`
    and expect the parser to know that the plus does not belong to the sinus
    and the parenthesis not to the zero.

All of these are enumeration items. The reason why numbers and decimal numbers
are different state is that it is legal for a number to have a decimal point
appended, but illegal to append a decimal point to a decimal number, there may
only be one point.



Transitioning between states
============================

A transition instruction consists of the next state and a function pointer to
the action to perform. We can use a structure to express this:

+--------------------------------------+
| struct parser_transition_instruction |
+===============+======================+
| next_state    |  enum parser_state   |
+---------------+----------------------+
| action        |  \*void(void)        |
+---------------+----------------------+

We also need to categorise the various input characters into a small set of
character classes:

:LETTER:   Class for letter a - z and A - Z.
:DIGIT:    Class for digits 0 - 9.
:DECIMAL:  Class for the *full stop* character.
:SYMBOL:   Class for non-alphanumeric characters.
:SPACE:    Class for whitespace characters.
:UNKNOWN:  Class for unknown characters.

These simply let us tell what type of character we are reading. With that said
it's time to set up the instructions, we take in the current state and the type
of character and return a transition instruction. We use the array approach
again, but I will display it as a table here for easier reading.

First the state transition. The row is the current state and the column is the
class of character read.

+------------+--------+------------+------------+--------+-------+---------+
|            | Letter | Digit      | Decimal    | Symbol | Space | Unknown |
+------------+--------+------------+------------+--------+-------+---------+
| Letter     | Letter | Error      | Error      | Symbol | Start | Error   |
+------------+--------+------------+------------+--------+-------+---------+
| Number     | Letter | Number     | |dec_pt|   | Symbol | Start | Error   |
+------------+--------+------------+------------+--------+-------+---------+
| |dec_pt|   | Error  | |dec_no|   | Error      | Error  | Error | Error   |
+------------+--------+------------+------------+--------+-------+---------+
| |dec_no|   | Letter | |dec_no|   | Error      | Symbol | Start | Error   |
+------------+--------+------------+------------+--------+-------+---------+
| Symbol     | Letter | Number     | Error      | Symbol | Start | Error   |
+------------+--------+------------+------------+--------+-------+---------+
| Start      | Letter | Number     | Error      | Symbol | Start | Error   |
+------------+--------+------------+------------+--------+-------+---------+
| Error      | Error  | Error      | Error      | Error  | Error | Error   |
+------------+--------+------------+------------+--------+-------+---------+
| Accept     | Error  | Error      | Error      | Error  | Error | Error   |
+------------+--------+------------+------------+--------+-------+---------+

.. |dec_no| replace:: Decimal number
.. |dec_pt| replace:: Decimal point

Next are the transition actions. An empty field means no action.

+------------+----------+----------+----------+-----------+----------+---------+
|            | Letter   | Digit    | Decimal  | Symbol    | Space    | Unknown |
+------------+----------+----------+----------+-----------+----------+---------+
| Letter     | |app_ch| |          |          | |pass_f|  | |pass_f| |         |
+------------+----------+----------+----------+-----------+----------+---------+
| Number     | |pass_n| | |app_di| | |ini_de| | |pass_n|  | |pass_n| |         |
+------------+----------+----------+----------+-----------+----------+---------+
| |dec_pt|   |          | |app_de| |          |           |          |         |
+------------+----------+----------+----------+-----------+----------+---------+
| |dec_no|   | |pass_n| | |app_de| |          | |pass_n|  | |pass_n| |         |
+------------+----------+----------+----------+-----------+----------+---------+
| Symbol     | |app_ch| | |app_di| |          | |pass_s|  |          |         |
+------------+----------+----------+----------+-----------+----------+---------+
| Start      | |app_ch| | |app_di| |          | |pass_s|  |          |         |
+------------+----------+----------+----------+-----------+----------+---------+
| Error      |          |          |          |           |          |         |
+------------+----------+----------+----------+-----------+----------+---------+
| Accept     |          |          |          |           |          |         |
+------------+----------+----------+----------+-----------+----------+---------+

.. |pass_n| replace:: Pass number
.. |pass_f| replace:: Pass function
.. |pass_s| replace:: Pass symbol
.. |app_ch| replace:: Append character
.. |app_di| replace:: Append digit
.. |app_de| replace:: Append decimal
.. |ini_de| replace:: Initialise decimal

The actions are carried out when transitioning state and the two tables above
are actually one combined table in the code.

Taking the above string ``5 * sin(2.5 + x * pi/2)`` as an example, here is what
the machine looks like initially

.. code::

     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ __
    |5| |*| |s|i|n|(|2|.|5| |+| |x| |*| |p|i|/|2|)|\0|
     ^ - - - - - - - - - - - - - - - - - - - - - - --
     |
     +---+
         |
    +--------------+
    | START        |
    |   string=    |
    |   number=0   <   (<-- token output)
    +--------------+

After one iteration the character ``5`` has been read and the machine is in the
number state and the number buffer contains the number 5.

.. code::

     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ __
    |5| |*| |s|i|n|(|2|.|5| |+| |x| |*| |p|i|/|2|)|\0|
     - ^ - - - - - - - - - - - - - - - - - - - - - --
       |
       +-+
         |
    +--------------+
    | NUMBER       |
    |   string=    |
    |   number=5   <
    +--------------+

A whitespace means that the number is over. So the next state is back in the
start state and the machine puts out a number token.

.. code::

     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ __
    |5| |*| |s|i|n|(|2|.|5| |+| |x| |*| |p|i|/|2|)|\0|
     - - ^ - - - - - - - - - - - - - - - - - - - - --
         |
         |
         |
    +--------------+
    | START        |
    |   string=    |
    |   number=0   <   [5]
    +--------------+

Symbols are handled all the same. Unlike letters symbols cannot be concatenated
into one token, every symbol is a token on its own. The only rule is that a
symbol cannot follow a decimal point.

.. code::

     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ __
    |5| |*| |s|i|n|(|2|.|5| |+| |x| |*| |p|i|/|2|)|\0|
     - - - ^ - - - - - - - - - - - - - - - - - - - --
           |
         +-+
         |
    +--------------+
    | SYMBOL       |
    |   string=    |
    |   number=0   <   [*][5]
    +--------------+

Letter characters prompt the letter state. As long as letters follow letters
they are concatenated into a string.

.. code::

     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ __
    |5| |*| |s|i|n|(|2|.|5| |+| |x| |*| |p|i|/|2|)|\0|
     - - - - - - - ^ - - - - - - - - - - - - - - - --
                   |
         +---------+
         |
    +--------------+
    | LETTER       |
    |   string=sin |
    |   number=0   <   [*][5]
    +--------------+

When transitioning to a state other than letter the string is looked up in a
table and if it matches a known function a token is created. I will skip over
the next three steps.

.. code::

     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ __
    |5| |*| |s|i|n|(|2|.|5| |+| |x| |*| |p|i|/|2|)|\0|
     - - - - - - - - ^ - - - - - - - - - - - - - - --
                     |
         +-----------+
         |
    +--------------+
    | LETTER       |
    |   string=    |
    |   number=0   <   [sin][*][5]
    +--------------+

And so on. Braces are treated like any other operator as well. The lexer does
not verify correct syntax, only correct format.

.. code::

     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ __
    |5| |*| |s|i|n|(|2|.|5| |+| |x| |*| |p|i|/|2|)|\0|
     - - - - - - - - ^ - - - - - - - - - - - - - - --
                     |
         +-----------+
         |
    +--------------+
    | ACCEPT       |
    |   string=    |
    |   number=0   <   [)][2][/][pi][*][x][+][2.5][(][sin][*][5]
    +--------------+

The machine has some special exception rules to also add implicit operators
such as the multiplication in something like ``3x``.



Parsing tokens into a tree
==========================
Parsing uses a variation of the Shunting-Yard algorithm. The regular form of
the algorithm turns an expression from infix-notation to prefix-notation
(Polish notation). We want a syntax tree instead.

The modified algorithm uses two stacks: the regular operator stack and an
operand stack that takes the place of the output queue. Whenever a node would be
placed into the output queue instead push it onto the operand stack. When
popping an operator from the operator stack also pop the appropriate amount of
operands and make them the children of the operator. Then push the operator onto
the operand stack.

+---------------------------------------------+
|               struct parser                 |
+================+============================+
| operand_stack  | struct syntax_node_stack * |
+----------------+----------------------------+
| operator_stack | struct syntax_node_stack * |
+----------------+----------------------------+
| previous_node  | struct syntax_node *       |
+----------------+----------------------------+

By the time all nodes have been processed the operand stack should have only one
node: the root node. All the other nodes will be in the tree. If this is not the
case we have a syntax error. The Shunting-Yard algorithm performs the syntax
checks for us, so if anything goes wrong we have found an error.



Tying lexer and parser together
===============================
A parser instance is just the two stacks, so we can let the lexer control it.
Whenever the lexer produces a token it actually produces a complete node and
performs an iteration of the Shunting-Yard algorithm with it. Once the lexer has
terminated there will be no more new nodes created, so the parser is instructed
to wrap up. Once the parser terminates the lexer terminates as well and returns
the root node.



The problem with syntax trees
*****************************
At this point we are essentially done. We have successfully lexed and parsed the
function string, made sure both format and syntax are correct, we have derived
the function tree and we can operate on it. Technically that's all we need for
Newton's method to work.

However, while technically correct, our trees have practical problems. For one,
the nodes are too large, there is no reason non-number nodes need to store a
numeric value and nodes with less than maximum children don't need all those
pointers.

In fact, the pointers themselves are a massive problem as well, breaking data
locality. The nodes are all over memory and getting a good cache line is near
impossible.

The solution is to discard the tree in favour for something more simple: a
linear sequence of instructions containing only the minimum of necessary
information. A bytecode representation.



Introducing the virtual machine
*******************************
Back in the old days before the good old times there were the bad old times. A
researcher would have to punch the program into a long paper tape, then wind it
into a computer, wait, and hope the program does not have a bug.

We will be using the same approach here: a virtual machine built specifically to
perform arithmetic will read a sequence of bytes and execute those instructions.
The machine is very simple and cannot do any branching or looping, it just
executes a batch job and returns the result. This allows our new flattened tree
to be read in contiguous chunks.

We'll illustrate it with a drawing::

       +--[CARTRIDGE]--+
       |               |
       |  length       |           +----+----+----+----+----+----+   +----+
       |  code---------+--[TAPE]-->|0x05|0x01|0x0a|0x03|0x05|0x01|...|0x0e|
       |               |           +----+----+----+----+----+----+   +----+
       +---------------+
               |             +-[MACHINE]-+
               |             |           |
    |          |         |   | reg_x     |
    |          |         |   | tmp_0     |
    |          |         |   | tmp_1     |
    |          V         |   | stack     |
    +-[insert cartridge]-+---+-----------+

The "cartridge" is the object encapsulating the bytecode, it contains metadata
such as the length of the code sequence and a pointer to said sequence. We could
also add other information to it, for example the syntax version in case we
changed the format in a way that is incompatible with a previous one. In our
case that's not necessary because the code will never leave the program, but if
we wanted to write the code to disc or send it to another program this would be
useful.

The machine is stack-based and reads the code backwards. Any time a number
literal is read it is pushed onto the stack and any time a number is needed it
is popped off the stack. The ``reg_x`` register stores the value of the
:math:`x` variable and the temporary registers store intermediate results before
they get pushed back onto the stack. Before running the machine the :math:`x`
register value is set so the machine can use it.

The reason we read the code backwards is that if we were reading is forwards we
would have to maintain a stack of operators as well as numbers. The function

.. math::
    5 \times \sin \left(2.5 + \frac{\pi}{2} x \right)

is written as

.. math::
    \times 5 \sin + 2.5 \times \div \pi 2 x

in Polish notation. Reading backwards we push three numbers onto the stack, then
we pop two off, divide them and push the result back on the stack. Note that
:math:`2` is above :math:`\pi` on the stack, so if we pop them off in that order
they will be in the right order for division: :math:`\pi \div 2`.

If we were to evaluate the expression forward we would have to push the
:math:`\times` onto the operator stack, then the :math:`5` onto the number
stack, :math:`\sin` and :math:`+` on the operator stack, :math:`2.5` on the
numbr stack and so on, until finally at the very end we could start actually
computing anything.

Note that reading Polish notation strictly backwards is not what is usually
reffered to as *reverse Polish notation* (RPN): an infix expression

.. math::
    a + b

in reverse Polish notation is written as

.. math::
    a b +

whereas the Polish notation is

.. math::
    + a b

and read backwards as

.. math::
    b a +

meaning the order of operands in backwards Polish notation is flipped in
comparison to reverse Polish notation.



Bytecode and opcodes
********************
Now that we have established what our goal is we need to write the compiler
backend that will produce the finished bytecode from the syntax tree. But first
we must decide on the format.

Since a byte is the smallest unit of memory we will use that as our base unit.
Let's keep things simple and assume that a byte is always eight bits from now
on, it holds true for pretty much any machine these days. That gives us 256
unique codes to work with, more than enough to cover the operators.

The next question is how to store number literals. This is where things get
ugly: The standard does not specify how exactly floating point numbers work,
only what it should support. So how do we store numbers in bytecode? One
solution would be to just to with whatever the compiler picks and dump the
memory as is into the bytcode. The other option is to decide upon a particular
implementation and then when writing a number convert it to the representation
and when reading convert it back. I chose the former because it is simpler and
the code will never leave the program.

To write a number the memory of the number is copied 1:1 to the end of the
array. Then the opcode for number literals is appended. We write the opcode
after the number because the code will be read backwards.



Opcodes
=======
A computer can only understand numbers, and our virtual machine is no different.
That's why every instruction is mapped to a number, and since we have chosen one
byte for every instruction that gives us up to 256 individual opcodes. We will
only use some of these, so the rest is considered and error.

Opcodes are simply defined using an enumeration and written as unsigned 8-bit
integers, or ``uint8_t`` types in C.

You might be noticing a problem now: we have too many codes, which is a waste of
memory. Furthermore, not all operators are equally common, so why do they
require the same amount of memory? It would be better if frequent opcodes used
less information to encode than infrequent ones. We could compress the code
using for example Huffman compression and store the Huffman tree as metadata in
the "cartridge". We won't be doing this though, it would not be worth the effort
for our limited scope.



Compiling the tree
******************
We know what our bytecode should look like, so we are ready to compile it.
Compilation is done recursively: start at a node and write its opcode, the
compile every child in order from first to last.

The result is the expression in Polish notation in bytecode. To compile the
entire tree start at the root node, and the recurring rule becomes a depth-first
search over the entire tree. Take the following tree as an example::

    []
                                |-[2]
                          |-[/]-|
                          |     |-[pi]
                    |-[*]-|
                    |     |-[x]
        |-[sin]-[+]-|
    [*]-|           |-[2.5]
        |-[5]

Starting at the root we write its opcode, then compile the children, and the
first child is a number literal, a leaf node, so it is compiled directly to
bytecode and we are done with it. We move on to the second child::

    [*][5]
                               |-[2]
                         |-[/]-|
                         |     |-[pi]
                   |-[*]-|
                   |     |-[x]
       |-[sin]-[+]-|
    []-|           |-[2.5]
       |-[]

We write its opcode, but since this child has children of its own we must
compile those as well::

    [*][5][sin]
                            |-[2]
                      |-[/]-|
                      |     |-[pi]
                |-[*]-|
                |     |-[x]
       |-[]-[+]-|
    []-|        |-[2.5]
       |-[]

Observe how we are now in a similar situation as when we started out at the
root. We can continue with the depth-first search and eventually the entire tree
will be compiled.::

    [*][5][sin][+][2.5][*][x][/][pi][2]

                         |-[]
                    |-[]-|
                    |    |-[]
               |-[]-|
               |    |-[]
       |-[]-[]-|
    []-|       |-[]
       |-[]

Now the entire tree has been flattened into a nice continuous array. In fact, we
could have skipped the tree entirely because the Shunting-Yard algorithm in its
original form does exactly produce an expression in Polish notation. The reason
why we used a tree anyway is that it is easier to analyse a tree than an array.
In a tree we can directly jump to a child via pointer, while in an array we have
to count indices. Add on top of that the fact that not all nodes have the same
number of children and it becomes impossible to predict where in the array a
child is. We would have to store with every opcode where its child is or more
from node to node. And that that point we are back to running back and forth in
memory, which is exactly what we were trying to avoid in the first place.



Virtual machine
***************
The virtual machine is very simple: it has a stack that can hold an unspecified
number of variables, a register for the :math:`x` variable, two temporary
registers and the code object.  First the code object, i.e. the "cartridge", is
loaded into the machine, the register set and then the machine can start
working.

Starting at the tail of the code sequence we work our way backwards. One way to
implement the machine would be to emulate assembly code using temporary register
variables

.. code::

    POP  tmp_0       ; pop number from stack into register tmp_0
    POP  tmp_1       ; pop number from stack into register tmp_1
    ADD  tmp_0 tmp_1 ; add tmp_1 to tmp_0
    PUSH tmp_0       ; push tmp_0 back to the stack

Since we are in C we can write it a bit nicer using preprocessor macros

.. code::

    double tmp[MAX_ARITY]; /**< Temporary registers.                         */
    int    i = 0;          /**< Which temporary register is the current one. */

    /** Pop a number off this function's stack. */
    #define POP  number_stack_pop(stack, &tmp[i++]);

    /** Push a number to function's stack. */
    #define PUSH(value)  number_stack_push(stack, value); i = 0;

    POP
    POP
    PUSH(tmp[0] + tmp[1])

We have as many temporary registers as the maximum arity demands and with every
consecutive pop we store the number in the next register. When we eventually
push the result we reset the variable ``i`` back to 0 so the next set of pops
starts with the first register again. There is no error handling above for
easier reading, but the real code does handle errors.

The size of the stack grows and shrinks dynamically during computation. It is
impossible to predict how large the stack needs to be, so we have to either set
a fixed limit or keep going as far as the physical machine will let us. I
decided to choose the latter, as there is no practical reason not to do so, on
today's home computers you have enough memory to process expressions you
wouldn't even be able to wrap your head around.

All that is left is a large ``switch`` block that will pick the operation for
every opcode and abort with an error if the opcode is undefined. Once the code
sequence has been processed the last value on the stack is copied to the register
so it can be read.

Every operation pops a number of values from the stack that is equal to its
arity, e.g. two for addition, one for sinus and zero for constants, and always
pushes one value.



Newton's method, again
**********************
Finally we can actually implement Newton's method now that we have all the
tools we need. First the function string is parsed into a syntax tree. This
tree is then derived into the derivative tree. Both trees are then compiled
into two bytecode objects. We also create a virtual machine.

To execute the method we iterate until the result is close enough to the root.
The exit criterion will the that either the result is close enough to the root
or that we have already performed a certain maximum number of iterations.

.. code:: cpp

    while (iteration_counter < MAX_ITERATIONS && fabs(result) >= EPSILON)

Without a limit on the number of iterations the algorithm could get stuck in an
endless loop because Newton's method does not always terminate. Here is the
complete loop, rewritten slightly

.. code:: cpp

    double x_n, f_xn, d_xn; /* x_n, f(x_n) and f'(x_n) */
    do {
        machine_load_code(*machine, function);
        if((exit_status = machine_execute(machine, &f_xn)) != 0) {goto end;}

        if (fabs(f_xn) < EPSILON) {goto end;}

        machine_load_code(*machine, derivative);
        if((exit_status = machine_execute(machine, &d_xn)) != 0) {goto end;}

        x_n = x_n - f_xn / d_xn;

        ++iterations;
    } while (iterations < MAX_ITERATIONS)
    exit_status = -1;

The error check when running the machine is necessary because the machine might
choke on an undefined bytecode. Technically running the check only on the first
run would be enough, but it doesn't hurt either.

Note how at the end of the loop we set the exit status to -1, that's because if
the loop runs until the end we haven't found a good enough solution, so the
function should return an error, not success. If the algorithm was successful
it would have skipped right to the end, over this line.




Conclusion and closing thoughts
###############################



History of *Newton's method implemented in C*
*********************************************
The idea to write an implementation of Newton's method in C came to me during
the summer of 2014. I had just finished reading *The C Programming Language* by
Kernighan  and Ritchie, and was looking for an idea for a simple program to
write as my first exercise. Newton's method looked simple enough, have three
variables, run a simple algorithm in a loop and print the result.

The original idea was to only support polynomial functions. The user would be
prompted to enter the degree and then the program would prompt the user again
for that many coefficients. This would be very easy to derive as well.

.. code::

    $ Please enter the grade of your function
    > 3
    $ Please enter coefficient 1
    > 5
    $ Please enter coefficient 2
    > -2
    $ Please enter coefficient 3
    > 0
    $ Please enter constant
    > 7
    $ Your function is 5x^3 + -2x^2 + 0x^1 + 7

This idea was quickly scrapped before I even wrote a single line of code, it
seemed lame. Wouldn't it be much better if the user could enter *any* function
as an argument? The program would be able to understand most common operators
and functions.

Going over this in my head I came up with layer upon layer of if-else-if blocks
and I quickly realised that it would turn into an utter mess. Remembering the
classes I took I realised that a Turing machine would be ideal for the job: a
Turing machine decides whether a problem is solvable, and in our case we want to
know if a string is meaningful.

This solved to question of how to turn naked characters into meaningful units
like numbers, operators and functions. It seems obvious that these units should
be arranged in a tree structure, so the next question was how to produce such a
tree. I found the Shunting-Yard algorithm and after some minor modification it
was able to produce syntax trees instead of Polish notation.

That's the point where I considered the problem solved. I had successfully lexed
a text string into tokens and parsed those tokens into a tree. I was then able
to derive that tree, optimise it and operate on it. What I did not realise at
that time was that I had effectively written a compiler frontend. In fact, I
didn't even know the proper terms, so in this original version things were named
wrong all the time. Sure, I knew that *parsing* is a word and that *tokens* are
a thing, but I had no idea what exactly those words meant.

Still, I had achieved what I had set out to do and I moved on. The program was
left as it was on my hard drive for over half a year. I never published the
code, I considered it a throwaway exercise and it was full of bad practices
like global variables, no error checking and of source wrong names.

During that time I learned more about programming and computers. In particular I
learned about cache locality, how the computer prefetches memory for faster
access and how things slow down if the computer makes a wrong guess. Cache
locality is impossible to achieve with a tree that consists of nodes and
pointers.

I also learned about the basics of compilers, I learned that my program was a
compiler frontend and I learned about compiler backends. While my knowledge is
nowhere near enough for writing a compiler for a real programming language, it
was enough to come up with a sort of bytecode for mathematical expressions and
a virtual machine to go along with it.

So I decided to re-visit my project, I got rid of the global variables,
refactored types and renamed everything until the project was at least in a
presentable state. I then went on to design and write the compiler and the
virtual machine.

I think the end result is quite presentable and so I have decided to write this
documentation, both for myself to challenge my own design, as well as for others
people to learn from it. I believe that you have only understood a topic if you
are also able to explain it to other people. So I hope you find reading this
program as informative as I found writing it.



Where to go from here
*********************
The program could still use some improvements:

- Functions currently only support one argument, in order to support more we
  need an argument separator symbol, usually a comma or semicolon. We need to
  be able to recognise that symbol in the lexer and handle it in the parser.
  The former is easy and the latter is already a solved problem in the
  Shunting-Yard algorithm.

- Variadic functions are functions that can take any number of arguments, for
  example a sum function that sums an arbitrary number of arguments. This would
  require deeper changes since there is no maximum arity anymore.

- More user options, such as user-specified precision and minimum number of
  iterations.

- Writing the compiled bytecode to disc so other programs could use it. The
  current format is not sufficient, because the way that floating point numbers
  are stored is implementation-dependent. There needs to be some standard.

- Control over optimisation of trees; currently the tree is always fully
  optimised, possibility loosing information about the original string in the
  process. A user might not want that.

- The parser does not need to be given entire nodes, just the operator (token)
  is enough. The parser can then construct the node when pushing the operator
  on the operand stack. However, the operand stack still needs to be a stack of
  syntax nodes, so we would need two kinds of stacks instead of one as it is
  now.

- More than one variable. Newton's method can be adapted to also handle
  functions :math:`f: \mathbb{R}^n \rightarrow \mathbb{R}^n`, but that is beyond
  the scope of this project. Since the number of variables is arbitrary it poses
  some heavy design decisions: how many variables are permitted, what are legal
  names, are they hardcoded, and if not how do you tell the program which
  substrings represent variables?

- There are two types of error: errors from user input, like trying to divide
  by zero or wrong syntax, and errors inherent to the program, like producing a
  stack overflow. The former needs to be handled as an exception, catch the
  error gracefully and report to the user where the problem lies. The latter is
  a bug and needs to be detected with an assertion and terminates the program
  immediately, reporting where the bug lies.

- Functions currently only return either 0 on success or 1 on failure. This is
  good for knowing that an error occurred, but not what kind of error. We need
  proper exit codes instead and every module should define its own set.


.. _LLVM: http://llvm.org

