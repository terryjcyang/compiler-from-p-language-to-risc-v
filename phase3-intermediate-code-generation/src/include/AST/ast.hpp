#ifndef AST_AST_NODE_H
#define AST_AST_NODE_H

#include <cstdint>
#include <vector>

class AstNodeVisitor;

struct Location {
    uint32_t line;
    uint32_t col;

    ~Location() = default;
    Location(const uint32_t line, const uint32_t col) : line(line), col(col) {}
};

///
/**
 * Types I defined in AST
 * shared btw many files
 *  */

enum class ScalarType {
    INTEGER,
    REAL,
    STRING,
    BOOLEAN,
    VOID
};  // void is not a keyword in P lang., it leaves blank(for return type)
struct Type {
    /**
   * Ex1: array 3 of array 2 of real;
   *      ->  
   *      scalarType = REAL;
   *      arrRefs = {3, 2};
   * 
   * Ex2: real;
   *      ->  
   *      scalarType = REAL;
   *      arrRefs = {};
   */
    ScalarType scalarType;
    std::vector<uint32_t> arrRefs;
    Type(ScalarType s) : scalarType(s), arrRefs(0) {}
    void add_outer_arr(uint32_t n) {
        // possibly type check
        arrRefs.insert(arrRefs.begin(), n);
    }
};
enum class OperatorType {
    /* Binary op */
    PLUS,                   // "+"
    SUBTRACTION,            // "-"
    MULTIPLICATION,         // "*"
    DIVISION,               // "/"
    MOD,                    // "mod"
    LESS_THAN,              // "<"
    LESS_THAN_OR_EQUAL,     // "<="
    NOT_EQUAL,              // "<>"
    GREATER_THAN_OR_EQUAL,  // ">="
    GREATER_THAN,           // ">"
    EQUAL,                  // "="
    AND,                    // "and"
    OR,                     // "or"

    /* Unary op */
    NEGATION,  // "-" ("neg")
    NOT        // "not"
};

class AstNode {
   protected:
    Location location;

   public:
    virtual ~AstNode() = 0;
    AstNode(const uint32_t line, const uint32_t col);

    // Delete copy/move operations to avoid slicing. [1]
    // And "You almost never want to copy or move polymorphic objects. They
    // generally live on the heap, and are accessed via (smart) pointers." [2]
    // [1]
    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-copy-virtual
    // [2] https://stackoverflow.com/a/54792149

    AstNode(const AstNode &) = delete;
    AstNode(AstNode &&) = delete;
    AstNode &operator=(const AstNode &) = delete;
    AstNode &operator=(AstNode &&) = delete;

    const Location &getLocation() const;
    /**
     * for production rule whose head is constant_declaration
     * (see that in parser.y)
     */
    void setLocation(const Location);

    // not visitor pattern:
    //virtual void print() = 0;

    ///
    // p_ prefix: parameter
    virtual void accept(AstNodeVisitor &p_visitor) = 0;
    virtual void visitChildNodes(AstNodeVisitor &p_visitor) {};
};

#endif
