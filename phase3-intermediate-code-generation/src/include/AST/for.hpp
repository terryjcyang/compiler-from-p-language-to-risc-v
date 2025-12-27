#ifndef __AST_FOR_NODE_H
#define __AST_FOR_NODE_H

#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/decl.hpp"
#include "visitor/AstNodeVisitor.hpp"

class ForNode : public AstNode {
   public:
    ForNode(const uint32_t line, const uint32_t col, DeclNode *p_loop_var_decl,
            AssignmentNode *p_init_stmt, ConstantValueNode *p_condition,
            CompoundStatementNode *p_body
            /* DONE: declaration, assignment, expression,
             *       compound statement */
    );
    ~ForNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

   private:
    // DONE: declaration, assignment, expression, compound statement
    /**
     * Note: here, the VariableNode in DeclNode has no ConstValNode.
     */
    DeclNode *m_loop_var_decl;
    /**
     * The ExpressionNode in this AssignmentNode 
     * will only be a constant value node.
     */
    AssignmentNode *m_init_stmt;
    ConstantValueNode *m_condition;
    CompoundStatementNode *m_body;
};

#endif
