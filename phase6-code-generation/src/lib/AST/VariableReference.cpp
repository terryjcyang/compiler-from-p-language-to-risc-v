#include "AST/VariableReference.hpp"

// hw3 work
VariableReferenceNode::VariableReferenceNode(const uint32_t line, const uint32_t col,
                                             const char *const p_name)
    : ExpressionNode{line, col}, m_name(p_name), m_indices(0) {}

// hw3 work
VariableReferenceNode::VariableReferenceNode(const uint32_t line, const uint32_t col,
                                             const char *const p_name,
                                             ExpressionNode *p_mostInnerIndex)
    : VariableReferenceNode(line, col, p_name) {
    m_indices.emplace_back(p_mostInnerIndex);
}

void VariableReferenceNode::addInnerIndex(ExpressionNode *p_index) {
    m_indices.emplace_back(p_index);
}
const std::vector<ExpressionNode *> &VariableReferenceNode::getIndices() const {
    return m_indices;
}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    for (auto &expr_ptr : m_indices) {
        expr_ptr->accept(p_visitor);
    }
}

VariableReferenceNode::~VariableReferenceNode() {
    for (auto idx : m_indices) {
        delete idx;
    }
}
