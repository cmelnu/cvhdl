Modules
=======

This project is organized into several source files, each responsible for a key part of the C-to-VHDL compilation process:

.. contents:: Source Code Overview
    :depth: 2
    :local:

compi.c
-------
The main entry point for the compiler. Handles command-line arguments, file I/O, and orchestrates the parsing and code generation process. It calls the parser to build the AST and then invokes the VHDL generator.

astnode.c / astnode.h
---------------------
Defines and implements the AST node structure and management functions.

- Provides the data structures for AST nodes used throughout the compiler.
- Functions for creating, freeing, and manipulating AST nodes.
- Used by the parser and code generator to represent the program structure.

parse.c / parse.h
-----------------
Implements the parser and AST construction.

- Parsing function declarations, parameters, variable declarations, assignments, return statements, and control flow (`if`, `else if`, `else`, `while`, `break`, `continue`).
- Building the AST (Abstract Syntax Tree) for the input C code.
- Handling binary expressions, operator precedence, parentheses, unary minus, and negative literals/identifiers.
- Supporting nested and chained expressions for assignments and conditions.
- Supporting nested while loops and correct handling of break/continue at any loop depth.
- Printing improved error messages with the exact line number of the source file where parsing errors occur.
- Declares AST node types, parser function prototypes, and supporting data structures for parsing, including support for control flow and arrays.

token.c / token.h
-----------------
Implements the lexical analyzer (tokenizer).

- Breaking the input C code into tokens (keywords, identifiers, numbers, operators, punctuation).
- Skipping whitespace and comments.
- Tracking the line number of each token for improved error diagnostics.
- Providing the token stream for the parser.
- Recognizing keywords for control flow (`while`, `break`, `continue`, etc.).
- Defines token types, token structure, and tokenizer function prototypes.

utils.c / utils.h
-----------------
Provides utility functions for string handling, error reporting, memory management, type mapping (C types to VHDL types), AST printing, and error handling used throughout the compiler.

examples/
---------
Contains example C files for testing parsing and code generation, including while loop and nested loop examples.

CMakeLists.txt
--------------
Build configuration for compiling all source files. Supports enabling developer debug output via `-DDEBUG=ON`.

