#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <string>

/**
 * a little bit heavier but easy to implement
 * (I tried union before)
 */
struct ConstValContainer {
    int32_t integer = -10;  // int
    double real = -10.0;    // real
    std::string string;     // string
    bool boolean = false;   // bool
};

struct ConstVal {
    ScalarType scalarType;
    ConstValContainer valContainer;
    ScalarType getConstType() const;
    std::string getConstValInString() const;
};

class ConstantValueNode : public ExpressionNode {
   public:
    ConstantValueNode(const uint32_t line, const uint32_t col, int32_t c /* hw3: constant value */);
    ConstantValueNode(const uint32_t line, const uint32_t col, double c /* hw3: constant value */);
    ConstantValueNode(const uint32_t line, const uint32_t col,
                      std::string c /* hw3: constant value */);
    ConstantValueNode(const uint32_t line, const uint32_t col, bool c /* hw3: constant value */);

    ~ConstantValueNode() = default;

    ConstVal getConstVal() const;
    void setNegative();

    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }

   private:
    // hw3 work: constant value
    ConstVal m_const_val;
};

#endif
