#include "AST/CompoundStatement.hpp"

// hw3 work
CompoundStatementNode::CompoundStatementNode(const uint32_t line, const uint32_t col,
                                             std::vector<DeclNode *> *p_declarations,
                                             std::vector<AstNode *> *p_statements)
    : AstNode{line, col}, m_declarations(*p_declarations), m_statements(*p_statements) {}

///
void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    for (auto &declNode_ptr : m_declarations) {
        declNode_ptr->accept(p_visitor);
    }
    for(auto &stmtNode_ptr : m_statements){
        /**
         * Datatype of stmtNode_ptr is actually (AstNode *), 
         * but we leverage polymorphism.
         */
        stmtNode_ptr->accept(p_visitor);
    }
}
