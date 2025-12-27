#ifndef UTIL_AST_HELPER_TYPES_HPP
#define UTIL_AST_HELPER_TYPES_HPP
/**
 * intermediate objects that hold data when parsing 
 */
#include "AST/ast.hpp"
#include <vector>
#include <string>

/* forward declaration */
class DeclNode;

/**
 * for 
 * id list 
 * production rule
 */
struct Id {
    Location location;
    std::string m_name;
    Id(const uint32_t line, const uint32_t col, const char *const p_name)
        : location(line, col), m_name(p_name) {}
};

using IdList = std::vector<Id>;

#endif  // UTIL_AST_HELPER_TYPES_HPP