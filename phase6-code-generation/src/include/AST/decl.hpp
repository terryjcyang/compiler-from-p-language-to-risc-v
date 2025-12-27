#ifndef __AST_DECL_NODE_H
#define __AST_DECL_NODE_H

#include "AST/ast.hpp"
///
#include "AST/variable.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include "util/astHelperTypes.hpp"  // IdList, ...
#include <vector>

class DeclNode : public AstNode {
   public:
    // variable declaration
    DeclNode(const uint32_t line, const uint32_t col, IdList *idList, Type *type
             /* hw3: identifiers, type */);

    // constant variable declaration
    DeclNode(const uint32_t line, const uint32_t col, IdList *idList, bool positive,
             ConstantValueNode *constValNode /* hw3: identifiers, constant */);
    DeclNode(const uint32_t line, const uint32_t col, IdList *idList,
             ConstantValueNode *constValNode /* hw3: identifiers, constant */);

    ~DeclNode() override;

    ///
    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    std::vector<VariableNode *>& getVariables(){
        return m_variables;
    }

   private:
    // hw3 work: variables
    std::vector<VariableNode *> m_variables;
};

#endif
