#ifndef __AST_READ_NODE_H
#define __AST_READ_NODE_H

#include "AST/ast.hpp"
#include "AST/VariableReference.hpp"
#include "visitor/AstNodeVisitor.hpp"

class ReadNode : public AstNode {
   public:
    ReadNode(const uint32_t line, const uint32_t col, VariableReferenceNode *p_var_ref
             /* hw3: variable reference */);
    ~ReadNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const VariableReferenceNode *getVarRef() const {
        return m_var_ref;
    }

   private:
    // hw3 work: variable reference
    VariableReferenceNode *m_var_ref;
};

#endif
