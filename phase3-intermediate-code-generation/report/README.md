# hw3 report

|      |                 |
| ---: | :-------------- |
| Name | 楊睿宸           |
|   ID | 112550049       |

## Table of Contents

- [hw3 report](#hw3-report)
  - [Table of Contents](#table-of-contents)
  - [How much time did you spend on this project](#how-much-time-did-you-spend-on-this-project)
  - [Project overview](#project-overview)
    - [Purpose \& Functionality](#purpose--functionality)
    - [scanner.l](#scannerl)
    - [parser.y](#parsery)
      - [Declarations](#declarations)
      - [Grammar (production rules) \& Semantic Actions](#grammar-production-rules--semantic-actions)
      - [User Routine](#user-routine)
    - [AST](#ast)
      - [AstNode (ast.hpp and .cpp)](#astnode-asthpp-and-cpp)
      - [AST Derived Node Classes](#ast-derived-node-classes)
      - [astHelperTypes (.hpp)](#asthelpertypes-hpp)
    - [Visitor Pattern: AstNodeVisitor \& AstDumper](#visitor-pattern-astnodevisitor--astdumper)
  - [What is the hardest you think in this project](#what-is-the-hardest-you-think-in-this-project)
    - [Plenty of Documents](#plenty-of-documents)
      - [Lots of files](#lots-of-files)
      - [C++ Makes Program Behavior Not Clear](#c-makes-program-behavior-not-clear)
  - [Feedback to T.A.s](#feedback-to-tas)


## How much time did you spend on this project

**Around 30 hours in total.**

* **~6 hours**: Reading the spec for the first time and searching online to build a rough understanding.
  The spec couldn’t be understood linearly — I had to jump back and forth, since AST design, scanner, parser, and visitor pattern are tightly connected.
* **Rest of the time**:

  * Designing initial classes like `ProgramNode` and `DeclNode` to get an overview of the assignment’s expectations.
  * Implementing and adjusting AST construction and visitor pattern.
* **~3 hours**: Writing and polishing this report.

---

## Project overview

### Purpose & Functionality

In **hw3**, I reuse the **LALR parser** from hw2 and add semantic actions to construct an **Abstract Syntax Tree (AST)** during parsing.

* The AST is an **intermediate representation** that:

  * records program structure (hierarchy of declarations/statements/expressions), and
  * stores essential attributes (types, operators, names, locations).
* This AST will be used in later assignments for **semantic analysis** and **code generation**.

---

### scanner.l

To support location-aware AST nodes, I modified `scanner.l` to record **position information for each token** using Bison/Flex location tracking.

Core macro:

```c
#define YY_USER_ACTION \
    yylloc.first_line = line_num; \
    yylloc.first_column = col_num; \
    updateCurrentLine(yytext);
```

* `YY_USER_ACTION` runs for **every matched token**.
* It initializes `yylloc` (used by Bison) with the current line/column.
* These locations are then passed to AST node constructors via `@n` in `parser.y`.

---

### parser.y

This file connects **grammar**, **semantic values**, and **AST construction**.

#### Declarations

1. **Includes**

   I include AST and visitor headers, plus my own helper types:

   ```cpp
   #include "AST/AstDumper.hpp"        // Visitor pattern
   #include "AST/ast.hpp"              // Base class: AST node
   #include "AST/program.hpp"          // Program node

   /* ... (17 other node headers) */

   #include "util/astHelperTypes.hpp"  // Helper structs defined by me
   ```

2. **Location type (`YYLTYPE`)**

   I define a custom `YYLTYPE` compatible with Bison’s `%locations`:

   ```c
   #define YYLTYPE yyltype

   typedef struct YYLTYPE {
       uint32_t first_line;
       uint32_t first_column;
       uint32_t last_line;
       uint32_t last_column;
   } yyltype;
   ```

3. **`%union` (semantic values)**

   * **Non-POD C++ types** (e.g., `std::vector`, node classes) are stored as **pointers**.
   * `%union` collects all possible semantic value types:

   ```cpp
   %union {
       /* Basic semantic values */
       char        *str_type;
       int32_t      int_type;
       /* ... (2 more basic types) */

       /* Self-defined types */
       ScalarType   scalar_type_type;  // enum class (ast.hpp)
       Type        *type_type;         // struct (ast.hpp)
       IdList      *id_list_type;      // (astHelperTypes.hpp)

       /* Pointers to AST node types */
       AstNode         *ast_node_ptr;
       ExpressionNode  *expr_ptr;

       DeclNode        *decl_ptr;
       /* ... (14 more pointers) */

       /* Pointers to vectors of nodes */
       std::vector<AstNode *> *ast_node_list_ptr;
       /* ... (3 other list_ptrs) */
   };
   ```

4. **Operator precedence & associativity**

   From **low to high**, all `left`-associative:

   ```yacc
   %left AND OR NOT
   %left '<' LESS_THAN_OR_EQUAL NOT_EQUAL GREATER_THAN_OR_EQUAL '>' '='

   /* ... (5 more lines omitted) */
   ```

5. **Non-terminals & `%type`**

   Some non-terminals carry semantic values and need `%type` declarations.

   * `program` itself does **not** need a semantic value because I store the AST root in a `static AstNode *root;` instead.

   ```yacc
   %type program
   %type <func_ptr> function function_declaration function_definition function_header

   /* ... (48 more lines omitted) */
   ```

---

#### Grammar (production rules) & Semantic Actions

Semantic actions mainly:

* construct AST nodes, and
* connect them according to the grammar structure.

**Example 1 – `while_statement`**

```cpp
loop_statement
    : while_statement { $$ = $1; }
    | for_statement   { $$ = $1; }
    ;

while_statement
    : KW_WHILE expression KW_DO compound_statement KW_END KW_DO
    {
        $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4);
    }
    ;
```

* `while_statement` is typed as `<while_ptr>`, `loop_statement` as `<ast_node_ptr>`.
* I rely on **upcasting** (`WhileNode*` → `AstNode*`) so different loop statement types can be stored uniformly.

**Example 2 – `for_statement`**

* This is the **most complicated** action in `parser.y`.
* It handles:

  * loop variable declaration,
  * lower/upper bounds,
  * loop body (`CompoundStatementNode`),
  * location information for error messages in later assignments.
* I added detailed comments around **lines 620–644** in `parser.y` to explain it step by step.

---

#### User Routine

After `yyparse()` finishes, I dump the AST using the visitor pattern:

```cpp
AstDumper ast_dumper;
root->accept(ast_dumper);

// Non-visitor version (not used in final submission):
// root->print();
```

---

### AST

#### AstNode (ast.hpp and .cpp)

Shared data types for the whole AST live here:

```cpp
struct Location;

enum class ScalarType {
    INTEGER,
    /* ... (4 lines omitted) */
};

struct Type {
    /**
     * Example:
     *   array 3 of array 2 of real;
     *   => scalarType = REAL;
     *      arrRefs   = {3, 2};
     */
    ScalarType scalarType;
    std::vector<uint32_t> arrRefs;
    /* ... (5 lines omitted) */
};

enum class OperatorType {
    // Binary operators
    PLUS,   // "+"
    /* ... (16 lines omitted) */
};

class AstNode;
```

Key idea:

* **`ScalarType` + `arrRefs`** represent P-language types in a way that is much easier to use in hw4 than raw strings.

---

#### AST Derived Node Classes

Each node typically carries:

1. **Attributes** (semantic information)
2. **Child pointers** (tree structure)

Example – constant value container:

```cpp
struct ConstValContainer {
    int32_t     i;  // integer
    double      r;  // real
    std::string s;  // string
    bool        b;  // boolean
};

ConstValContainer constValContainer;
```

Example – declaration node holding child variable nodes:

```cpp
std::vector<VariableNode *> m_variables;
```

So each node combines:

* **what it means** (attributes), and
* **what it contains** (child nodes).

---

#### astHelperTypes (.hpp)

Types here are **not** AST nodes; they are **temporary containers** to move data between grammar rules and constructors.

Example:

```cpp
struct Id {
    Location    location;
    std::string m_name;
};
```

Usage (in `decl.cpp`):

```cpp
for (auto &id : *idList) {
    m_variables.emplace_back(new VariableNode(id, *type));
}
```

* `IdList` groups identifiers plus their locations.
* `DeclNode` later turns them into real `VariableNode` objects.

---

### Visitor Pattern: AstNodeVisitor & AstDumper

I follow the standard visitor pattern:

* Each AST node **inherits** from `AstNode` and implements:

  ```cpp
  void accept(AstNodeVisitor &v) override {
      v.visit(*this);
  }
  ```

* `AstNodeVisitor` declares:

  ```cpp
  virtual void visit(ProgramNode &);
  virtual void visit(DeclNode &);
  // ...
  ```

* `AstDumper` **inherits** from `AstNodeVisitor` and overrides those `visit` functions to print each node’s information in **pre-order**.

Conceptually:

* AST nodes are **data**.
* Visitors (like `AstDumper`) are **operations** on that data.

This makes it straightforward to add another operation (e.g., semantic analyzer in hw4) without modifying the node classes.

---

## What is the hardest you think in this project

### Plenty of Documents

There were many documents to digest:

* `README.md`
* AST guideline
* Visitor pattern tutorial
* Visitor pattern hints

To really understand the project, I had to constantly:

* switch between these documents and
* inspect the project structure and code.

It took time to build a consistent view of how scanner, parser, AST, and visitor are supposed to work together.

#### Lots of files

OOP in C++ is still not very familiar for many students (including me).

* Once I decided to change a **design choice** (for example, how to represent types, or how to store children), I had to touch **many files**:

  * headers for each AST node,
  * the `.cpp` implementations,
  * visitor declarations and definitions,
  * includes and forward declarations.

Keeping everything consistent across all of these was **tedious** and easy to break.

#### C++ Makes Program Behavior Not Clear

Some bugs came from **language details** rather than logic.

Example – union with `std::string`:

```cpp
union ConstValContainer {
    int32_t     i;   // int
    double      r;   // real
    std::string s;   // string
    bool        b;   // bool
};
```

* C++ does **not** automatically call constructors/destructors when switching active union members.
* This caused subtle lifetime issues for the `std::string` member.
* I had to adjust the design (or manually manage construction/destruction) to avoid undefined behavior.

These details made the program’s behavior harder to reason about and increased debugging time.

---

## Feedback to T.A.s

I really appreciate the use of **modern and consistent tools**:

* Markdown reports
* GitHub repositories
* Docker images
* Provided Makefile and test scripts

This setup:

* standardizes the environment,
* avoids wasting time on random system issues, and
* strongly encourages good engineering habits (version control, reproducible builds, clear reporting).

Compared to classmates who need to configure everything by themselves and submit via fragile school systems, I honestly feel lucky to have this environment for the compiler course.

---
