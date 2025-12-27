#include "AST/read.hpp"

// hw3 work
ReadNode::ReadNode(const uint32_t line, const uint32_t col, VariableReferenceNode *p_var_ref)
    : AstNode{line, col}, m_var_ref(p_var_ref) {}

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    m_var_ref->accept(p_visitor);
}

ReadNode::~ReadNode() {
    delete m_var_ref;
}
