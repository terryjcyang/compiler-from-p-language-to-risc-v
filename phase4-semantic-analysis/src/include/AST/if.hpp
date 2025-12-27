#ifndef __AST_IF_NODE_H
#define __AST_IF_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "AST/CompoundStatement.hpp"

class IfNode : public AstNode {
   public:
    IfNode(const uint32_t line, const uint32_t col, ExpressionNode *p_condition,
           CompoundStatementNode *p_body, CompoundStatementNode *p_bodyOfElse = nullptr
           /* hw3: expression, compound statement, compound statement */);
    ~IfNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const ExpressionNode *getCondition() {
        return m_condition;
    }

   private:
    // hw3 work: expression, compound statement, compound statement
    ExpressionNode *m_condition;
    CompoundStatementNode *m_body, *m_body_of_else /* optional */;
};

#endif
