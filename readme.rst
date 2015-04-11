################################
Newton's method implemented in C
################################

This program is an implementation of Newton's method, a numeric method for
computing the root of a function, in the C programming language. It
demonstrates the implementation of a compiler frontend, analysis of the
resulting syntax tree, a compiler backend to generate bytecode and a virtual
machine to execute the code.

The entire project was inteded as a learning exercise for myself and I am
releasing the source code to the public for others to learn from it. The focus
of the project was simplicity and clean code, so don't expect a GUI, complex
operations or any features that would detract from the main focus.

Building
########
A makefile is included with the project, to build navigate to the source folder
and type on of the following into your terminal::

    make
    make release

The *release* target will apply compiler optimisations, while the default target
does not, it is intended for debugging where we don't want the  compiler to alter
our code.

The makefile was written for the clang compiler, if you want to use a different
compiler you have to re-assign the ``CC`` variable::

    make release CC=gcc

To clean up the build directory use the clean target::

    make clean

You can also run the clang static analyser, both of the spellings below are
valid::

    make analyze
    make analyse

There are no dependencies, other than a C11-compliant compiler with support for
variable-sized arrays. You can refer to the makefile for more details if you
wish, it is a very simple makefile.

Runnig
######
The program is a command-line application and all information is passed as
arguments. Here is the syntax::

    newton function guess

Where ``function`` is a string representing the function and ``guess`` is a
string that represents the starting value for the method. There is no need to
specify a derivative function, the program will compute it.

A more verbose way of calling the program is this::

    newton --f function --g guess [--p]

The optional ``--p`` flage makes the program print every step of the iteration
(if you are not familiar with syntax notation, the brackets indicate something
optional, you don't actually type them).

Legal function syntax
*********************
The program is pretty clever about its input and will insert implicit
operators, i.e. ``5x`` will be understood to mean ``5 * x``. Here is an
overview of known symbols and functions:

===========  =============================================
Symbol       Meaning
===========  =============================================
``(`` ``[``  Opening parethesis
``)`` ``]``  Closing parethesis
``x``        Variable x
``+``        Addition
``-``        Negation or Subtraction, depending on context
``*``        Multiplication
``/``        Division
``^``        Power
``exp``      Exponential function
``ln``       Natural logarithm
``sin``      Sine
``cos``      Cosine
``tan``      Tangent
``pi``       Constant number π
``e``        Constant number e
===========  =============================================

Both types of parantheses can be used interchangably, one can even close the
other. It's just an aesthetic difference.

Square roots can be entered using a power via the following rule:

.. math::
   \sqrt[b]{a} = a^{\frac{1}{b}}

Numbers can be entered as either integers or decimal numbers, it does not
matter, they always get converted to double-precision floating point numbers.

Further reading
###############
There is an in-depth documentation on the working of the program in the *Docs*
directory. If you want to read the source code you will find everything
documented using Doxygen comments. To generate a Doxygen documentation use the
doxyfile in the *Docs* directory.

License
#######

The MIT License (MIT)

Copyright (c) 2015 "HiPhish"

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

