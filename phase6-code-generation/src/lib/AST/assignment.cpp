#include "AST/assignment.hpp"

// hw3 work
AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col,
                               VariableReferenceNode *p_left_val, ExpressionNode *p_expression)
    : AstNode{line, col}, m_left_val(p_left_val), m_expression(p_expression) {}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    m_left_val->accept(p_visitor);
    m_expression->accept(p_visitor);
}

AssignmentNode::~AssignmentNode() {
    delete m_left_val;
    delete m_expression;
}
