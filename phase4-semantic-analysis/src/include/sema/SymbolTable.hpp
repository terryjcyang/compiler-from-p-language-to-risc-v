#ifndef SEMA_SYMBOL_TABLE_HPP
#define SEMA_SYMBOL_TABLE_HPP

#include "AST/ast.hpp"            // struct Type
#include "AST/ConstantValue.hpp"  // struct ConstVal
#include <vector>
#include <stack>

#define MAX_SYMBOL_NAME_LEN 32

enum class KindOfSymbol { PROGRAM, FUNCTION, PARAMETER, VARIABLE, LOOP_VAR, CONSTANT };

struct SymbolEntry {
    SymbolEntry(const char *p_name, KindOfSymbol p_kind, int p_level, Type p_type);

    char name[MAX_SYMBOL_NAME_LEN + 1];  // The extra part of an identifier will be discarded.

    // The name type of the symbol.
    KindOfSymbol kind;

    // level of scope
    int level;

    Type type;  // can be used for the return type of a function.

    // Other attributes of the symbol:
    struct Attribute {
        // the value of a constant
        ConstVal constVal;
        // list of the types of the formal parameters of a function.
        std::vector<Type> typesOfFormalParam;
    } attribute;

    // For now, it is used for:
    // Skip the rest of semantic checks if there are any errors in the node of the declaration of the referred symbol
    bool declErr;
};

struct SymbolTable {
    /* Operations */
    SymbolEntry *findSymbol(const char *targetId);
    void printTable();

    /*
    A symbol table can be implemented in one of the following ways to represent entries in it:
    - Linear list
    - Binary search tree
    - Hash table

    For simplicity, I choose Linear list.
    */
    std::vector<SymbolEntry> entries;
};

struct SymbolManager {
    SymbolManager();

    void pushScope(SymbolTable *new_scope);
    void popScope();

    SymbolEntry *findSymbol(const char *id);
    bool isRedeclared(const char *id);

    /* Entry related */
    void pushEntry(const char *name, KindOfSymbol kind, Type type);
    // for constant
    void pushEntry(const char *name, KindOfSymbol kind, Type type, ConstVal p_constVal);
    // for function
    void pushEntry(const char *name, KindOfSymbol kind, Type type, std::vector<Type> &paramTypes);

    void setCurrEntryDeclErr();

    void printCurrTable() const;

    std::vector<SymbolTable *> tables;
    /**
     * inLoopInit
     * 
     * - Purpose:
     *      to distinguish [ loop var decl ,and loop init assignment ] from [ other var decl, and other assignment ]
     * 
     * - When to set:
     *      in visit(ForNode &), before visitChildren()
     * - When to read:
     *      in visit(variableNode &)
     *      in visit(assignmentNode &)
     * - When to clear:
     *      in visit(assignmentNode &)
     */
    bool inLoopInit;
    // scope management:
    int currlvl;
    /**
     * A `FunctionNode` should share the same symbol table with its body (`CompoundStatementNode`).
     */
    bool upperIsFunction;
    std::stack<ScalarType> returnTypes;
};

#endif
