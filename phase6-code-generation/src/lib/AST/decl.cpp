#include "AST/decl.hpp"
#include <stdexcept>

namespace {

/* Make a fresh ConstantValueNode so every identifier owns its own literal. */

ConstantValueNode *cloneConstantValueNode(const ConstantValueNode *node) {
    const auto &loc = node->getLocation();
    const auto constVal = node->getConstVal();
    switch (constVal.scalarType) {
        case ScalarType::INTEGER:
            return new ConstantValueNode(loc.line, loc.col, constVal.valContainer.integer);
        case ScalarType::REAL:
            return new ConstantValueNode(loc.line, loc.col, constVal.valContainer.real);
        case ScalarType::STRING:
            return new ConstantValueNode(loc.line, loc.col, constVal.valContainer.string);
        case ScalarType::BOOLEAN:
            return new ConstantValueNode(loc.line, loc.col, constVal.valContainer.boolean);
        default:
            throw std::runtime_error("Unsupported constant type in declaration");
    }
}

}  // namespace

/**
 * variable decl
 * dump type to variables
 */
// hw3 work
DeclNode::DeclNode(const uint32_t line, const uint32_t col, IdList *idList, Type *type
                   /* hw3: identifiers, type */)
    : AstNode{line, col}, m_variables(0) {
    for (auto &id : *idList) {
        m_variables.emplace_back(new VariableNode(id, *type));
    }
}

/**
 * const decl
 * dump literal to variables
 */
// hw3 work
DeclNode::DeclNode(const uint32_t line, const uint32_t col, IdList *idList, bool positive,
                   ConstantValueNode *constValNode /* hw3: identifiers, constant */)
    : AstNode{line, col} {
    if (!positive) {
        constValNode->setNegative();
        /** for my design,
         * initially, the constValNode's loc is integer_literal/real_literal
         * **not including** negative sign.
         * So we need to -1 here.
         */
        Location temLoc = constValNode->getLocation();
        temLoc.col -= 1;
        constValNode->setLocation(temLoc);
    }
    for (auto &id : *idList) {
        m_variables.emplace_back(new VariableNode(id, cloneConstantValueNode(constValNode)));
    }
    delete constValNode;
}
DeclNode::DeclNode(const uint32_t line, const uint32_t col, IdList *idList,
                   ConstantValueNode *constValNode /* hw3: identifiers, constant */)
    : AstNode{line, col} {
    for (auto &id : *idList) {
        m_variables.emplace_back(new VariableNode(id, cloneConstantValueNode(constValNode)));
    }
    delete constValNode;  // consume the template literal once copies are made
}

void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    for (auto &var : m_variables) {
        var->accept(p_visitor);
    }
}

DeclNode::~DeclNode() {
    for (auto var : m_variables) {
        delete var;
    }
}
