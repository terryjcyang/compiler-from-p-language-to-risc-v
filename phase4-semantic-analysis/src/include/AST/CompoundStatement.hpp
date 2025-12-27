#ifndef __AST_COMPOUND_STATEMENT_NODE_H
#define __AST_COMPOUND_STATEMENT_NODE_H

#include "AST/ast.hpp"
///
#include "AST/decl.hpp"
#include "visitor/AstNodeVisitor.hpp"

class CompoundStatementNode : public AstNode {
   public:
    CompoundStatementNode(
        const uint32_t line, const uint32_t col, std::vector<DeclNode *> *p_declarations,
        std::vector<AstNode *> *p_statements /* hw3: declarations, statements */);
    ~CompoundStatementNode() = default;

    ///
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

   private:
    // hw3 work: declarations, statements
    std::vector<DeclNode *> m_declarations;
    /**
     * A list of "statement node"
     * 
     * Nodes that are classified as statement nodes
     *   - Compound statement node
     *   - Assignment node
     *   - Print node
     *   - Read node
     *   - If node
     *   - While node
     *   - For node
     *   - Return node
     *   - Function call node
     */
    std::vector<AstNode *> m_statements;
};

#endif
