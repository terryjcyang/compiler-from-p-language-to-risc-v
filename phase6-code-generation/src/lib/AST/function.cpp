#include "AST/function.hpp"
#include <sstream>

// hw3 work
FunctionNode::FunctionNode(
    const uint32_t line, const uint32_t col, const char *const p_name,
    std::vector<DeclNode *> *p_parameterList,
    ScalarType
        p_returnType /* hw3: name, declarations, return type, compound statement (optional) */)
    : AstNode{line, col},
      m_name(p_name),
      m_parameters(*p_parameterList),
      m_returnType(p_returnType),
      m_compound_statement(nullptr) {}

FunctionNode::~FunctionNode() {
    for (auto param : m_parameters) {
        delete param;
    }
    delete m_compound_statement;
}

void FunctionNode::setCompoundStatement(CompoundStatementNode *p_compoundStatementNode) {
    m_compound_statement = p_compoundStatementNode;
}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    for (auto &declPtr : m_parameters) {
        declPtr->accept(p_visitor);
    }
    if (m_compound_statement) {
        m_compound_statement->accept(p_visitor);
    }
}

int FunctionNode::getNumOfParameters() const {
    int num = 0;
    for (auto &decl : m_parameters) {
        num += decl->getVariables().size();
    }
    return num;
}
std::string FunctionNode::getFormalParametersInString() const {
    std::stringstream formalParamSs;
    bool isFirst = true;
    for (auto &declNode_ptr : m_parameters) {
        for (auto &variableNode_ptr : declNode_ptr->getVariables()) {
            if (!isFirst) {
                formalParamSs << ", ";
            }
            Type type = variableNode_ptr->getType();
            formalParamSs << type.typeToString();
            isFirst = false;
        }
    }
    return formalParamSs.str();
}
