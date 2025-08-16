Known Issues
============

Last reviewed: |today|

- Signal name collision for 'result' in generated VHDL output ports.
- No support for control flow statements (`while`, `for`) yet.
- Global variable declarations are not handled.
- VHDL codegen does not optimize for hardware resources or timing.
- Error messages now include the exact line number in the source file where parsing errors occur, making diagnostics easier.