# Compi: C-to-VHDL Compiler

Minimal C subset → VHDL translator. Focused on simple functions, control flow, expressions, and arrays to explore software → hardware mapping.

## Key Features (High-Level)
* Tokenizer, recursive-descent parser, AST builder
* Expressions with precedence (arith / shifts / bitwise / compare / logical / unary)
* Control flow: `if / else if / else`, `while`, `for`, `break`, `continue`
* Arrays with declarations, initializers, indexed access
* Function call support (entity instantiation, unique result signals)
* Basic VHDL code generation (entity/architecture skeleton + signal mapping)
* Type mapping: `int|float|double|char` → suitable VHDL types
* GoogleTest unit tests (auto-discovered via CTest)
* Sphinx docs

## 🛠️ Installation

```bash
git clone https://github.com/cmelnu/compi.git
cd compi
mkdir build

## Helper Script: build_and_run.sh

To simplify building and running the compiler, use the provided script:

```sh
./build_and_run.sh <input.c> [output.vhdl]
```

This script will:
- Configure and build the project (if needed)
- Run the compiler on your input file
- Output VHDL to the specified file (default: output.vhdl)
- Show a preview of the generated VHDL

Example:
```sh
./build_and_run.sh examples/example.c
```

---

## Building

This project uses CMake. To build manually:

```sh
mkdir -p build
cmake -S . -B build
cmake --build build --target compi -j$(nproc)
```

This will produce the `compi` binary in `build/`.

## Usage

```sh
./build/compi examples/example.c output.vhdl
```

This will parse the input C file and generate VHDL code to `output.vhdl`.

## 🧪 Testing

```bash
cmake -S . -B build -DENABLE_TESTING=ON
cmake --build build --target test_all -j 4
ctest --test-dir build --output-on-failure
```
Shortcuts:
```bash
./run_tests.sh              # build + run tests
./build_docs.sh             # build Sphinx docs
./build/compi_tests --gtest_filter=TokenTests.BasicLexing  # single test
```

## 🗂️ Project Structure

- `src/` — Source code (.c, .h files)
- `examples/` — Example C files for testing
- `docs/` — Sphinx documentation
- `build/` — Build output
- `run_tests.sh` — Helper to configure (if needed), build, and run all tests
- `build_docs.sh` — Helper to configure (if needed) and build Sphinx HTML docs
- `CMakeLists.txt` — Build configuration
- `.gitignore` — Git ignore rules

## 🚧 Known Issues
* Global variables not yet implemented
* Logical ops don’t model C short‑circuit timing
* No optimization / resource tuning in VHDL output
* Potential signal naming collisions

## ✅ Operator Coverage (Summary)
Arithmetic, shifts, bitwise, comparisons, logical (no short‑circuit semantics), unary minus / logical not, control flow + arrays.

## 🗺️ Roadmap (Short)
Next focus areas: global vars, function calls, better diagnostics, naming & optimization improvements, integration & coverage tests. Full details: see docs (architecture / roadmap sections).