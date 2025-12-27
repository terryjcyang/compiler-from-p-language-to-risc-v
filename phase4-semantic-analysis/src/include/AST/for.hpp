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
            /* hw3: declaration, assignment, expression,
             *       compound statement */
    );
    ~ForNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const ConstantValueNode *getInitConstVal() {
        return static_cast<const ConstantValueNode *>(m_init_stmt->getExpression());
    }
    const ConstantValueNode *getCondition() {
        return m_condition;
    }

   private:
    // hw3 work: declaration, assignment, expression, compound statement
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

    /*
    Example program:
        for i := 19 to 22 do
            begin
                var level3 : 3;

                begin
                    var level4 : 4;
                end
            end
        end do
    
    1. DeclNode *m_loop_var_decl:
        { ptr<i, w/o const> }
    2. AssignmentNode *m_init_stmt:
        lval = i, expr = 19
    3. ConstantValueNode *m_condition:
        22
    4. CompoundStatementNode *m_body:
        begin
            var level3 : 3;

            begin
                var level4 : 4;
            end
        end
    */
};

#endif
