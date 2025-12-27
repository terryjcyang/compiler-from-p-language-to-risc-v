#ifndef __AST_FUNCTION_INVOCATION_NODE_H
#define __AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <string>

class FunctionInvocationNode : public ExpressionNode {
   public:
    FunctionInvocationNode(const uint32_t line, const uint32_t col, const char *const p_name,
                           std::vector<ExpressionNode *> *p_expressions
                           /* DONE: function name, expressions */);
    ~FunctionInvocationNode() = default;

    const char *getNameCString() const {
        return m_name.c_str();
    }

    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

   private:
    // DONE: function name, expressions
    std::string m_name;
    std::vector<ExpressionNode *> m_arguments;
};

#endif
