#ifndef __AST_EXPRESSION_NODE_H
#define __AST_EXPRESSION_NODE_H

#include "AST/ast.hpp"

/*
  Nodes that are classified as expression nodes
  - Binary operator node
  - Unary operator node
  - Constant value node
  - Variable reference node
  - Function call node
 */

class ExpressionNode : public AstNode {
   public:
    ExpressionNode(const uint32_t line, const uint32_t col);
    // for constant node
    ExpressionNode(const uint32_t line, const uint32_t col, ScalarType p_typeOfResult);
    ~ExpressionNode() = default;

    Type getTypeOfResult() const;
    void setTypeOfResult(const Type t);

    /**
     * hw4 mod:
     * this function should run during semantic analyses
     * since the type of var_ref and func_invocation are determined during semantic analyses
     * (should not be done too early)
     */
    virtual void determineTypeOfResult() {}
    bool isUnknownType() const;

   protected:
    // for carrying type of result of an expression
    // hw4
    Type m_typeOfResult;
};

#endif
