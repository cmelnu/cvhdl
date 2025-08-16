Architecture
============

The compiler is structured as a pipeline:

1. **Lexical Analysis (token.c)**
	- Reads the input C file and produces a stream of tokens.
	- Handles keywords, identifiers, literals, operators, and comments.

2. **Parsing (parse.c, parse.h)**
	- Consumes the token stream and builds an Abstract Syntax Tree (AST).
	- Supports function declarations, variable declarations, assignments, return statements, and control flow (`if`, `else if`, `else`).
	- Handles binary and unary expressions, including negative literals and identifiers.

3. **AST Representation (parse.h)**
	- Defines node types for all supported C constructs.
	- Supports nested expressions and statement blocks.

4. **VHDL Code Generation (compi.c, parse.c)**
	- Traverses the AST and emits VHDL code.
	- Maps C types to VHDL types, handles signal declarations, assignments, and control flow.
	- Handles negative values and binary expressions correctly in VHDL.

5. **Utilities (utils.c, utils.h)**
	- Provides string manipulation, error handling, and memory management.

The modular design allows for easy extension to new C constructs and VHDL features.

Overview
--------
The compiler is organized into modular components for tokenization, parsing, AST construction, and VHDL code generation.

Main Components
---------------
- **Tokenizer**: Scans C source code and produces tokens for keywords, identifiers, numbers, operators, and punctuation.
- **Parser**: Uses recursive descent to build an AST representing the structure of the C code.
- **AST**: Abstract Syntax Tree nodes represent functions, variables, assignments, and return statements.
- **Code Generator**: Traverses the AST to produce VHDL code, mapping C types to VHDL types and handling signal/port declarations.

Data Flow
---------
1. Input C file is tokenized.
2. Tokens are parsed into an AST.
3. AST is printed for debugging (optional).
4. VHDL code is generated from the AST.

Extensibility
-------------
- New C constructs can be supported by adding AST node types and parser logic.
- VHDL codegen can be expanded for more complex hardware descriptions.
