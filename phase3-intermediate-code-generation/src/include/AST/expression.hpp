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
    ~ExpressionNode() = default;

  protected:
    // for carrying type of result of an expression
    // TODO: for next assignment
};

#endif
