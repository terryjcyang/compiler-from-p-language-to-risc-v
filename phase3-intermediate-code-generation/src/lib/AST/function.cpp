#include "AST/function.hpp"

// DONE
FunctionNode::FunctionNode(
    const uint32_t line, const uint32_t col, const char *const p_name,
    std::vector<DeclNode *> *p_parameterList,
    ScalarType
        p_returnType /* DONE: name, declarations, return type, compound statement (optional) */)
    : AstNode{line, col},
      m_name(p_name),
      m_parameters(*p_parameterList),
      m_returnType(p_returnType),
      m_compound_statement(nullptr) {}

void FunctionNode::setCompoundStatement(CompoundStatementNode *p_compoundStatementNode) {
    m_compound_statement = p_compoundStatementNode;
}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // DONE
    for (auto &declPtr : m_parameters) {
        declPtr->accept(p_visitor);
    }
    if (m_compound_statement) {
        m_compound_statement->accept(p_visitor);
    }
}
