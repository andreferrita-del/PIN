PIN

<p align="center">
  <img src="icon/file_00000000f3a4820eb8d8b3137b4f4752.webp" width="180" alt="PIN Logo">
</p><h3 align="center">A Compiler Made in C++</h3><p align="center">
  A programming language built from scratch in C++.
</p><p align="center">
  <img src="https://img.shields.io/badge/Language-C++-blue?style=flat-square" alt="C++">
  <img src="https://img.shields.io/badge/Version-0.6-purple?style=flat-square" alt="Version 0.6">
  <img src="https://img.shields.io/badge/Status-Development-orange?style=flat-square" alt="Development">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=flat-square" alt="Platforms">
</p>---

About

PIN is a programming language project developed from scratch using C++.

Its syntax and design take inspiration from three languages:

- JSON — Simple and readable data structures.
- Python — Clean and expressive syntax.
- Lua — Lightweight scripting concepts.

The goal is to build a programming language that is simple to learn, flexible to use, and powerful enough to grow into a complete development platform.

Features

- Variables and data types
- Numbers, strings and booleans
- Arrays and objects
- Arithmetic and logical operations
- Conditional statements
- Loops ("while" and "for/in")
- Functions and return values
- Recursion
- Array and string indexing
- Built-in functions
- Interactive developer console
- Source code parsing and syntax checking

Example

let name = "Hello from PIN!"

let a = 20
let b = 5

print(name)
print(a + b)
print(a * b)

if a > b {
    print("A is greater than B!")
}

Built-in Functions

PIN currently includes functions such as:

Function| Description
"len()"| Get the length
"str()"| Convert to string
"number()"| Convert to number
"sqrt()"| Square root
"abs()"| Absolute value
"type()"| Get value type
"upper()"| Uppercase text
"lower()"| Lowercase text
"range()"| Generate a range

Developer Console

PIN includes an interactive console.

PIN> help
PIN> version
PIN> pwd
PIN> ls
PIN> mkdir projects
PIN> create hello.pin
PIN> nano hello.pin
PIN> compile hello.pin
PIN> run hello.pin
PIN> exit

The built-in editor is a basic line-oriented editor.

Architecture

The project is organized around these main components:

PIN/
├── include/
│   ├── lexer/
│   ├── parser/
│   └── runtime/
│
├── src/
│   ├── lexer/
│   ├── parser/
│   ├── runtime/
│   └── main.cpp
│
├── icons/
│   └── file_00000000f3a4820eb8d8b3137b4f4752.webp
│
├── build/
└── README.md

How PIN Works

PIN Source Code
      |
      v
    Lexer
      |
      v
    Parser
      |
      v
     AST
      |
      v
    Runtime
      |
      v
    Output

Building

PIN is developed in C++ and can be built using Clang.

Requirements

- C++17 compatible compiler
- Clang or compatible toolchain
- Termux or a Linux development environment

Build on Linux / Termux

From the project directory:

clang++ -std=c++17 -O2 -Wall -Wextra \
  -Iinclude \
  src/main.cpp \
  src/lexer/token.cpp \
  src/lexer/lexer.cpp \
  src/parser/parser.cpp \
  src/runtime/runtime.cpp \
  -o build/host/pin

Sorry i dont know
Windows

The project also has a Windows x64 cross-compilation workflow using MinGW/Clang.

The Windows executable must be built with a compatible Windows-targeting toolchain.

Project Roadmap

- [x] Lexer
- [x] Parser
- [x] Abstract Syntax Tree
- [x] Runtime / Interpreter
- [x] Variables and expressions
- [x] Arrays and objects
- [x] Functions
- [x] Loops and conditions
- [x] Built-in functions
- [x] Interactive console
- [ ] Improved error handling
- [ ] Multiline REPL
- [ ] Improved built-in editor
- [ ] Module system
- [ ] Native code generation
- [ ] Independent executable compilation
- [ ] PIN Core 1.0

Current Version

PIN Core 0.6 — Developer Console

PIN is actively under development. Features, syntax and architecture may change as the language evolves.

Philosophy

«Built from scratch. Designed to evolve.»

PIN is an independent programming language project focused on simplicity, creativity and learning how programming languages work internally.

Author

PIN Development Project

Created with C++.

---

<p align="center">
  <b>PIN — A Compiler Made in C++</b>
  <br>
  <sub>Built from scratch. Made to evolve.</sub>
</p>
