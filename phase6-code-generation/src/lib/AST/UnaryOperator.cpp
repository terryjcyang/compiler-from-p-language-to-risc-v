#include "AST/UnaryOperator.hpp"

// hw3 work
UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col,
                                     OperatorType p_operator, ExpressionNode *p_expression)
    : ExpressionNode{line, col}, m_operator(p_operator), m_expression(p_expression) {}

UnaryOperatorNode::~UnaryOperatorNode() {
    delete m_expression;
}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    m_expression->accept(p_visitor);
}

void UnaryOperatorNode::determineTypeOfResult() {
    Type type = m_expression->getTypeOfResult();
    ScalarType scalar = type.scalarType;
    OperatorType op = m_operator;

    // Some operand already corrupted
    if (scalar == ScalarType::UNKNOWN) {
        m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        return;
    }
    // An array cannot be an operand of binary operation
    if (!type.arrRefs.empty()) {
        // some operand is array
        m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        return;
    }

    // Neg operator
    if (op == OperatorType::NEGATION) {
        if (scalar <= ScalarType::REAL) {  // int / real
            m_typeOfResult.scalarType = scalar;
        } else {
            m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        }
        return;
    }
    // Not operator
    if (op == OperatorType::NOT) {
        if (scalar == ScalarType::BOOLEAN) {
            m_typeOfResult.scalarType = ScalarType::BOOLEAN;
        } else {
            m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        }
        return;
    }
    perror("UnaryOperator cpp determineTypeOfResult error\n");
    m_typeOfResult.scalarType = ScalarType::UNKNOWN;
}
