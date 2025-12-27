#include "AST/while.hpp"

// hw3 work
WhileNode::WhileNode(const uint32_t line, const uint32_t col, ExpressionNode *p_condition,
                     CompoundStatementNode *p_body)
    : AstNode{line, col}, m_condition(p_condition), m_body(p_body) {}

WhileNode::~WhileNode() {
    delete m_condition;
    delete m_body;
}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    m_condition->accept(p_visitor);
    m_body->accept(p_visitor);
}
