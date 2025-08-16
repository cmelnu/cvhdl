Usage
=====

This section explains how to use the C-to-VHDL compiler, including command-line options and example workflows.

Examples:
---------


.. code-block:: bash

   ./compi input.c output.vhdl
   ./compi -d input.c output.vhdl  # Print AST for debugging

Debug mode (-d) prints a readable AST tree with labeled nodes and branches for easier debugging.

Error messages now include the exact line number in the source file where the error was found, e.g.:

   Error (line 15): Expected ';' after variable declaration

See the `examples/` folder for sample input files.
