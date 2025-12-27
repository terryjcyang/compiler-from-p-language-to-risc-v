#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/decl.hpp"
#include "util/astHelperTypes.hpp"  // IdList, ...

class FunctionNode : public AstNode {
   public:
    FunctionNode(const uint32_t line, const uint32_t col, const char *const p_name, 
                 std::vector<DeclNode *> *p_parameterList, ScalarType p_returnType = ScalarType::VOID
                 /* hw3: name, declarations, return type,
                  *       compound statement (optional) */);
    ~FunctionNode() override;

    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *getNameCString() const {
        return m_name.c_str();
    }
    std::vector<DeclNode *> &getParameters() {
        return m_parameters;
    }
    int getNumOfParameters() const;
    std::string getFormalParametersInString() const;
    ScalarType getReturnType() const {
        return m_returnType;
    }
    void setCompoundStatement(CompoundStatementNode *p_compoundStatementNode);

   private:
    // hw3 work: name, declarations, return type, compound statement
    std::string m_name;
    // zero or more
    std::vector<DeclNode *> m_parameters;
    ScalarType m_returnType;
    // optional
    CompoundStatementNode *m_compound_statement;
};

#endif
