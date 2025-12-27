#include "AST/expression.hpp"
// Here, temp let m_typeOfResult(ScalarType::UNKNOWN),
// but should remember to determine its actual type later.
ExpressionNode::ExpressionNode(const uint32_t line, const uint32_t col)
    : AstNode{line, col}, m_typeOfResult(ScalarType::UNKNOWN) {}
ExpressionNode::ExpressionNode(const uint32_t line, const uint32_t col, ScalarType p_typeOfResult)
    : AstNode{line, col}, m_typeOfResult(p_typeOfResult) {}

Type ExpressionNode::getTypeOfResult() const {
    return m_typeOfResult;
}
void ExpressionNode::setTypeOfResult(const Type t) {
    m_typeOfResult = t;
}
bool ExpressionNode::isUnknownType() const {
    return (m_typeOfResult.scalarType == ScalarType::UNKNOWN);
}