#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"
#include "AST/VariableReference.hpp"
#include "visitor/AstNodeVisitor.hpp"

class AssignmentNode : public AstNode {
   public:
    AssignmentNode(const uint32_t line, const uint32_t col, VariableReferenceNode *p_left_val,
                   ExpressionNode *p_expression
                   /* hw3 : variable reference, expression */);
    ~AssignmentNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const VariableReferenceNode *getVarRef() {
        return m_left_val;
    }
    const ExpressionNode *getExpression() {
        return m_expression;
    }

   private:
    // hw3 work: variable reference, expression
    VariableReferenceNode *m_left_val;
    ExpressionNode *m_expression;
};

#endif
