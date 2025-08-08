# 🚀 Compi: C-to-VHDL Compiler

Compi is a tool that translates C source code into VHDL hardware descriptions.  
Easily convert your algorithms from software to hardware! 🖥️➡️🔌

## ✨ Features

- 📝 Lexical analysis of C code (tokenizer)
- 🏗️ Parsing of function declarations and parameter lists
- 🌳 Abstract Syntax Tree (AST) construction
- 🛠️ Generation of VHDL entities and architecture skeletons

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

## 🧪 Testing

You can test the compiler with example C files provided in the `examples/` directory:

```bash
./compi ../examples/example.c output.vhdl
```

## ⚠️ Limitations

- Only basic function declarations and parameter lists are supported
- Function bodies and complex statements are not yet translated
- No support for pointers, arrays, or advanced C features in parameters

## 🗺️ Roadmap

- Support for expressions and statements
- Translation of control structures (if, for, while)
- Improved VHDL code generation
- Support for variable declarations and assignments
- Error reporting improvements

## 📄 License

This project is licensed under the GNU General Public License v3.0.