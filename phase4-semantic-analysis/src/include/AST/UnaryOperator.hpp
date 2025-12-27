#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

class UnaryOperatorNode : public ExpressionNode {
   public:
    UnaryOperatorNode(const uint32_t line, const uint32_t col, OperatorType p_operator,
                      ExpressionNode *p_expression /* hw3: operator, expression */);
    ~UnaryOperatorNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    OperatorType getOperator() const {
        return m_operator;
    }
    ExpressionNode *getOperand() const {
        return m_expression;
    }

    void determineTypeOfResult() override;

   private:
    // hw3 work: operator, expression
    OperatorType m_operator;
    ExpressionNode *m_expression;
};

#endif
