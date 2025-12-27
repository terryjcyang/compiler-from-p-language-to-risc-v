#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <memory>

class BinaryOperatorNode : public ExpressionNode {
   public:
    BinaryOperatorNode(const uint32_t line, const uint32_t col, ExpressionNode *p_left_operand,
                       OperatorType p_operator,
                       ExpressionNode *p_right_operand /* hw3: operator, expressions */);
    ~BinaryOperatorNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    ExpressionNode *getLeftOperand() const {
        return m_left_operand;
    }
    ExpressionNode *getRightOperand() const {
        return m_right_operand;
    }
    OperatorType getOperator() const {
        return m_operator;
    }

    void determineTypeOfResult() override;

   private:
    // hw3 work: operator, expressions
    OperatorType m_operator;
    ExpressionNode *m_left_operand, *m_right_operand;
};

#endif
