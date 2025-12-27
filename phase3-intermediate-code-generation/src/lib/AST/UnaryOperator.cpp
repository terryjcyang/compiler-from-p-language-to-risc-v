#include "AST/UnaryOperator.hpp"

// DONE
UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col,
                                     OperatorType p_operator, ExpressionNode *p_expression)
    : ExpressionNode{line, col}, m_operator(p_operator), m_expression(p_expression) {}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    m_expression->accept(p_visitor);
}
