#include "AST/read.hpp"

// DONE
ReadNode::ReadNode(const uint32_t line, const uint32_t col, VariableReferenceNode *p_var_ref)
    : AstNode{line, col}, m_var_ref(p_var_ref) {}

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    m_var_ref->accept(p_visitor);
}
