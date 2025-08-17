.. compi documentation master file, created by
   sphinx-quickstart on Sat Aug  9 14:10:06 2025.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Compi's documentation!
=================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   usage
   examples
   modules
   architecture
   contributing
   known_issues
   license
   
Features
--------

- Lexical analysis of C code (tokenizer)
- Parsing of function declarations, parameter lists, variable declarations, assignments, return statements, and if/else if/else control flow
- Expression parsing with proper precedence and associativity:
    - Arithmetic: + - * /
    - Shifts: << >>
    - Bitwise: & | ^
    - Comparisons: == != < <= > >=
    - Logical: && || and unary !
    - Unary minus: -x
- Abstract Syntax Tree (AST) construction with improved visualization
- Generation of VHDL entities and architecture skeletons
- Automatic type mapping between C types (int, float, double, char) and VHDL types
- Example C files for testing in examples/
- Sphinx documentation in docs/ with Read the Docs theme
- Array support:
   - Parse and generate VHDL for C arrays of types int, float, double, and char
   - Array declaration and initialization: int arr[3] = {1,2,3};, float arr[2] = {1.0, 2.5};, char arr[4] = {'a','b','c','d'};
   - Array access and assignment: arr[i] = x;, y = arr[j];
   - Returning array elements: return arr[i];
   - Full index expressions inside brackets: arr[i+1], arr[(j<<1)+k], return arr[2*i]
   - Type mapping:
      - int[] → std_logic_vector(31 downto 0)
      - float[], double[] → real
      - char[] → character
   - Initializers are converted to valid VHDL literals for each type
   - Array element access and assignment use VHDL syntax: arr(i)
- While loop, break, and continue support
   - Parse and generate VHDL for while (<expr>) { ... } loops
   - Support for break; and continue; statements inside loops, including inside if, else if, and else blocks within loops
   - VHDL codegen emits exit; for break and next; for continue

Debug Build Instructions
-----------------------

To enable verbose debug output for developers, configure the build with the -DDEBUG=ON argument:

.. code-block:: bash

   cmake -DDEBUG=ON ..
   make

This will provide additional debug prints and diagnostics during parsing and code generation.

Roadmap
-------

1. Control Flow Statements
   - Next: Add support for for loops
2. Global Variable Support
   - Parse and represent global variable declarations
   - Generate VHDL for global signals
3. Function Calls
   - Parse function calls within expressions and statements
   - Inline or generate VHDL for simple calls
4. Error Handling & Diagnostics
   - Improve error messages and diagnostics for unsupported constructs
5. VHDL Codegen Enhancements
   - Optimize generated VHDL for hardware resources and timing
   - Handle signal name collisions (e.g., local result)
6. Documentation & Examples
   - Expand Sphinx documentation with module, function, and data structure details
   - Add more example C files and expected VHDL outputs
7. Code Cleanup and Restructuring
   - Ensure clean code, improve maintainability, and refactor as needed

Introduction
------------

Compi is a C-to-VHDL compiler for basic hardware synthesis.

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
