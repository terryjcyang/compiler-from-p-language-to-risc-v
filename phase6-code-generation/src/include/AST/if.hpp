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
    ~IfNode() override;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    ExpressionNode *getCondition() const {
        return m_condition;
    }
    CompoundStatementNode *getBody() const {
        return m_body;
    }
    CompoundStatementNode *getElseBody() const {
        return m_body_of_else;
    }

   private:
    // hw3 work: expression, compound statement, compound statement
    ExpressionNode *m_condition;
    CompoundStatementNode *m_body, *m_body_of_else /* optional */;
};

#endif
