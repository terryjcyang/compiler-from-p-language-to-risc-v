#include "AST/for.hpp"

// DONE
ForNode::ForNode(const uint32_t line, const uint32_t col, DeclNode *p_loop_var_decl,
                 AssignmentNode *p_init_stmt, ConstantValueNode *p_condition,
                 CompoundStatementNode *p_body)
    : AstNode{line, col},
      m_loop_var_decl(p_loop_var_decl),
      m_init_stmt(p_init_stmt),
      m_condition(p_condition),
      m_body(p_body) {}

void ForNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    m_loop_var_decl->accept(p_visitor);
    m_init_stmt->accept(p_visitor);
    m_condition->accept(p_visitor);
    m_body->accept(p_visitor);
}
