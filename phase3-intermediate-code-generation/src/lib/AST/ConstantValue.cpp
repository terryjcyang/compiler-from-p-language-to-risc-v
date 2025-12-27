#include "AST/ConstantValue.hpp"
#include <string>

// DONE
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     int32_t c /* DONE: constant value */)
    : ExpressionNode{line, col}, constType(ScalarType::INTEGER) {
    constValContainer.i = c;
}
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     double c /* DONE: constant value */)
    : ExpressionNode{line, col}, constType(ScalarType::REAL) {
    constValContainer.r = c;
}
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     std::string c /* DONE: constant value */)
    : ExpressionNode{line, col}, constType(ScalarType::STRING) {
    constValContainer.s = c;
}
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                                     bool c /* DONE: constant value */)
    : ExpressionNode{line, col}, constType(ScalarType::BOOLEAN) {
    constValContainer.b = c;
}


ScalarType ConstantValueNode::getConstType(){
    return constType;
}
std::string ConstantValueNode::getConstValInCString() const {
    switch (constType) {
        case ScalarType::INTEGER:
            return std::to_string(constValContainer.i);
            break;
        case ScalarType::REAL:
            return std::to_string(constValContainer.r);
            break;
        case ScalarType::STRING:
            return constValContainer.s;
            break;
        case ScalarType::BOOLEAN:
            return (constValContainer.b) ? "true" : "false";
            break;
        default:
            return "const val error";
            break;
    }
}

void ConstantValueNode::setNegative() {
    if (constType != ScalarType::INTEGER && constType != ScalarType::REAL) {
        printf("setNegative error");
        return;
    } else if (constType == ScalarType::INTEGER) {
        constValContainer.i *= -1;
    } else if (constType == ScalarType::REAL){
        constValContainer.r *= -1;
    }
}