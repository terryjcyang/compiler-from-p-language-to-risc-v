#include "AST/variable.hpp"
#include <string>

// hw3 work
// with a type; w/o (ConstantValueNode *)
VariableNode::VariableNode(const Id &p_id, Type p_type)
    : AstNode(0, 0), m_name(p_id.m_name), m_type(p_type), m_const_value(nullptr) {
    location = p_id.location;
}
// with (ConstantValueNode *); w/o type explicitly provided
VariableNode::VariableNode(const Id &p_id, ConstantValueNode *p_constValNode)
    : AstNode(0, 0),
      m_name(p_id.m_name),
      m_type(p_constValNode->getConstVal().getConstType()),
      m_const_value(p_constValNode) {
    location = p_id.location;
}

void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    if (m_const_value) {
        m_const_value->accept(p_visitor);
    }
}
