Usage
=====

This section explains how to use the C-to-VHDL compiler, including command-line options and example workflows.

Examples
--------

.. code-block:: bash

   ./compi input.c output.vhdl

Error messages include the exact line number in the source file where the error was found, e.g.:

   Error (line 15): Expected ';' after variable declaration

See the `examples/` folder for sample input files.

Developer Debug Output
---------------------

To enable verbose debug output for developers, configure the build with the -DDEBUG=ON argument:

.. code-block:: bash

   cmake -DDEBUG=ON ..
   make

This will provide additional debug prints and diagnostics during parsing and code generation.
