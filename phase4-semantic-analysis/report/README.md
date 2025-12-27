# hw4 report

|||
|-:|:-|
|Name|楊睿宸|
|ID|112550049|

## How much time did you spend on this project

**~44 hours total:**

- **Reading Specification (~3 hours):** Carefully reading the `README.md` and error message specifications.
- **Implementation (~32 hours / 4 days):** Developing the symbol table, semantic analyzer, and visitor methods during the day.
- **Debugging (~8 hours / 1 day):** Fixing semantic checks and verifying against test cases.
- **Writing Report (~1 hour):** Summarizing the architecture and feedback.

## Project overview

This project implements **Semantic Analysis** for the **P language** by extending the AST constructed in the previous assignment. The core logic involves traversing the AST using the **Visitor Pattern**, managing scopes via a **Symbol Table**, and performing type checking.

### 1. Symbol Table Management

I implemented a hierarchical symbol table structure to manage scopes and identifiers efficiently.

#### a) Data Structures
- **`SymbolEntry`**: Represents a single symbol (variable, function, constant, etc.). It stores the name, kind, level, type, and specific attributes (like constant values or function parameter types).
- **`SymbolTable`**: A linear list (`std::vector<SymbolEntry>`) representing a single scope.
- **`SymbolManager`**: Manages the stack of active scopes.

#### b) Scope Handling
The `SymbolManager` handles entering and exiting scopes. A key design choice was using `std::vector` instead of `std::stack` for managing table layers.

```cpp
class SymbolManager {
    // ...
    // Use vector to allow iteration through outer scopes for lookup
    std::vector<SymbolTable *> tables; 
    
    // Helper to track context
    bool inLoopInit;
    bool upperIsFunction;
    std::stack<ScalarType> returnTypes;
};
```

This allows the `lookup` function to iterate from the current scope (back of vector) to the global scope (front of vector) to find symbols, adhering to static scoping rules.

### 2. Semantic Analysis (Visitor Pattern)

The `SemanticAnalyzer` class inherits from `AstNodeVisitor`. It traverses the AST in a specific order to perform checks.

#### a) Traversal Order

1.  **Scope Entry:** Push a new symbol table if the node starts a scope.
2.  **Pre-order Checks:** Specifically for **Symbol Redeclaration**. If a symbol is redeclared, it is reported immediately, and the symbol is *not* added to the table to prevent cascading errors.
3.  **Child Traversal:** Visit child nodes to propagate type information up the tree.
4.  **Post-order Checks:** Perform the majority of semantic validations (Type checking, Operator compatibility, etc.) using the types determined from children.
5.  **Scope Exit:** Pop the symbol table (and dump it if `opt_sym_table` is set).

#### b) Type Information Propagation

I modified the AST nodes to hold a `m_typeOfResult` field. When visiting expression nodes, the analyzer determines the result type based on operands and stores it in the node for parent nodes to access.

```cpp
// Example: Determining type for Binary Operator
void BinaryOperatorNode::determineTypeOfResult() {
    Type type1 = m_left_operand->getTypeOfResult();
    Type type2 = m_right_operand->getTypeOfResult();
    // Logic to check for integer/real compatibility and set m_typeOfResult
    // ...
}
```

### 3. Special Case Handling

  - **For Loop Scopes:** The loop variable initialization and the loop body are treated as distinct interactions. I added a flag `inLoopInit` to `SymbolManager` to distinguish between declaring the loop variable (which might shadow an outer variable) and assigning to it.
  - **Error Recovery:** When a semantic error is found (e.g., `ScalarType::UNKNOWN`), the analyzer reports the error but continues processing to find as many errors as possible, while suppressing cascading errors caused by the initial corruption.

## What is the hardest you think in this project

### Primary Challenge: Lack of Linear Workflow

The hardest part was the complexity of the regulations and the "unknown unknowns." Unlike previous assignments, I couldn't form a clear, linear implementation plan from the start. I frequently had to jump back and forth between the `README` spec, the `error-message.md`, and the code.

### Technical Challenge: Data Structure Pivot

A specific technical hurdle was the `SymbolManager` implementation. Initially, I used a `std::stack` to store symbol tables, thinking it naturally fit the "push/pop" scope concept. However, I later realized that `std::stack` does not support random access iterators. To implement the `lookup` function (which checks the current scope *and* all outer scopes), I had to refactor the manager to use `std::vector`. This allowed me to iterate backwards from the inner-most scope to the global scope.

## Feedback to T.A.s

1.  **Syntax Error in Spec Code:**
    In the `README.md` output format example, there is a missing semicolon after the demarcation call:

    ```c
    dumpDemarcation('-') // <--- Missing semicolon here
    {
      printf("%-33s", "func");
      // ...
    }
    ```

2.  **Incorrect Language Highlighting:**
    Some code snippets in the spec are marked as C code but contain C++ syntax (e.g., constructors/classes). While GitHub renders this okay, some local Markdown viewers do not highlight it correctly.

    ```c
    // marked as c, but is C++
    m_error_printer.print(SymbolRedeclarationError(node.getLocation(), node.getNameCString()));
    ```

3.  **Logic Order in Spec vs. Implementation:**
    The "Recommended Workflow" in the description suggests:

    1.  Push a new symbol table if this node forms a scope.
    2.  Insert the symbol into current symbol table.

    However, for the **Program Node**, the expectation (based on sample outputs) is that the program name entry and other global variables/functions reside in the **same** symbol table (the Global Scope). Following the strict "Push then Insert" logic for the Program Node would create a separate scope just for the Program name, which seems counter-intuitive compared to standard scoping rules.
