#include "AST/return.hpp"

// DONE
ReturnNode::ReturnNode(const uint32_t line, const uint32_t col, ExpressionNode *p_returnVal)
    : AstNode{line, col}, m_return_val(p_returnVal) {}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    m_return_val->accept(p_visitor);
}
