#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/ast.hpp"
///
#include "AST/CompoundStatement.hpp"
#include "AST/decl.hpp"
#include "AST/function.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <vector>
#include <memory>

#include <string>

class ProgramNode final : public AstNode {
   private:
    /* m_ prefix: member */
    std::string m_name;
    // DONE: return type, declarations, functions, compound statement
    //ReturnType m_returnType; // TODO (hw4)
    std::vector<DeclNode *> m_declarations;
    std::vector<FunctionNode *> m_functions;
    CompoundStatementNode *m_body;

   public:
    ~ProgramNode() = default;
    ProgramNode(const uint32_t line, const uint32_t col,
                const char *const p_name, std::vector<DeclNode *> *p_declarations,
                std::vector<FunctionNode *> *p_functions, 
                CompoundStatementNode *const p_body
                /* DONE: return type, declarations, functions,
                 *       compound statement */);

    const char *getNameCString() const {
        return m_name.c_str();
    }
    ///
    // visitor pattern version
    void accept(AstNodeVisitor &p_visitor) override {
        p_visitor.visit(*this);
    }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
