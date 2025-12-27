#include "AST/print.hpp"

// DONE
PrintNode::PrintNode(const uint32_t line, const uint32_t col, ExpressionNode *p_expressNode)
    : AstNode{line, col}, m_expression(p_expressNode) {}

void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    m_expression->accept(p_visitor);
}
