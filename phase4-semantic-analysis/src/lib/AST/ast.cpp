#include <AST/ast.hpp>
#include <sstream>

// struct Type
void Type::add_outer_arr(int32_t n) {
    // possibly type check
    arrRefs.insert(arrRefs.begin(), n);
}
bool Type::isSameType(const Type &t2) const {
    const Type &t1 = *this;
    if (t1.scalarType != t2.scalarType) {
        // type of element is diff.
        return false;
    }
    if (t1.arrRefs.size() != t2.arrRefs.size()) {
        // dimension of array is diff.
        return false;
    };
    for (int i = 0; i < t1.arrRefs.size(); i++) {
        if (t1.arrRefs[i] != t2.arrRefs[i]) {
            // size of that dimension is diff.
            return false;
        }
    }
    return true;
}
std::string Type::typeToString() const {
    std::stringstream typeSs;
    typeSs << ScalarTypeStrings[static_cast<int>(scalarType)];
    if (arrRefs.size()) {
        typeSs << " ";
        for (auto &size : arrRefs) {
            typeSs << "[" << size << "]";
        }
    }
    return typeSs.str();
}

// prevent the linker from complaining
AstNode::~AstNode() {}

AstNode::AstNode(const uint32_t line, const uint32_t col) : location(line, col) {}

const Location &AstNode::getLocation() const {
    return location;
}
void AstNode::setLocation(const Location newLoc) {
    location = newLoc;
}