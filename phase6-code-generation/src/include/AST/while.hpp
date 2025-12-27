#ifndef __AST_WHILE_NODE_H
#define __AST_WHILE_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "AST/CompoundStatement.hpp"

class WhileNode : public AstNode {
   public:
    WhileNode(const uint32_t line, const uint32_t col, ExpressionNode *p_condition,
              CompoundStatementNode *p_body
              /* hw3: expression, compound statement */);
    ~WhileNode() override;

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

   private:
    // hw3 work: expression, compound statement
    ExpressionNode *m_condition;
    CompoundStatementNode *m_body;
};

#endif
