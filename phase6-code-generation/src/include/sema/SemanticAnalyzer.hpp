#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "sema/ErrorPrinter.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include "sema/SymbolTable.hpp"

#include <unordered_map>

class SemanticAnalyzer final : public AstNodeVisitor {
   public:
    /**
    * hw5 mod:
    * 
    */
    using AstNodeAddr = const AstNode *;

   private:
    ErrorPrinter m_error_printer{stderr};
    // DONE: something like symbol manager (manage symbol tables)
    //       context manager, return type manager
    SymbolManager m_symbolManager;
    /**
     * hw5 mod:
     */
    /// @brief Four kinds of AST nodes opens a scope: program, function, loop, and
    /// compound statement. The symbol table of the scope they opened is stored
    /// and mapped back to the AST node. This is for the scope structure to be
    /// reconstructed while generating code.
    std::unordered_map<AstNodeAddr, SymbolManager::Table> m_symbol_table_of_scoping_nodes;

   public:
    /**
     * hw5 mod:
     */
    /// @return The symbol table of the AST nodes that open a scope: program,
    /// function, loop, and compound statement. This is for the scope structure
    /// to be reconstructed while generating code.
    /// @note This function is called after the semantic analysis is done and can
    /// only be called once.
    std::unordered_map<AstNodeAddr, SymbolManager::Table> &&acquireSymbolTableOfScopingNodes() {
        return std::move(m_symbol_table_of_scoping_nodes);
    }

    ~SemanticAnalyzer() = default;
    SemanticAnalyzer() = default;

    bool hasSemanticError() {
        return m_error_printer.hasSemanticErr();
    }

    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;
};

#endif
