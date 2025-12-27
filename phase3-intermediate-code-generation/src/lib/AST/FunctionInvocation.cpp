#include "AST/FunctionInvocation.hpp"

// DONE
FunctionInvocationNode::FunctionInvocationNode(const uint32_t line, const uint32_t col,
                                               const char *const p_name,
                                               std::vector<ExpressionNode *> *p_expressions)
    : ExpressionNode{line, col}, m_name(p_name), m_arguments(*p_expressions) {}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    for (auto &expr_ptr : m_arguments) {
        expr_ptr->accept(p_visitor);
    }
}
