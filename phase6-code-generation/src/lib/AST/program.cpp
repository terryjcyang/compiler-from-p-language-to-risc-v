#include "AST/program.hpp"

// hw3 work
ProgramNode::ProgramNode(const uint32_t line, const uint32_t col, const char *const p_name,
                         std::vector<DeclNode *> *p_declarations,
                         std::vector<FunctionNode *> *p_functions,
                         CompoundStatementNode *const p_body)
    : AstNode{line, col},
      m_name(p_name),
      m_declarations(*p_declarations),
      m_functions(*p_functions),
      m_body(p_body) {}

ProgramNode::~ProgramNode() {
    for (auto decl : m_declarations) {
        delete decl;
    }
    for (auto func : m_functions) {
        delete func;
    }
    delete m_body;
}

// visitor pattern version
void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    for (auto &decl : m_declarations) {
        decl->accept(p_visitor);
    }
    for (auto &func : m_functions) {
        func->accept(p_visitor);
    }
    m_body->accept(p_visitor);
}
