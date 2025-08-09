Known Issues
============

Last reviewed: |today|

- Signal name collision for 'result' in generated VHDL output ports.
- Limited expression parsing: only identifiers and numbers are supported (no binary operators).
- No support for control flow statements (if, while, for) yet.
- Global variable declarations are not handled.
- Error messages may be minimal for some parsing failures.
- VHDL codegen does not optimize for hardware resources or timing.