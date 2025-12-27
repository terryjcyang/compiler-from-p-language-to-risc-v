#include "AST/AstDumper.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/decl.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"

#include <cstdio>
#include <sstream>
#include <string>

const char *const scalarTypeStrings[] = {"integer", "real", "string", "boolean", "void"};
const char *const operatorTypeStrings[] = {
    /* Binary op */
    "+", "-", "*", "/", "mod", "<", "<=", "<>", ">=", ">", "=", "and", "or",
    /* Unary op */
    "neg", "not"};

/* -------------------- function declarations -------------------- */
static std::string typeToString(Type type);

/* -------------------- function definitions -------------------- */

void AstDumper::printIndent() const {
    std::printf("%s", m_indenter.indent().c_str());
}

void AstDumper::visit(ProgramNode &p_program) {
    printIndent();

    std::printf("program <line: %u, col: %u> %s %s\n", p_program.getLocation().line,
                p_program.getLocation().col, p_program.getNameCString(),
                "void");  // for hw3, all return types are "void"

    m_indenter.increaseLevel();
    p_program.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(DeclNode &p_decl) {
    printIndent();

    std::printf("declaration <line: %u, col: %u>\n", p_decl.getLocation().line,
                p_decl.getLocation().col);

    m_indenter.increaseLevel();
    p_decl.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(VariableNode &p_variable) {
    printIndent();

    // DONE: name, type
    Type type = p_variable.getType();

    std::printf("variable <line: %u, col: %u> %s %s\n", p_variable.getLocation().line,
                p_variable.getLocation().col, p_variable.getNameCString(),
                typeToString(type).c_str());

    m_indenter.increaseLevel();
    p_variable.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(ConstantValueNode &p_constant_value) {
    printIndent();

    // DONE: string of constant value
    std::printf("constant <line: %u, col: %u> %s\n", p_constant_value.getLocation().line,
                p_constant_value.getLocation().col,
                p_constant_value.getConstValInCString().c_str());
}

void AstDumper::visit(FunctionNode &p_function) {
    printIndent();

    // DONE: name, prototype string
    /**
     * get prototype string
     * <return type> (<param1 type>, <param2 type>, ...)
     */
    std::stringstream protoTypeSs;
    int idx;
    idx = static_cast<int>(p_function.getReturnType());
    protoTypeSs << scalarTypeStrings[idx] << " (";
    bool isFirst = true;
    for (auto &declNode_ptr : p_function.getParameters()) {
        for (auto &variableNode_ptr : declNode_ptr->getVariables()) {
            if (!isFirst) {
                protoTypeSs << ", ";
            }
            Type type = variableNode_ptr->getType();
            protoTypeSs << typeToString(type);
            isFirst = false;
        }
    }
    protoTypeSs << ")";

    std::printf("function declaration <line: %u, col: %u> %s %s\n", p_function.getLocation().line,
                p_function.getLocation().col, p_function.getNameCString(),
                protoTypeSs.str().c_str());

    m_indenter.increaseLevel();
    p_function.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(CompoundStatementNode &p_compound_statement) {
    printIndent();

    std::printf("compound statement <line: %u, col: %u>\n", p_compound_statement.getLocation().line,
                p_compound_statement.getLocation().col);

    m_indenter.increaseLevel();
    p_compound_statement.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(PrintNode &p_print) {
    printIndent();

    std::printf("print statement <line: %u, col: %u>\n", p_print.getLocation().line,
                p_print.getLocation().col);

    m_indenter.increaseLevel();
    p_print.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(BinaryOperatorNode &p_bin_op) {
    printIndent();

    // DONE: operator string
    OperatorType op = p_bin_op.getOperator();
    std::string opStr = operatorTypeStrings[static_cast<int>(op)];
    std::printf("binary operator <line: %u, col: %u> %s\n", p_bin_op.getLocation().line,
                p_bin_op.getLocation().col, opStr.c_str());

    m_indenter.increaseLevel();
    p_bin_op.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(UnaryOperatorNode &p_un_op) {
    printIndent();

    // DONE: operator string
    OperatorType op = p_un_op.getOperator();
    std::string opStr = operatorTypeStrings[static_cast<int>(op)];
    std::printf("unary operator <line: %u, col: %u> %s\n", p_un_op.getLocation().line,
                p_un_op.getLocation().col, opStr.c_str());

    m_indenter.increaseLevel();
    p_un_op.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(FunctionInvocationNode &p_func_invocation) {
    printIndent();

    // DONE: function name
    std::printf("function invocation <line: %u, col: %u> %s\n",
                p_func_invocation.getLocation().line, p_func_invocation.getLocation().col,
                p_func_invocation.getNameCString());

    m_indenter.increaseLevel();
    p_func_invocation.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(VariableReferenceNode &p_variable_ref) {
    printIndent();

    // DONE: variable name
    std::printf("variable reference <line: %u, col: %u> %s\n", p_variable_ref.getLocation().line,
                p_variable_ref.getLocation().col, p_variable_ref.getNameCString());

    m_indenter.increaseLevel();
    p_variable_ref.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(AssignmentNode &p_assignment) {
    printIndent();

    std::printf("assignment statement <line: %u, col: %u>\n", p_assignment.getLocation().line,
                p_assignment.getLocation().col);

    m_indenter.increaseLevel();
    p_assignment.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(ReadNode &p_read) {
    printIndent();

    std::printf("read statement <line: %u, col: %u>\n", p_read.getLocation().line,
                p_read.getLocation().col);

    m_indenter.increaseLevel();
    p_read.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(IfNode &p_if) {
    printIndent();

    std::printf("if statement <line: %u, col: %u>\n", p_if.getLocation().line,
                p_if.getLocation().col);

    m_indenter.increaseLevel();
    p_if.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(WhileNode &p_while) {
    printIndent();

    std::printf("while statement <line: %u, col: %u>\n", p_while.getLocation().line,
                p_while.getLocation().col);

    m_indenter.increaseLevel();
    p_while.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(ForNode &p_for) {
    printIndent();

    std::printf("for statement <line: %u, col: %u>\n", p_for.getLocation().line,
                p_for.getLocation().col);

    m_indenter.increaseLevel();
    p_for.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

void AstDumper::visit(ReturnNode &p_return) {
    printIndent();

    std::printf("return statement <line: %u, col: %u>\n", p_return.getLocation().line,
                p_return.getLocation().col);

    m_indenter.increaseLevel();
    p_return.visitChildNodes(*this);
    m_indenter.decreaseLevel();
}

/* -------------------- Helper functions -------------------- */

static std::string typeToString(Type type) {
    std::stringstream typeSs;
    typeSs << scalarTypeStrings[static_cast<int>(type.scalarType)];
    if (type.arrRefs.size()) {
        typeSs << " ";
        for (auto &size : type.arrRefs) {
            typeSs << "[" << size << "]";
        }
    }
    return typeSs.str();
}
