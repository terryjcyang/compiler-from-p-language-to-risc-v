# P Language to RISC-V Compiler

> [!WARNING]
> This repository contains course assignment implementations.
> If you are taking this course (or a similar one), DO NOT copy, reuse, or submit any part of this code as your own work.
> Doing so may result in **plagiarism violations** and academic penalties.

A complete compiler implementation for the **P** programming language that generates RISC-V assembly code. This project was developed as part of the "Introduction to Compiler Design" course at NYCU (National Yang Ming Chiao Tung University) by Prof. Yi-Ping You.

## Overview

This repository contains a multi-phase compiler that translates programs written in the P language into executable RISC-V assembly code. The compiler is built incrementally across five phases, each implementing a fundamental component of the compilation pipeline.

## Project Structure

The compiler is organized into the following phases:

### **Phase 1: Lexical Analysis** ([phase1-lexical-analysis](phase1-lexical-analysis/))

- **Due Date:** September 23, 2025
- **Description:** Implements a scanner (lexical analyzer) using Flex to tokenize P language source code
- **Key Components:**
  - Token recognition (keywords, operators, identifiers, constants)
  - String and comment handling
  - Error detection for invalid lexical patterns

### **Phase 2: Syntactic Analysis** ([phase2-syntatic-analysis](phase2-syntatic-analysis/))

- **Due Date:** October 14, 2025
- **Description:** Implements a parser (syntax analyzer) using Bison to validate program structure
- **Key Components:**
  - Grammar rules for P language constructs
  - Syntax validation
  - Parse tree generation

### **Phase 3: Intermediate Code Generation** ([phase3-intermediate-code-generation](phase3-intermediate-code-generation/))

- **Due Date:** November 14, 2025
- **Description:** Extends the parser to construct an Abstract Syntax Tree (AST)
- **Key Components:**
  - AST node definitions for all language constructs
  - Visitor pattern implementation
  - AST traversal and visualization

### **Phase 4: Semantic Analysis** ([phase4-semantic-analysis](phase4-semantic-analysis/))

- **Due Date:** December 5, 2025
- **Description:** Performs semantic validation and type checking
- **Key Components:**
  - Symbol table construction and management
  - Type checking and type inference
  - Scope management
  - Semantic error detection and reporting

### **Phase 6: Code Generation** ([phase6-code-generation](phase6-code-generation/))

- **Due Date:** December 26, 2025
- **Description:** Generates RISC-V assembly code from the validated AST
- **Key Components:**
  - RISC-V instruction generation
  - Register allocation
  - Stack frame management
  - Runtime support for P language features
  - Support for Spike simulator and RISC-V development boards

## Technology Stack

- **Lexer:** Flex (lex)
- **Parser:** Bison (yacc)
- **Programming Language:** C/C++
- **Target Architecture:** RISC-V
- **Testing:** Python test framework
- **Build System:** Make
- **Simulator:** Spike RISC-V ISA Simulator
- **Optional:** PlatformIO for hardware deployment

## Building the Compiler

Each phase can be built independently. Navigate to the respective phase directory and run:

```bash
cd phase<N>-<phase-name>/src
make
```

For example, to build the final compiler:

```bash
cd phase6-code-generation/src
make
```

## Testing

Each phase includes comprehensive test suites with sample test cases and expected outputs.

To run tests for a specific phase:

```bash
cd phase<N>-<phase-name>/test
make
python3 test.py
```

## P Language Features

The P language is a Pascal-like programming language that supports:

- **Data Types:** Integer, Real, Boolean, String, Arrays
- **Control Structures:** if-else, while loops, for loops
- **Functions:** Function declarations with parameters and return values
- **Operators:** Arithmetic, logical, and relational operators
- **I/O:** print and read statements
- **Declarations:** Variable and constant declarations
- **Scope:** Nested scopes with proper variable shadowing

## Running Generated Code

The final compiler generates RISC-V assembly that can be executed on:

1. **Spike Simulator:**

   ```bash
   spike --isa=rv32i pk <output.s>
   ```

2. **RISC-V Development Board:**
   See [phase6-code-generation/board](phase6-code-generation/board/) for hardware deployment instructions

## Documentation

Each phase contains detailed documentation:

- **README.md:** Assignment specifications and implementation guidelines
- **report/:** Design decisions and implementation reports
- **test_cases/:** Sample P language programs
- **sample_solutions/:** Expected outputs for validation

Additional resources:

- [AST Guidelines](phase3-intermediate-code-generation/ast_guideline.md)
- [Error Message Specifications](phase4-semantic-analysis/error-message.md)
- [RISC-V Tutorial](phase6-code-generation/RISC-V-tutorial/)
- [Yacc Documentation](phase3-intermediate-code-generation/Yacc.html)

## Project Timeline

| Phase | Topic              | Due Date           |
| ----- | ------------------ | ------------------ |
| 1     | Lexical Analysis   | September 23, 2025 |
| 2     | Syntactic Analysis | October 14, 2025   |
| 3     | AST Construction   | November 14, 2025  |
| 4     | Semantic Analysis  | December 5, 2025   |
| 6     | Code Generation    | December 26, 2025  |

_Note: Phase 5 is integrated into other phases_

## Development Notes

- Each phase builds upon the previous one
- Reference solutions are provided for previous phases
- Both C and C++ are supported (C++ recommended for STL containers)
- Code is designed to be modular and extensible

## Course Information

- **Course:** Introduction to Compiler Design
- **Institution:** National Yang Ming Chiao Tung University (NYCU)
- **Instructor:** Prof. Yi-Ping You
- **Academic Year:** 2025

## License

This project is an educational assignment for a university course. Please refer to course policies regarding code sharing and academic integrity.

---

**Note:** This compiler is designed for educational purposes as part of a compiler design course. While fully functional, it may not include all optimizations found in production compilers.
