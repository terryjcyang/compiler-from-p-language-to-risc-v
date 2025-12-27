#include "sema/SymbolTable.hpp"
#include <cstring>
#include <memory>
#include <sstream>

extern uint32_t opt_sym_table; /* declared in scanner.l */

// class SymbolEntry

SymbolEntry::SymbolEntry(const char *p_name, KindOfSymbol p_kind, int p_level, Type p_type,
                         int p_addrOfLocal)
    : kind(p_kind), level(p_level), type(p_type), declErr(false), addrOfLocal(p_addrOfLocal) {
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
    /**
     * hw5 mod:
     * since we need to restore table from m_symbol_table_of_scoping_nodes
     */
    //pushScope(std::make_unique<SymbolTable>());
}

void SymbolManager::pushScope(Table new_scope) {
    tables.emplace_back(std::move(new_scope));
    currlvl++;
}

SymbolManager::Table SymbolManager::popScope() {
    /**
     * opt_sym_table is 1 when scanner sees //&D+
     *                  0 when              //&D-
     */
    if (opt_sym_table) {
        // dump symbol table
        printCurrTable();
    }

    /**
     * hw5 mod:
     * code generation needs symbol tables.
     */
    // The reason I convert raw ptr to unique ptr is
    // that I originally use raw ptr but unique ptr is used in TA's starter code in hw5.
    Table table_ptr = std::move(tables.back());
    tables.pop_back();
    currlvl--;
    return table_ptr;
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
    /**
     * hw5 mod:
     * addrOfNextLocal
     */
    int addrOfLocal;
    if (currlvl == 0) {   // global
        addrOfLocal = 0;  // 0: n/a
    } else {              // local
        if (listOfAddrOfNextLocal.empty()) {
            perror("listOfAddrOfNextLocal.empty() in pushEntry()\n");
            exit(1);
        }
        addrOfLocal = listOfAddrOfNextLocal.back();
        listOfAddrOfNextLocal.back() -= 4;  // for now, all is int
    }
    tables.back()->entries.emplace_back(name, kind, currlvl, type, addrOfLocal);
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