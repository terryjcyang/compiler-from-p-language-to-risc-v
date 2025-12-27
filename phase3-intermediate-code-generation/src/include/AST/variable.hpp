#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ast.hpp"
#include "AST/ConstantValue.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include "util/astHelperTypes.hpp"
#include <vector>

class VariableNode : public AstNode {
   public:
    // DONE
    VariableNode(const Id &p_id, Type p_type);
    VariableNode(const Id &p_id, ConstantValueNode *p_constValNode);

    ~VariableNode() = default;

    const char *getNameCString() const {
        return m_name.c_str();
    }
    const Type getType() const {
        return m_type;
    }

    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

   private:
    // DONE: variable name, type, constant value
    std::string m_name;
    Type m_type;
    ConstantValueNode *m_const_value;   // optional(0 or 1 node)
};

#endif
