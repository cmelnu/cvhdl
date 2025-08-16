# 🚀 Compi: C-to-VHDL Compiler

Compi is a tool that translates C source code into VHDL hardware descriptions.  
Easily convert your algorithms from software to hardware! 🖥️➡️🔌

## ✨ Features

- 📝 Lexical analysis of C code (tokenizer)
- 🏗️ Parsing of function declarations, parameter lists, variable declarations, assignments, return statements, and `if`/`else if`/`else` control flow
- 🌳 Abstract Syntax Tree (AST) construction with visualization
- 🛠️ Generation of VHDL entities and architecture skeletons
- 🔄 Automatic type mapping between C types (`int`, `float`, `double`, `char`) and VHDL types
- 🗂️ Example C files for testing in `examples/`
- 📚 Sphinx documentation in `docs/` with Read the Docs theme

## 🛠️ Installation

```bash
git clone https://github.com/cmelnu/compi.git
cd compi
mkdir build
cd build
cmake ..
make
```

## ▶️ Usage

```bash
./compi input.c output.vhdl
```

### Debugging

Print the AST for debugging:
```bash
./compi -d input.c output.vhdl
```

## 📖 Documentation

Sphinx documentation is available in the `docs/` folder.  
To build the docs:
```bash
cmake --build . --target docs
"$BROWSER" docs/build/html/index.html
```

## 🧹 Cleaning

To remove build artifacts and documentation output:
```bash
cmake --build . --target clean-all
```

## 🗂️ Project Structure

- `src/` — Source code (.c, .h files)
- `examples/` — Example C files for testing
- `docs/` — Sphinx documentation
- `build/` — Build output
- `CMakeLists.txt` — Build configuration
- `.gitignore` — Git ignore rules

## 🚧 Known Issues

- Signal name collision for local variable 'result' in VHDL output ports (should be renamed to 'internal_result')
- Global variables not yet implemented
- Limited expression parsing (only identifiers and literals)
- No support for `while` and `for` control flow statements
- VHDL codegen does not optimize for hardware resources or timing

## 🗺️ Roadmap

Here’s what’s planned next for Compi, based on the current state of `parse.c` and recent development:

### ✅ Current Capabilities
- Function declarations and parameter parsing
- Variable declarations and initializations
- Assignment statements (`x = value;`)
- Return statements with expressions
- AST construction and visualization
- Parsing and code generation for `if`, `else if`, and `else` statements
- Support for unary minus and negative literals/identifiers in expressions
- Basic VHDL code generation with type mapping

### 🚧 Roadmap

1. **Expression Parsing Improvements**
   - Support binary operations (e.g., `a + b`, `x * 2`)
   - Handle operator precedence and parentheses

2. **Control Flow Statements**
   - Parse and represent `while` and `for` statements in the AST
   - Generate VHDL comments or skeletons for control flow

3. **Global Variable Support**
   - Parse and represent global variable declarations
   - Generate VHDL for global signals

4. **Function Calls**
   - Parse function calls within expressions and statements
   - Inline or generate VHDL for simple calls

5. **Error Handling & Diagnostics**
   - Improve error messages and diagnostics for unsupported constructs

6. **VHDL Codegen Enhancements**
   - Optimize generated VHDL for hardware resources and timing
   - Handle signal name collisions (e.g., local `result`)

7. **Documentation & Examples**
   - Expand Sphinx documentation with module, function, and data structure details
   - Add more example C files and expected VHDL outputs

---

Contributions and feedback are welcome!  
Join the discussion and development at [GitHub - cmelnu/compi](https://github.com/cmelnu/compi)