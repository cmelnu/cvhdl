# Compi: C-to-VHDL Compiler

Minimal C subset → VHDL translator. Focused on simple functions, control flow, expressions, and arrays to explore software → hardware mapping.

## Key Features (High-Level)
* Tokenizer, recursive-descent parser, AST builder
* Expressions with precedence (arith / shifts / bitwise / compare / logical / unary)
* Control flow: `if / else if / else`, `while`, `for`, `break`, `continue`
* Arrays with declarations, initializers, indexed access
* Basic VHDL code generation (entity/architecture skeleton + signal mapping)
* Type mapping: `int|float|double|char` → suitable VHDL types
* GoogleTest unit tests (auto-discovered via CTest)
* Sphinx docs

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

**Developer Debug Output:**

To enable verbose debug output for developers, configure the build with the `-DDEBUG=ON` argument:

```bash
cmake -DDEBUG=ON ..
make
```

This will provide additional debug prints and diagnostics during parsing and code generation.

## 🧪 Testing

GoogleTest is integrated. Each individual test case is auto-discovered (via `gtest_discover_tests`) so CTest shows every `TEST()` separately.

### Quick start
```bash
# Configure (with tests enabled by default) and build
cmake -S . -B build -DENABLE_TESTING=ON
cmake --build build --target test_all -j 4

# Or use the helper script (configures if needed, builds, runs tests)
## 🧪 Testing (Quick)
```bash
cmake -S . -B build -DENABLE_TESTING=ON
cmake --build build --target test_all -j 4
ctest --test-dir build --output-on-failure
```
Shortcut scripts:
```bash
./run_tests.sh      # build + run tests
./build_docs.sh     # build Sphinx docs
```
Filter a single test:
```bash
./build/compi_tests --gtest_filter=TokenTests.BasicLexing
```
Docs helper:
```bash
./build_docs.sh
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

## �️ Roadmap (Short)
Next focus areas: global vars, function calls, better diagnostics, naming & optimization improvements, integration & coverage tests. Full details: see docs (architecture / roadmap sections).