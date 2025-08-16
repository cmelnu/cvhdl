Modules
=======

This project is organized into several source files, each responsible for a key part of the C-to-VHDL compilation process:

.. contents:: Source Code Overview
	:depth: 2
	:local:

compi.c
------
The main entry point for the compiler. Handles command-line arguments, file I/O, and orchestrates the parsing and code generation process. It calls the parser to build the AST and then invokes the VHDL generator.

parse.c
-------
Implements the parser and AST construction. Responsible for:

- Parsing function declarations, parameters, variable declarations, assignments, return statements, and control flow (`if`, `else if`, `else`).
- Building the AST (Abstract Syntax Tree) for the input C code.
- Handling binary expressions, unary minus, and negative literals/identifiers.
- Supporting nested and chained expressions for assignments and conditions.

parse.h
-------
Defines the AST node types, parser function prototypes, and supporting data structures for parsing.

token.c
-------
Implements the lexical analyzer (tokenizer). Responsible for:

- Breaking the input C code into tokens (keywords, identifiers, numbers, operators, punctuation).
- Skipping whitespace and comments.
- Providing the token stream for the parser.

token.h
-------
Defines token types, token structure, and tokenizer function prototypes.

utils.c
-------
Provides utility functions for string handling, error reporting, and memory management used throughout the compiler.

utils.h
-------
Declares utility function prototypes and shared constants.

Example usage and integration of these modules can be found in `compi.c` and the test files in `examples/`.

This section describes the main modules of the C-to-VHDL compiler.

compi.c
-------
- Entry point for the compiler.
- Handles command-line arguments, file I/O, and orchestrates parsing and code generation.

parse.c / parse.h
-----------------
- Implements the recursive descent parser.
- Builds the Abstract Syntax Tree (AST) from C source code.
- Supports function declarations, parameters, variable declarations, assignments, and return statements.

utils.c / utils.h
-----------------
- Utility functions for type mapping (C types to VHDL types), AST printing, and error handling.

examples/
---------
- Contains example C files for testing parsing and code generation.

CMakeLists.txt
--------------
- Build configuration for compiling all source files.

