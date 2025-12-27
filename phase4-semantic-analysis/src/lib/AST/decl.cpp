#include "AST/decl.hpp"

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
        m_variables.emplace_back(new VariableNode(id, constValNode));
    }
}
DeclNode::DeclNode(const uint32_t line, const uint32_t col, IdList *idList,
                   ConstantValueNode *constValNode /* hw3: identifiers, constant */)
    : AstNode{line, col} {
    for (auto &id : *idList) {
        m_variables.emplace_back(new VariableNode(id, constValNode));
    }
}

void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    for (auto &var : m_variables) {
        var->accept(p_visitor);
    }
}
