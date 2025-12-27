#include "AST/while.hpp"

// DONE
WhileNode::WhileNode(const uint32_t line, const uint32_t col, ExpressionNode *p_condition,
                     CompoundStatementNode *p_body)
    : AstNode{line, col}, m_condition(p_condition), m_body(p_body) {}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    m_condition->accept(p_visitor);
    m_body->accept(p_visitor);
}
