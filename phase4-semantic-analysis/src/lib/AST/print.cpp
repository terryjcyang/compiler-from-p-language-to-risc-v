#include "AST/print.hpp"

// hw3 work
PrintNode::PrintNode(const uint32_t line, const uint32_t col, ExpressionNode *p_expressNode)
    : AstNode{line, col}, m_expression(p_expressNode) {}

void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    m_expression->accept(p_visitor);
}

const ExpressionNode *PrintNode::getExpression() const {
    return m_expression;
}