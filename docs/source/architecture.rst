Architecture
============

The compiler is structured as a pipeline:

1. **Lexical Analysis (token.c / token.h)**
    - Reads the input C file and produces a stream of tokens.
    - Handles keywords, identifiers, literals, operators, and comments.

2. **Parsing (parse.c / parse.h)**
    - Consumes the token stream and builds an Abstract Syntax Tree (AST).
    - Supports function declarations, variable declarations, assignments, return statements, and control flow (`if`, `else if`, `else`, `while`, `for`, `break`, `continue`).
    - Handles binary and unary expressions, including negative literals and identifiers.
    - Supports nested while loops, nested for loops, and correct handling of break/continue at any loop depth.

3. **AST Representation (astnode.c / astnode.h)**
    - Defines node types for all supported C constructs.
    - Provides data structures and functions for creating, freeing, and manipulating AST nodes.
    - Supports nested expressions and statement blocks, including nested for and while loops.

4. **VHDL Code Generation (compi.c, parse.c)**
    - Traverses the AST and emits VHDL code.
    - Maps C types to VHDL types, handles signal declarations, assignments, and control flow.
    - Handles negative values and binary expressions correctly in VHDL.
    - Generates VHDL for while loops and for loops, including nested loops, break, and continue statements.

5. **Utilities (utils.c / utils.h)**
    - Provides string manipulation, error handling, memory management, type mapping, and AST printing.

6. **Testing (tests/*.cpp)**
    - GoogleTest unit tests covering AST construction, utilities (precedence, numeric detection), lexer tokenization, and control-flow scaffolding.
    - Each test is discovered individually via ``gtest_discover_tests`` allowing granular execution (e.g. regex filtering) through CTest or direct GoogleTest filters.

The modular design allows for easy extension to new C constructs and VHDL features.

Overview
--------
The compiler is organized into modular components for tokenization, parsing, AST construction, and VHDL code generation.

Main Components
---------------
- **Tokenizer**: Scans C source code and produces tokens for keywords, identifiers, numbers, operators, and punctuation.
- **Parser**: Uses recursive descent to build an AST representing the structure of the C code, including support for nested while loops and control flow.
- **AST**: Abstract Syntax Tree nodes represent functions, variables, assignments, return statements, and control flow constructs.
- **Code Generator**: Traverses the AST to produce VHDL code, mapping C types to VHDL types and handling signal/port declarations, while loops, break, and continue.

Data Flow
---------
1. Input C file is tokenized.
2. Tokens are parsed into an AST.
3. VHDL code is generated from the AST.

Extensibility
-------------
- New C constructs can be supported by adding AST node types and parser logic.
- VHDL codegen can be expanded for more complex hardware descriptions.

Developer Debug Output
----------------------
- Enable verbose debug output for developers by configuring the build with the `-DDEBUG=ON` argument in CMake. This provides additional debug prints and diagnostics during parsing and code generation.

Testing Workflow
----------------

1. Configure with ``-DENABLE_TESTING=ON`` (default).
2. Build and run tests: ``cmake --build build --target test_all``.
3. Inspect or filter tests: ``ctest -N`` / ``ctest -R <regex>``.
4. Use direct filtering: ``./build/compi_tests --gtest_filter=TokenTests.*``.

Planned additions include integration (golden) tests for VHDL output and coverage reporting.

