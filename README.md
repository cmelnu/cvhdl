# 🚀 Compi: C-to-VHDL Compiler

Compi is a tool that translates C source code into VHDL hardware descriptions.  
Easily convert your algorithms from software to hardware! 🖥️➡️🔌

## ✨ Features

- 📝 Lexical analysis of C code
- 🏗️ Basic parsing of function declarations
- 🛠️ Generation of VHDL entity skeletons

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

## ⚠️ Limitations

- Only basic function declarations are supported
- Function bodies and complex statements are not yet translated

## 🗺️ Roadmap

- Support for expressions and statements
- Translation of control structures
- Improved VHDL code generation

## 📄 License

This project is licensed under the GNU General Public License v3.0.