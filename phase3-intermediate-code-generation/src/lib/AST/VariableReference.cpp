#include "AST/VariableReference.hpp"

// DONE
VariableReferenceNode::VariableReferenceNode(const uint32_t line, const uint32_t col,
                                             const char *const p_name)
    : ExpressionNode{line, col}, m_name(p_name), m_indices(0) {}

// DONE
VariableReferenceNode::VariableReferenceNode(const uint32_t line, const uint32_t col,
                                             const char *const p_name, ExpressionNode *p_mostInnerIndex)
    : VariableReferenceNode(line, col, p_name) {
    m_indices.emplace_back(p_mostInnerIndex);
}

void VariableReferenceNode::addInnerIndex(ExpressionNode *p_index){
    m_indices.emplace_back(p_index);
}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    for (auto &expr_ptr : m_indices) {
        expr_ptr->accept(p_visitor);
    }
}
