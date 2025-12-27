#include "AST/BinaryOperator.hpp"

// hw3 work
BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col,
                                       ExpressionNode *p_left_operand, OperatorType p_operator,
                                       ExpressionNode *p_right_operand)
    : ExpressionNode{line, col},
      m_operator(p_operator),
      m_left_operand(p_left_operand),
      m_right_operand(p_right_operand) {}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // hw3 work
    m_left_operand->accept(p_visitor);
    m_right_operand->accept(p_visitor);
}

void BinaryOperatorNode::determineTypeOfResult() {
    Type type1 = m_left_operand->getTypeOfResult();
    Type type2 = m_right_operand->getTypeOfResult();
    ScalarType scalar1 = type1.scalarType;
    ScalarType scalar2 = type2.scalarType;
    OperatorType op = m_operator;

    // Some operand already corrupted
    if (scalar1 == ScalarType::UNKNOWN || scalar2 == ScalarType::UNKNOWN) {
        m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        //fprintf(stderr, "<<b op: has corrupted, line:%u>>\n", this->getLocation().line);
        if (scalar1 == ScalarType::UNKNOWN) {
            //fprintf(stderr, ">> is scalar1 unk");
        }
        return;
    }
    // An array cannot be an operand of binary operation
    if (!type1.arrRefs.empty() || !type2.arrRefs.empty()) {
        // some operand is array
        m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        //fprintf(stderr, "b op: is arr");
        return;
    }

    // String concatenation
    if (op == OperatorType::PLUS && scalar1 == ScalarType::STRING &&
        scalar2 == ScalarType::STRING) {
        m_typeOfResult.scalarType = ScalarType::STRING;
        return;
    }
    // Arithmetic operator
    if (OperatorType::PLUS <= op && op <= OperatorType::DIVISION) {
        if (scalar1 > ScalarType::REAL || scalar2 > ScalarType::REAL) {
            m_typeOfResult.scalarType = ScalarType::UNKNOWN;
            return;
        }
        if (scalar1 == ScalarType::INTEGER && scalar2 == ScalarType::INTEGER) {
            m_typeOfResult.scalarType = ScalarType::INTEGER;
            //fprintf(stderr, "both int in b op");
        } else {
            // type coercion
            m_typeOfResult.scalarType = ScalarType::REAL;
            //fprintf(stderr, "real in b op");
        }
        return;
    }
    // Mod operator
    if (op == OperatorType::MOD) {
        if (scalar1 == ScalarType::INTEGER && scalar2 == ScalarType::INTEGER) {
            m_typeOfResult.scalarType = ScalarType::INTEGER;
        } else {
            m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        }
        return;
    }
    // Boolean operator
    if (op == OperatorType::AND || op == OperatorType::OR) {
        if (scalar1 == ScalarType::BOOLEAN && scalar2 == ScalarType::BOOLEAN) {
            m_typeOfResult.scalarType = ScalarType::BOOLEAN;
        } else {
            m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        }
        return;
    }
    // Relational operator
    if (OperatorType::LESS_THAN <= op && op <= OperatorType::EQUAL) {
        if (scalar1 > ScalarType::REAL || scalar2 > ScalarType::REAL) {
            m_typeOfResult.scalarType = ScalarType::UNKNOWN;
        } else {
            m_typeOfResult.scalarType = ScalarType::BOOLEAN;
        }
        return;
    }
    perror("BinaryOperator cpp determineTypeOfResult error\n");
    m_typeOfResult.scalarType = ScalarType::UNKNOWN;
}
