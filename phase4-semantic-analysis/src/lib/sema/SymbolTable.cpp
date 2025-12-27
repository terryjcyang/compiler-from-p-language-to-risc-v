#include "sema/SymbolTable.hpp"
#include <cstring>
#include <sstream>

extern uint32_t opt_sym_table; /* declared in scanner.l */

// class SymbolEntry

SymbolEntry::SymbolEntry(const char *p_name, KindOfSymbol p_kind, int p_level, Type p_type)
    : kind(p_kind), level(p_level), type(p_type), declErr(false) {
    strncpy(name, p_name, MAX_SYMBOL_NAME_LEN);
    name[MAX_SYMBOL_NAME_LEN] = '\0';  // since strncpy doesn't add '\0' if src string len == n
}

// class SymbolTable

SymbolEntry *SymbolTable::findSymbol(const char *targetId) {
    // linear search
    for (auto &entry : entries) {
        if (strncmp(targetId, entry.name, MAX_SYMBOL_NAME_LEN) == 0) {
            return &entry;
        }
    }
    return nullptr;
}

void dumpDemarcation(const char chr) {
    for (size_t i = 0; i < 110; ++i) {
        printf("%c", chr);
    }
    puts("");
}

void SymbolTable::printTable() {
    //
    dumpDemarcation('=');
    printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type", "Attribute");
    dumpDemarcation('-');
    // Each entry
    const char *const KindStrings[] = {"program",  "function", "parameter",
                                       "variable", "loop_var", "constant"};

    for (auto &entry : entries) {
        // Name
        printf("%-33s", entry.name);
        // Kind
        printf("%-11s", KindStrings[static_cast<int>(entry.kind)]);
        // Level
        if (entry.level == 0) {
            printf("%d%-10s", 0, "(global)");
        } else {
            printf("%d%-10s", entry.level, "(local)");
        }
        // Type
        printf("%-17s", entry.type.typeToString().c_str());
        // Attribute (Optional)
        // constant
        if (entry.kind == KindOfSymbol::CONSTANT) {
            std::string constValStr = entry.attribute.constVal.getConstValInString();
            printf("%-11s", constValStr.c_str());
        }
        // types of formal parameters
        else if (entry.kind == KindOfSymbol::FUNCTION) {
            std::stringstream ss;
            bool isFirst = true;
            for (auto &paramType : entry.attribute.typesOfFormalParam) {
                if (!isFirst) {
                    ss << ", ";
                }
                isFirst = false;
                ss << paramType.typeToString();
            }
            printf("%-11s", ss.str().c_str());
        }
        // still need to format the entry
        else {
            printf("%-11s", "");
        }
        printf("\n");
    }
    //
    dumpDemarcation('-');
}

// class SymbolManager
SymbolManager::SymbolManager() : inLoopInit(false), currlvl(-1), upperIsFunction(false) {
    pushScope(new SymbolTable());
}

void SymbolManager::pushScope(SymbolTable *new_scope) {
    tables.emplace_back(new_scope);
    currlvl++;
}

void SymbolManager::popScope() {
    /**
     * opt_sym_table is 1 when scanner sees //&D+
     *                  0 when              //&D-
     */
    if (opt_sym_table) {
        // dump symbol table
        printCurrTable();
    }

    delete tables.back();
    tables.pop_back();
    currlvl--;
}

// be careful if you push to table after retrieve ptr, the ptr may fail
SymbolEntry *SymbolManager::findSymbol(const char *id) {
    for (int i = tables.size() - 1; i >= 0; i--) {
        SymbolEntry *entry = tables[i]->findSymbol(id);
        if (entry) {
            return entry;
        }
    }
    return nullptr;
}
bool SymbolManager::isRedeclared(const char *id) {
    // Check redecl in current scope
    if (tables.back()->findSymbol(id)) {
        return true;
    }
    // Check all for loop var table (skip those not)
    for (auto &table : tables) {
        if (table->entries.empty()) {
            continue;
        }
        bool inLoopInitTable = (table->entries.front().kind == KindOfSymbol::LOOP_VAR);
        if (inLoopInitTable && table->findSymbol(id)) {
            return true;
        }
    }
    return false;
}

// push entry
void SymbolManager::pushEntry(const char *name, KindOfSymbol kind, Type type) {
    tables.back()->entries.emplace_back(name, kind, currlvl, type);
}
// for constant
void SymbolManager::pushEntry(const char *name, KindOfSymbol kind, Type type, ConstVal p_constVal) {
    pushEntry(name, kind, type);
    tables.back()->entries.back().attribute.constVal = p_constVal;
}
// for function
void SymbolManager::pushEntry(const char *name, KindOfSymbol kind, Type type,
                              std::vector<Type> &paramTypes) {
    pushEntry(name, kind, type);
    tables.back()->entries.back().attribute.typesOfFormalParam = paramTypes;
}
void SymbolManager::setCurrEntryDeclErr() {
    tables.back()->entries.back().declErr = true;
}

void SymbolManager::printCurrTable() const {
    tables.back()->printTable();
}