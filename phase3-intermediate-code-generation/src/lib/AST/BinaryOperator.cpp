#include "AST/BinaryOperator.hpp"

// DONE
BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col,
                                       ExpressionNode *p_left_operand, OperatorType p_operator,
                                       ExpressionNode *p_right_operand)
    : ExpressionNode{line, col},
      m_operator(p_operator),
      m_left_operand(p_left_operand),
      m_right_operand(p_right_operand) {}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    m_left_operand->accept(p_visitor);
    m_right_operand->accept(p_visitor);
}
