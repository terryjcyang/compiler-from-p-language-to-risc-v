#include "AST/ConstantValue.hpp"
#include <string>

/* struct ConstVal */

ScalarType ConstVal::getConstType() const {
    return scalarType;
}
std::string ConstVal::getConstValInString() const {
    switch (scalarType) {
        case ScalarType::INTEGER:
            return std::to_string(valContainer.integer);
            break;
        case ScalarType::REAL:
            return std::to_string(valContainer.real);
            break;
        case ScalarType::STRING:
            return valContainer.string;
            break;
        case ScalarType::BOOLEAN:
            return (valContainer.boolean) ? "true" : "false";
            break;
        default:
            return "const val error";
            break;
    }
}

/* class ConstantValueNode */

// hw3 work
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     int32_t c /* hw3: constant value */)
    : ExpressionNode{line, col, ScalarType::INTEGER} {
    m_const_val.scalarType = ScalarType::INTEGER;
    m_const_val.valContainer.integer = c;
}
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     double c /* hw3: constant value */)
    : ExpressionNode{line, col, ScalarType::REAL} {
    m_const_val.scalarType = ScalarType::REAL;
    m_const_val.valContainer.real = c;
}
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     std::string c /* hw3: constant value */)
    : ExpressionNode{line, col, ScalarType::STRING} {
    m_const_val.scalarType = ScalarType::STRING;
    m_const_val.valContainer.string = c;
}
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     bool c /* hw3: constant value */)
    : ExpressionNode{line, col, ScalarType::BOOLEAN} {
    m_const_val.scalarType = ScalarType::BOOLEAN;
    m_const_val.valContainer.boolean = c;
}

ConstVal ConstantValueNode::getConstVal() const {
    return m_const_val;
}

void ConstantValueNode::setNegative() {
    ScalarType constType = m_const_val.getConstType();
    if (constType != ScalarType::INTEGER && constType != ScalarType::REAL) {
        printf("setNegative error");
        return;
    } else if (constType == ScalarType::INTEGER) {
        m_const_val.valContainer.integer *= -1;
    } else if (constType == ScalarType::REAL) {
        m_const_val.valContainer.real *= -1;
    }
}