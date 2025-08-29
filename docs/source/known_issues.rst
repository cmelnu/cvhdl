Known Issues
============

Last reviewed: |today|

- Signal name collision for 'result' in generated VHDL output ports.
- Global variable declarations are not handled.
- VHDL codegen does not optimize for hardware resources or timing.
- Short-circuit evaluation semantics (&& / ||) are not modeled exactly as in C (pure combinational evaluation used).
- No function call parsing / inlining yet.
- Limited test coverage for full parser end-to-end pathways (unit-level focus so far).