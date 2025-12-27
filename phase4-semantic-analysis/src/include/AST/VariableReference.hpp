#ifndef __AST_VARIABLE_REFERENCE_NODE_H
#define __AST_VARIABLE_REFERENCE_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <string>

class VariableReferenceNode : public ExpressionNode {
   public:
    // normal reference
    VariableReferenceNode(const uint32_t line, const uint32_t col, const char *const p_name
                          /* hw3: name */);
    // array reference
    VariableReferenceNode(const uint32_t line, const uint32_t col, const char *const p_name,
                          ExpressionNode *p_mostInnerIndex
                          /* hw3: name, expressions */);

    ~VariableReferenceNode() = default;

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *getNameCString() const {
        return m_name.c_str();
    }
    const std::vector<ExpressionNode *> &getIndices() const;
    void addInnerIndex(ExpressionNode *p_index);

   private:
    // hw3 work: variable name, expressions
    std::string m_name;
    /**
     * indices: outer first
     * E.g.
     *    arr[1][5]
     *    m_indices = {Expr<1>, Expr<5>};
     */
    std::vector<ExpressionNode *> m_indices;
};

#endif
