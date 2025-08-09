Modules
=======

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

