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
    int32_t i = -10;   // int
    double r = -10.0;  // real
    std::string s;     // string
    bool b = false;    // bool
};

class ConstantValueNode : public ExpressionNode {
   public:
    ConstantValueNode(const uint32_t line, const uint32_t col,
                      int32_t c /* DONE: constant value */);
    ConstantValueNode(const uint32_t line, const uint32_t col, double c /* DONE: constant value */);
    ConstantValueNode(const uint32_t line, const uint32_t col,
                      std::string c /* DONE: constant value */);
    ConstantValueNode(const uint32_t line, const uint32_t col, bool c /* DONE: constant value */);

    ~ConstantValueNode() = default;

    ScalarType getConstType();
    std::string getConstValInCString() const;
    void setNegative();

    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }

   private:
    // DONE: constant value
    ScalarType constType;
    ConstValContainer constValContainer;
};

#endif
