#include "AST/if.hpp"

// hw3 work
IfNode::IfNode(const uint32_t line, const uint32_t col, ExpressionNode *p_condition,
               CompoundStatementNode *p_body, CompoundStatementNode *p_bodyOfElse)
    : AstNode{line, col}, m_condition(p_condition), m_body(p_body), m_body_of_else(p_bodyOfElse) {}

IfNode::~IfNode() {
    delete m_condition;
    delete m_body;
    delete m_body_of_else;
}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    m_condition->accept(p_visitor);
    m_body->accept(p_visitor);
    if (m_body_of_else) {
        m_body_of_else->accept(p_visitor);
    }
}
