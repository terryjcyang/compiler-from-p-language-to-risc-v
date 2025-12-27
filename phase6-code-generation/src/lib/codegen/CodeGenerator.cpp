#include "AST/CompoundStatement.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/program.hpp"
#include "codegen/CodeGenerator.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

CodeGenerator::CodeGenerator(const std::string &source_file_name, const std::string &save_path,
                             std::unordered_map<SemanticAnalyzer::AstNodeAddr, SymbolManager::Table>
                                 &&p_symbol_table_of_scoping_nodes)
    : m_source_file_path(source_file_name),
      m_symbol_table_of_scoping_nodes(std::move(p_symbol_table_of_scoping_nodes)),
      nextL(1) {
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path = save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind('/');
    auto dot_pos = source_file_name.rfind('.');

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    } else {
        slash_pos = 0;
    }
    auto output_file_path{real_path + "/" +
                          source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

std::string getImmediateInString(const ConstVal &constVal) {
    switch (constVal.scalarType) {
        case ScalarType::INTEGER:
        case ScalarType::REAL: {
            return constVal.getConstValInString();
            break;
        }
        case ScalarType::STRING: {
            std::string s = constVal.getConstValInString();
            std::string out;
            out.push_back('"');
            // escape '"' in string
            for (char c : s) {
                if (c == '"') {
                    out.push_back('\\');
                }
                out.push_back(c);
            }
            out.push_back('"');
            return out;
            break;
        }
        case ScalarType::BOOLEAN: {
            return constVal.valContainer.boolean ? "1" : "0";
            break;
        }
        default: {
            fprintf(stderr, "Code generation for this constant type is not supported yet.\n");
            exit(1);
        }
    }
}
std::string getImmediateInString(const ConstantValueNode *constantValueNode) {
    return getImmediateInString(constantValueNode->getConstVal());
}
std::string getImmediateInString(const VariableNode *variableNode) {
    return getImmediateInString(variableNode->getConstValueNode());
}

/* ------------------------------------------------------------------------------------------------- */

void CodeGenerator::visit(ProgramNode &p_program) {
    /* Step 1: Ouput assembly                                   */

    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char *const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue, m_source_file_path.c_str());

    /* Step 2: Push scope                                       */

    // Reconstruct the scope for looking up the symbol entry.
    // Hint: Use m_symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager.pushScope(std::move(m_symbol_table_of_scoping_nodes.at(&p_program)));

    /* Step 3: Visit child nodes & Ouput assembly               */

    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(*this); };
    for_each(p_program.getDeclarations()->begin(), p_program.getDeclarations()->end(),
             visit_ast_node);
    for_each(p_program.getFunctions()->begin(), p_program.getFunctions()->end(), visit_ast_node);

    // main function
    /**
     * We allocate 128 bytes for convenience.
     * In hw5, allocate 128 bytes of local memories is sufficient for the parameters and the local variables.
     */
    // clang-format off
    constexpr const char *const riscv_assembly_main_func =
        "    .section    .text\n"
        "    .align 2\n"
        "    .globl main          # emit symbol 'main' to the global symbol table\n"
        "    .type main, @function\n"
        "main:\n"
        "    # in the function prologue\n"
        "    addi sp, sp, -128    # move stack pointer to lower address to allocate a new stack\n"
        "    sw ra, 124(sp)       # save return address of the caller function in the current stack\n"
        "    sw s0, 120(sp)       # save frame pointer of the last stack in the current stack\n"
        "    addi s0, sp, 128     # move frame pointer to the bottom of the current stack\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_main_func);

    const_cast<CompoundStatementNode *>(p_program.getBody())->accept(*this);

    // clang-format off
    constexpr const char *const riscv_assembly_main_func_epilogue = 
        "    # in the function epilogue\n"
        "    lw ra, 124(sp)       # load return address saved in the current stack\n"
        "    lw s0, 120(sp)       # move frame pointer back to the bottom of the last stack\n"
        "    addi sp, sp, 128     # move stack pointer back to the top of the last stack\n"
        "    jr ra                # jump back to the caller function\n"
        "    .size main, .-main\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_main_func_epilogue);

    /* Step 4: Pop scope                                        */

    m_symbol_manager.popScope();
}

void CodeGenerator::visit(DeclNode &p_decl) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_decl.visitChildNodes(*this);

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(VariableNode &p_variable) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    // No child needs visiting for codegen; const value is handled explicitly below.

    if (m_symbol_manager.currlvl == 0) {
        // global var
        if (p_variable.getConstValueNode() == nullptr) {
            // clang-format off
            constexpr const char *const riscv_assembly_global_var =
                "    .comm %s, 4, 4            # emit object '%s' to .bss section with size = 4, align = 4\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_global_var,
                             p_variable.getNameCString(), p_variable.getNameCString());
        }
        // global const
        else {
            ConstVal constVal = p_variable.getConstValueNode()->getConstVal();
            std::string immediate = getImmediateInString(constVal);

            bool isReal = constVal.scalarType == ScalarType::REAL;
            bool isString = constVal.scalarType == ScalarType::STRING;
            if (isReal || isString) {
                // clang-format off
                constexpr const char *const riscv_assembly_global_const_string =
                    "    .section    .rodata       # emit rodata section\n"
                    "    .align 2\n"
                    ".LC%d:\n"
                    "    .%s %s\n";
                // clang-format on
                const char *directive = isReal ? "float" : "string";
                dumpInstructions(m_output_file.get(), riscv_assembly_global_const_string,
                                 getNextL(), directive, immediate.c_str());
                nextL_add(1);
            }

            // clang-format off
            constexpr const char *const riscv_assembly_global_const =
                "    .section    .rodata       # emit rodata section\n"
                "    .align 2\n"
                "    .globl %s              # emit symbol '%s' to the global symbol table\n"
                "    .type %s, @object\n"
                "%s:\n"
                "    .word %s\n";
            // clang-format on
            std::string content = isString ? ".LC" + std::to_string(getNextL() - 1) : immediate;
            dumpInstructions(m_output_file.get(), riscv_assembly_global_const,
                             p_variable.getNameCString(), p_variable.getNameCString(),
                             p_variable.getNameCString(), p_variable.getNameCString(),
                             content.c_str());
        }
    }
    // local const
    else if (p_variable.getConstValueNode()) {
        const SymbolEntry *entry = m_symbol_manager.findSymbol(p_variable.getNameCString());
        if (entry == nullptr) {
            perror("var not found");
            exit(1);
        }

        p_variable.visitChildNodes(*this);

        if (entry->type.scalarType == ScalarType::REAL) {
            // clang-format off
            constexpr const char *const riscv_assembly_local_const_float =
                "    flw ft0, 0(sp)      # pop the value(of float) from the stack\n"
                "    addi sp, sp, 4\n"
                "    fsw ft0, %d(s0)\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_local_const_float,
                             entry->addrOfLocal);
        } else {
            // clang-format off
            constexpr const char *const riscv_assembly_local_const =
                "    lw t0, 0(sp)      # pop the value(of string) from the stack\n"
                "    addi sp, sp, 4\n"
                "    sw t0, %d(s0)\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_local_const, entry->addrOfLocal);
        }
    }

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    /**
     * NOTE: not all constant value nodes are visited
     *       e.g. global const
     */

    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    // Set the const value

    ScalarType scalarType = p_constant_value.getConstVal().scalarType;
    std::string immediate = getImmediateInString(&p_constant_value);

    if (scalarType == ScalarType::REAL || scalarType == ScalarType::STRING) {
        // clang-format off
        constexpr const char *const riscv_assembly_constVal_load_str =
        // value
            "    j .L%d\n"
            "    .section    .rodata\n"
            "    .align 2\n"
            ".LC%d:\n"
            "    .%s %s\n"
            
        // get address 
            "    .section    .text\n"
            ".L%d:\n";
        // clang-format on
        int labelNo = getNextL();
        nextL_add(1);
        const char *directive = (scalarType == ScalarType::REAL) ? "float" : "string";
        dumpInstructions(m_output_file.get(), riscv_assembly_constVal_load_str, labelNo, labelNo,
                         directive, immediate.c_str(), labelNo);
    }

    // Load value(address for string) to register
    switch (scalarType) {
        case ScalarType::INTEGER:
        case ScalarType::BOOLEAN: {
            // clang-format off
            constexpr const char *const riscv_assembly_constVal_load_int_bool =
                "    li t0, %s\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_constVal_load_int_bool,
                             immediate.c_str());
            break;
        }
        case ScalarType::REAL: {
            // clang-format off
            constexpr const char *const riscv_assembly_constVal_load_real =
                "    lui t0, %%hi(.LC%d)       # load the upper 20 bits of address of the label\n"
                "    flw ft0, %%lo(.LC%d)(t0)\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_constVal_load_real, getNextL() - 1,
                             getNextL() - 1);
            break;
        }
        case ScalarType::STRING: {
            // clang-format off
            constexpr const char *const riscv_assembly_constVal_load_str =
                "    la t0, .LC%d\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_constVal_load_str, getNextL() - 1);
            break;
        }
        default: {
            printf("Const val node: invalid const type during codegen\n");
            exit(1);
        }
    }

    // Push the value(for int, real, and bool)/address(for str) to stack
    if (scalarType == ScalarType::REAL) {
        // clang-format off
        constexpr const char *const riscv_assembly_constVal_real_store =
            "    addi sp, sp, -4\n"
            "    fsw ft0, 0(sp)   # push the immediate(of float) to the stack\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_constVal_real_store);
    } else {
        // clang-format off
        constexpr const char *const riscv_assembly_constVal_others_store =
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)      # push the immediate/address(for str) (of const val) to the stack\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_constVal_others_store);
    }

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(FunctionNode &p_function) {
    /* Step 1: Ouput assembly                                   */

    // clang-format off
    constexpr const char *const riscv_assembly_func =
        "    .section    .text\n"
        "    .align 2\n"
        "    .globl %s          # emit symbol '%s' to the global symbol table\n"
        "    .type %s, @function\n"
        "%s:\n"
        "    # in the function prologue\n"
        "    addi sp, sp, -128    # move stack pointer to lower address to allocate a new stack\n"
        "    sw ra, 124(sp)       # save return address of the caller function in the current stack\n"
        "    sw s0, 120(sp)       # save frame pointer of the last stack in the current stack\n"
        "    addi s0, sp, 128     # move frame pointer to the bottom of the current stack\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_func, p_function.getNameCString(),
                     p_function.getNameCString(), p_function.getNameCString(),
                     p_function.getNameCString());

    /* Step 2: Push scope                                       */

    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(std::move(m_symbol_table_of_scoping_nodes.at(&p_function)));
    m_symbol_manager.upperIsFunction = true;

    /* Step 3: Visit child nodes & Ouput assembly               */

    // Push value of parameters to (callee's) stack
    /**
     * Calling convention:
     *   - The caller places the first eight arguments in registers a0~a7.
     *   - Remaining arguments are pushed (right to left) onto the caller's stack frame.
     * Here in the callee we:
     *   1. Copy each register argument into its slot in our local frame (relative to s0).
     *   2. For stack arguments, load them from the caller's frame (still reachable via s0)
     *      and store them into the matching slot in our own frame.
     */
    const int numOfParam = p_function.getNumOfParameters();
    const std::vector<SymbolEntry> &currTable = m_symbol_manager.tables.back()->entries;

    struct ParamLoc {
        enum class Kind { FloatReg, IntReg, Stack } kind;
        int index;  // register index or stack slot
    };
    std::vector<ParamLoc> paramLocs(numOfParam);
    int floatRegCnt = 0, intRegCnt = 0, stackCnt = 0;
    for (int i = 0; i < numOfParam; ++i) {
        const bool isReal = currTable[i].type.isSameType(ScalarType::REAL);
        if (isReal && floatRegCnt < 8) {
            paramLocs[i] = {ParamLoc::Kind::FloatReg, floatRegCnt++};
        } else if (!isReal && intRegCnt < 8) {
            paramLocs[i] = {ParamLoc::Kind::IntReg, intRegCnt++};
        } else {
            paramLocs[i] = {ParamLoc::Kind::Stack, stackCnt++};
        }
    }

    for (int paramIdx = 0; paramIdx < numOfParam; paramIdx++) {
        const ParamLoc &loc = paramLocs[paramIdx];
        const int addrInCallee = currTable[paramIdx].addrOfLocal;

        if (currTable[paramIdx].type.isSameType(ScalarType::REAL)) {
            if (loc.kind == ParamLoc::Kind::FloatReg) {  // float parameters in fa0 - fa7
                // clang-format off
                constexpr const char *const riscv_assembly_register_float_arg_to_stack =
                    "    fsw fa%d, %d(s0)    # save parameter '%s' in the local stack from register\n";
                // clang-format on
                dumpInstructions(m_output_file.get(), riscv_assembly_register_float_arg_to_stack,
                                 loc.index, addrInCallee, currTable[paramIdx].name);
            } else {  // float parameters stored in caller's stack
                // clang-format off
                constexpr const char *const riscv_assembly_stack_float_arg_to_stack = 
                    "    flw ft0, %d(s0)      # load the value(of parameter) from the caller stack\n"
                    "    fsw ft0, %d(s0)      # save parameter '%s' in the local stack\n";
                // clang-format on
                dumpInstructions(m_output_file.get(), riscv_assembly_stack_float_arg_to_stack,
                                 loc.index * 4, addrInCallee, currTable[paramIdx].name);
            }
        } else {
            if (loc.kind == ParamLoc::Kind::IntReg) {  // non-float parameters in a0 - a7
                // clang-format off
                constexpr const char *const riscv_assembly_register_nonfloat_arg_to_stack =
                    "    sw a%d, %d(s0)    # save parameter '%s' in the local stack from register\n";
                // clang-format on
                dumpInstructions(m_output_file.get(), riscv_assembly_register_nonfloat_arg_to_stack,
                                 loc.index, addrInCallee, currTable[paramIdx].name);
            } else {  // non-float parameters stored in caller's stack
                // clang-format off
                constexpr const char *const riscv_assembly_stack_nonfloat_arg_to_stack = 
                    "    lw t0, %d(s0)      # load the value(of parameter) from the caller stack\n"
                    "    sw t0, %d(s0)      # save parameter '%s' in the local stack\n";
                // clang-format on
                dumpInstructions(m_output_file.get(), riscv_assembly_stack_nonfloat_arg_to_stack,
                                 loc.index * 4, addrInCallee, currTable[paramIdx].name);
            }
        }
    }

    p_function.visitChildNodes(*this);

    // clang-format off
    constexpr const char *const riscv_assembly_func_epilogue = 
        "    # in the function epilogue\n"
        "    lw ra, 124(sp)       # load return address saved in the current stack\n"
        "    lw s0, 120(sp)       # move frame pointer back to the bottom of the last stack\n"
        "    addi sp, sp, 128     # move stack pointer back to the top of the last stack\n"
        "    jr ra                # jump back to the caller function\n"
        "    .size %s, .-%s\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_func_epilogue, p_function.getNameCString(),
                     p_function.getNameCString());

    /* Step 4: Pop scope                                        */

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
    m_symbol_manager.upperIsFunction = false;
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */

    const bool upperIsFunction = m_symbol_manager.upperIsFunction;
    if (upperIsFunction) {
        m_symbol_manager.upperIsFunction = false;
    } else {
        // Reconstruct the scope for looking up the symbol entry.
        m_symbol_manager.pushScope(
            std::move(m_symbol_table_of_scoping_nodes.at(&p_compound_statement)));
    }

    /* Step 3: Visit child nodes & Ouput assembly               */

    // Declarations
    for (auto &decl : p_compound_statement.getDeclarations()) {
        decl->accept(*this);
    }
    // Statements
    for (auto &stmt : p_compound_statement.getStatements()) {
        stmt->accept(*this);

        bool isFuncInvocation = dynamic_cast<FunctionInvocationNode *>(stmt) != nullptr;
        if (isFuncInvocation) {
            // Return value unused in statement context; reclaim the stack slot.
            /// NOTE: even if return is void, we still pushed 'a0' to stack.
            // clang-format off
            constexpr const char *const riscv_assembly_consume_unused_return_value = 
                "    addi sp, sp, 4           # consume unused return value\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_consume_unused_return_value);
        }
    }

    /* Step 4: Pop scope                                        */

    if (upperIsFunction) {
        m_symbol_manager.upperIsFunction = true;
    } else {
        m_symbol_manager.popScope();
    }
}

void CodeGenerator::visit(PrintNode &p_print) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_print.visitChildNodes(*this);

    ScalarType typeToPrint =
        p_print.getExpression()->getTypeOfResult().scalarType;  // only scalar type can be print
    switch (typeToPrint) {
        case ScalarType::INTEGER:
        case ScalarType::STRING: {
            // clang-format off
            constexpr const char *const riscv_assembly_print = 
                "    lw a0, 0(sp)     # pop the value from the stack to the first argument register 'a0'\n"
                "    addi sp, sp, 4\n"
                "    jal ra, %s # call function '%s'\n";
            // clang-format on
            const char *functionToCall =
                (typeToPrint == ScalarType::INTEGER) ? "printInt" : "printString";
            dumpInstructions(m_output_file.get(), riscv_assembly_print, functionToCall,
                             functionToCall);
            break;
        }
        case ScalarType::REAL: {
            // clang-format off
            constexpr const char *const riscv_assembly_print = 
                "    flw fa0, 0(sp)     # pop the value from the stack to the first argument register 'fa0'\n"
                "    addi sp, sp, 4\n"
                "    jal ra, printReal # call function 'printReal'\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_print);
            break;
        }
        default: {
            printf("print: not implemented type\n");
            exit(1);
        }
    }

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_bin_op.visitChildNodes(*this);

    bool leftOperandIsReal =
        p_bin_op.getLeftOperand()->getTypeOfResult().scalarType == ScalarType::REAL;
    bool rightOperandIsReal =
        p_bin_op.getRightOperand()->getTypeOfResult().scalarType == ScalarType::REAL;

    // - Float operations

    if (leftOperandIsReal || rightOperandIsReal) {
        /* 1. Load operands to float registers */

        if (rightOperandIsReal) {
            // clang-format off
            constexpr const char *const riscv_assembly_right =
                "    flw ft0, 0(sp)      # pop the value(of right operand) from the stack\n"
                "    addi sp, sp, 4\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_right);

        } else {
            // Coercion(int -> real)
            // clang-format off
            constexpr const char *const riscv_assembly_right_is_int =
                "    lw t0, 0(sp)      # pop the value(of right operand) from the stack\n"
                "    addi sp, sp, 4\n"
                "    fcvt.s.w ft0, t0        # Convert integer in t0 to float in ft0\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_right_is_int);
        }
        /// NOTE: same as above
        if (leftOperandIsReal) {
            // clang-format off
            constexpr const char *const riscv_assembly_left =
                "    flw ft1, 0(sp)      # pop the value(of left operand) from the stack\n"
                "    addi sp, sp, 4\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_left);
        } else {
            // Coercion(int -> real)
            // clang-format off
            constexpr const char *const riscv_assembly_left_is_int =
                "    lw t1, 0(sp)      # pop the value(of left operand) from the stack\n"
                "    addi sp, sp, 4\n"
                "    fcvt.s.w ft1, t1        # Convert integer in t1 to float in ft1\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_left_is_int);
        }

        /* 2. Float operation */

        OperatorType operatorType = p_bin_op.getOperator();

        std::string riscv_assembly_bin_op_operation;
        switch (operatorType) {
                /* (a) Result is real(float), stored in 'ft0' */

            case OperatorType::PLUS:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    fadd.s ft0, ft1, ft0    # ft0 = ft1 + ft0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::SUBTRACTION:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    fsub.s ft0, ft1, ft0    # ft0 = ft1 - ft0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::MULTIPLICATION:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    fmul.s ft0, ft1, ft0    # ft0 = ft1 * ft0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::DIVISION:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    fdiv.s ft0, ft1, ft0    # ft0 = ft1 / ft0, always save the value in a certain register you choose\n";
                // clang-format on
                break;

                /* (b) Result is bool(int 0/1), stored in 't0' */

            case OperatorType::LESS_THAN:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    flt.s t0, ft1, ft0    # t0 = (ft1 < ft0) ? 1 : 0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::LESS_THAN_OR_EQUAL:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    fle.s t0, ft1, ft0    # t0 = (ft1 <= ft0) ? 1 : 0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::NOT_EQUAL:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    feq.s t0, ft1, ft0    # t0 = (ft1 == ft0) ? 1 : 0\n"
                    "    xori  t0, t0, 1       # Invert: t0 = (ft1 != ft0) ? 1 : 0\n";
                    "                          # t0 = (ft1 != ft0) ? 1 : 0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::GREATER_THAN_OR_EQUAL:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    fle.s t0, ft0, ft1     # t0 = (ft1 >= ft0) ? 1 : 0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::GREATER_THAN:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    flt.s t0, ft0, ft1    # t0 = (ft1 > ft0) ? 1 : 0, always save the value in a certain register you choose\n";
                // clang-format on
                break;
            case OperatorType::EQUAL:
                // clang-format off
                riscv_assembly_bin_op_operation =
                    "    feq.s  t0, ft1, ft0    # t0 = (ft1 == ft0) ? 1 : 0, always save the value in a certain register you choose\n";
                // clang-format on
                break;

            case OperatorType::MOD:
            case OperatorType::AND:
            case OperatorType::OR:
                printf("Invalid bin op for real type\n");
                exit(1);
            default:
                printf("Unknown bin op\n");
                exit(1);
        }
        dumpInstructions(m_output_file.get(), riscv_assembly_bin_op_operation.c_str());

        /* 3. Push result to stack */

        if (OperatorType::PLUS <= operatorType && operatorType <= OperatorType::DIVISION) {
            // clang-format off
            constexpr const char *const riscv_assembly_float_res_store =
                "    addi sp, sp, -4\n"
                "    fsw ft0, 0(sp)      # push the value to the stack\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_float_res_store);
        } else if (OperatorType::LESS_THAN <= operatorType && operatorType <= OperatorType::EQUAL) {
            // clang-format off
            constexpr const char *const riscv_assembly_float_res_store =
                "    addi sp, sp, -4\n"
                "    sw t0, 0(sp)      # push the value to the stack\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_float_res_store);
        } else {
            printf("bin op float: error op");
            exit(1);
        }

        return;
    }

    // - Non float operations

    // clang-format off
    constexpr const char *const riscv_assembly_bin_op_load_operands = 
        "    lw t0, 0(sp)      # pop the value(of right operand) from the stack\n"
        "    addi sp, sp, 4\n"
        "    lw t1, 0(sp)      # pop the value(of left operand) from the stack\n"
        "    addi sp, sp, 4\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_bin_op_load_operands);

    std::string riscv_assembly_bin_op_operation;
    switch (p_bin_op.getOperator()) {
        case OperatorType::PLUS:
            /// NOTE: There is no string concatenation in this hw.
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    add t0, t1, t0    # t0 = t1 + t0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::SUBTRACTION:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    sub t0, t1, t0    # t0 = t1 - t0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::MULTIPLICATION:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    mul t0, t1, t0    # t0 = t1 * t0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::DIVISION:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    div t0, t1, t0    # t0 = t1 / t0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::MOD:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    rem t0, t1, t0    # t0 = t1 % t0, always save the value in a certain register you choose\n";
            // clang-format on
            break;

        case OperatorType::LESS_THAN:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    slt t0, t1, t0    # t0 = (t1 < t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::LESS_THAN_OR_EQUAL:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    slt t0, t0, t1\n"
                "    xori t0, t0, 1\n"
                "                      # t0 = (t1 <= t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::NOT_EQUAL:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    xor  t0, t1, t0      # t0 = 0 if t1 == t0, else non-zero\n"
                /* -- to ensure when equal, res is 1. It is not necessary. */
                "    sltiu t0, t0, 1      # t0 = 1 if t0 == 0 (equal), else 0\n"
                "    xori  t0, t0, 1      # t0 = 0 if equal, else 1\n"
                /* --                                                      */
                "                         # t0 = (t1 != t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::GREATER_THAN_OR_EQUAL:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    slt t0, t1, t0\n"
                "    xori t0, t0, 1\n"
                "                      # t0 = (t1 >= t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::GREATER_THAN:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    slt t0, t0, t1    # t0 = (t1 > t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::EQUAL:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    xor  t0, t1, t0      # t0 = 0 if t1 == t0, else non-zero\n"
                "    sltiu t0, t0, 1      # t0 = 1 if t0 == 0 (equal), else 0\n"
                "                         # t0 = (t1 == t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::AND:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    and t0, t1, t0    # t0 = (t1 & t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;
        case OperatorType::OR:
            // clang-format off
            riscv_assembly_bin_op_operation =
                "    or t0, t1, t0    # t0 = (t1 | t0) ? 1 : 0, always save the value in a certain register you choose\n";
            // clang-format on
            break;

        default:
            printf("Unknown bin op\n");
            exit(1);
            break;
    }
    dumpInstructions(m_output_file.get(), riscv_assembly_bin_op_operation.c_str());

    // clang-format off
    constexpr const char *const riscv_assembly_bin_op_store_result =
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)      # push the value to the stack\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_bin_op_store_result);

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_un_op.visitChildNodes(*this);

    bool operandIsReal = p_un_op.getOperand()->getTypeOfResult().scalarType == ScalarType::REAL;

    // - Float operations

    if (operandIsReal) {
        // clang-format off
        constexpr const char *const riscv_assembly_un_op_load_operand = 
            "    flw ft0, 0(sp)      # pop the value(of operand) from the stack\n"
            "    addi sp, sp, 4\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_un_op_load_operand);

        std::string riscv_assembly_un_op_operation;
        switch (p_un_op.getOperator()) {
            case OperatorType::NEGATION:
                // clang-format off
                riscv_assembly_un_op_operation =
                    "    fsgnjn.s ft0, ft0, ft0     # ft0 = -ft0, always save the value in a certain register you choose\n";
                // clang-format on
                break;

            case OperatorType::NOT:
                printf("un op: invalid type for float\n");
                exit(1);
            default:
                printf("Unknown un op\n");
                exit(1);
        }
        dumpInstructions(m_output_file.get(), riscv_assembly_un_op_operation.c_str());

        // clang-format off
        constexpr const char *const riscv_assembly_bin_op_store_result =
            "    addi sp, sp, -4\n"
            "    fsw ft0, 0(sp)      # push the value to the stack\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_bin_op_store_result);

        return;
    }

    // - Non float operations

    // clang-format off
    constexpr const char *const riscv_assembly_un_op_load_operand = 
        "    lw t0, 0(sp)      # pop the value(of operand) from the stack\n"
        "    addi sp, sp, 4\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_un_op_load_operand);

    std::string riscv_assembly_un_op_operation;
    switch (p_un_op.getOperator()) {
        case OperatorType::NEGATION:
            // clang-format off
            riscv_assembly_un_op_operation =
                "    sub t0, x0, t0    # t0 = 0 - t0, always save the value in a certain register you choose\n";
            // clang-format on
            break;

        case OperatorType::NOT:
            // clang-format off
            riscv_assembly_un_op_operation =
                "    xori t0, t0, 1    # t0 = !t0, always save the value in a certain register you choose\n";
            // clang-format on
            break;

        default:
            printf("Unknown un op\n");
            exit(1);
            break;
    }
    dumpInstructions(m_output_file.get(), riscv_assembly_un_op_operation.c_str());

    // clang-format off
    constexpr const char *const riscv_assembly_bin_op_store_result =
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)      # push the value to the stack\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_bin_op_store_result);

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_func_invocation.visitChildNodes(*this);  // arguments (expr)

    SymbolEntry *entry = m_symbol_manager.findSymbol(p_func_invocation.getNameCString());
    const std::vector<Type> &typesOfParam = entry->attribute.typesOfFormalParam;
    const int numOfParam = typesOfParam.size();

    struct ArgLoc {
        enum class Kind { FloatReg, IntReg, Stack } kind;
        int index;  // register index or stack slot
    };
    std::vector<ArgLoc> argLocs(numOfParam);
    int floatRegCnt = 0, intRegCnt = 0, stackCnt = 0;
    for (int i = 0; i < numOfParam; ++i) {
        const bool isReal = typesOfParam[i].isSameType(ScalarType::REAL);
        if (isReal && floatRegCnt < 8) {
            argLocs[i] = {ArgLoc::Kind::FloatReg, floatRegCnt++};
        } else if (!isReal && intRegCnt < 8) {
            argLocs[i] = {ArgLoc::Kind::IntReg, intRegCnt++};
        } else {
            argLocs[i] = {ArgLoc::Kind::Stack, stackCnt++};
        }
    }
    const int stackSizeBytes = stackCnt * 4;
    const int spAdjustBytes = (numOfParam - stackCnt) * 4;

    if (numOfParam > 0) {
        // t2 is the base of evaluated arguments (top of stack after evaluation).
        // t3 will become the final sp for the call, pointing to the outgoing stack-argument area.
        // clang-format off
        constexpr const char *const riscv_assembly_arg_setup =
            "    mv t2, sp                 # base of evaluated arguments\n"
            "    addi t3, sp, %d           # target sp (keeps only stack args)\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_arg_setup, spAdjustBytes);

        for (int argIdx = 0; argIdx < numOfParam; ++argIdx) {
            const bool isReal = typesOfParam[argIdx].isSameType(ScalarType::REAL);
            const ArgLoc &loc = argLocs[argIdx];
            const int offsetFromTop = (numOfParam - 1 - argIdx) * 4;

            if (isReal) {
                // clang-format off
                constexpr const char *const riscv_assembly_load_arg =
                    "    flw ft0, %d(t2)         # load argument #%d (float) from eval stack\n";
                // clang-format on
                dumpInstructions(m_output_file.get(), riscv_assembly_load_arg, offsetFromTop,
                                 argIdx + 1);

                if (loc.kind == ArgLoc::Kind::FloatReg) {
                    // clang-format off
                    constexpr const char *const riscv_assembly_move_to_reg =
                        "    fsgnj.s fa%d, ft0, ft0 # move argument #%d to float arg register\n";
                    // clang-format on
                    dumpInstructions(m_output_file.get(), riscv_assembly_move_to_reg, loc.index,
                                     argIdx + 1);
                } else {  // stack
                    // clang-format off
                    constexpr const char *const riscv_assembly_store_to_stack =
                        "    fsw ft0, %d(t3)         # store argument #%d to caller stack slot\n";
                    // clang-format on
                    dumpInstructions(m_output_file.get(), riscv_assembly_store_to_stack,
                                     loc.index * 4, argIdx + 1);
                }
            } else {
                // clang-format off
                constexpr const char *const riscv_assembly_load_arg =
                    "    lw t0, %d(t2)          # load argument #%d (int/bool/str) from eval stack\n";
                // clang-format on
                dumpInstructions(m_output_file.get(), riscv_assembly_load_arg, offsetFromTop,
                                 argIdx + 1);

                if (loc.kind == ArgLoc::Kind::IntReg) {
                    // clang-format off
                    constexpr const char *const riscv_assembly_move_to_reg =
                        "    mv a%d, t0              # move argument #%d to integer arg register\n";
                    // clang-format on
                    dumpInstructions(m_output_file.get(), riscv_assembly_move_to_reg, loc.index,
                                     argIdx + 1);
                } else {  // stack
                    // clang-format off
                    constexpr const char *const riscv_assembly_store_to_stack =
                        "    sw t0, %d(t3)           # store argument #%d to caller stack slot\n";
                    // clang-format on
                    dumpInstructions(m_output_file.get(), riscv_assembly_store_to_stack,
                                     loc.index * 4, argIdx + 1);
                }
            }
        }

        // Drop the evaluation stack; keep only the outgoing stack arguments (if any).
        // clang-format off
        constexpr const char *const riscv_assembly_finalize_sp =
            "    mv sp, t3\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_finalize_sp);
    }

    // Jump to function & get the return value
    // clang-format off
    constexpr const char *const riscv_assembly_func_invocation =
        "    jal ra, %s      # call function '%s'\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_func_invocation,
                     p_func_invocation.getNameCString(), p_func_invocation.getNameCString());

    if (stackSizeBytes > 0) {
        // clang-format off
        constexpr const char *const riscv_assembly_pop_stack_args =
            "    addi sp, sp, %d         # pop stack arguments\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_pop_stack_args, stackSizeBytes);
    }

    // Push return value to stack
    /// NOTE: Here, even if function returns 'void', we still push 'a0' to stack. And this is okay(according to implementation).
    if (entry->type.isSameType(ScalarType::REAL)) {
        // clang-format off
        constexpr const char *const riscv_assembly_func_invocation =
            //"    mv ft0, fa0        # always move the return value to a certain register you choose\n"
            "    addi sp, sp, -4\n"
            "    fsw fa0, 0(sp)     # push the value to the stack\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_func_invocation);
    } else {
        // clang-format off
        constexpr const char *const riscv_assembly_func_invocation =
            "    mv t0, a0        # always move the return value to a certain register you choose\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)     # push the value to the stack\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_func_invocation);
    }

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_variable_ref.visitChildNodes(*this);

    SymbolEntry *entry = m_symbol_manager.findSymbol(p_variable_ref.getNameCString());
    if (entry == nullptr) {
        perror("p_variable_ref, no symbol found\n");
        exit(1);
    }

    if (entry->type.isSameType(ScalarType::REAL)) {
        // Load value of variable to 'ft0'
        if (entry->level == 0) {
            // Global
            // clang-format off
            constexpr const char *const riscv_assembly_global_var_ref = 
                "    la t0, %s\n"
                "    flw ft0, 0(t0)     # load the value of '%s'\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_global_var_ref,
                             p_variable_ref.getNameCString(), p_variable_ref.getNameCString());
        } else {
            // Local
            // clang-format off
            constexpr const char *const riscv_assembly_local_var_ref = 
                "    flw ft0, %d(s0)        # load the value of '%s'\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_local_var_ref, entry->addrOfLocal,
                             p_variable_ref.getNameCString());
        }
    } else {
        // Load value of variable to 't0'
        if (entry->level == 0) {
            // Global
            // clang-format off
            constexpr const char *const riscv_assembly_global_var_ref = 
                "    la t0, %s\n"
                "    lw t1, 0(t0)     # load the value of '%s'\n"
                "    mv t0, t1\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_global_var_ref,
                             p_variable_ref.getNameCString(), p_variable_ref.getNameCString());
        } else {
            // Local
            // clang-format off
            constexpr const char *const riscv_assembly_local_var_ref = 
                "    lw t0, %d(s0)        # load the value of '%s'\n";
            // clang-format on
            dumpInstructions(m_output_file.get(), riscv_assembly_local_var_ref, entry->addrOfLocal,
                             p_variable_ref.getNameCString());
        }
    }

    // Push to stack
    if (entry->type.isSameType(ScalarType::REAL)) {
        // clang-format off
        constexpr const char *const riscv_assembly_var_ref =
            "    addi sp, sp, -4\n"
            "    fsw ft0, 0(sp)     # push the value to the stack\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_var_ref);
    } else {
        // clang-format off
        constexpr const char *const riscv_assembly_var_ref =
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)     # push the value to the stack\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_var_ref);
    }

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    // (a) lval addr
    // Here, we just gain address of lvalue variable in assignment node, rather than visit child node
    std::string lvalName = p_assignment.getVarRef()->getNameCString();
    const SymbolEntry *const lvalEntry = m_symbol_manager.findSymbol(lvalName.c_str());
    if (lvalEntry == nullptr) {
        perror("lvalEntry not found\n");
        exit(1);
    }

    if (lvalEntry->level == 0) {
        // Global
        // clang-format off
        constexpr const char *const riscv_assembly_global_var_ref = 
            "    la t0, %s\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_global_var_ref, lvalName.c_str());
    } else {
        // Local
        // clang-format off
        constexpr const char *const riscv_assembly_local_var_ref = 
            "    addi t0, s0, %d\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_local_var_ref, lvalEntry->addrOfLocal);
    }
    // clang-format off
    constexpr const char *const riscv_assembly_var_ref = 
        "    addi sp, sp, -4\n"
        "    sw t0, 0(sp)     # push the address to the stack\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_var_ref);

    // (2) expression (rvalue)
    // After visiting expression node subtree, a single value will be pushed onto the stack.
    p_assignment.getExpression()->accept(*this);

    // (3) Assign val to var
    if (lvalEntry->type.isSameType(ScalarType::REAL)) {
        // clang-format off
        constexpr const char *const riscv_assembly_assign = 
            "    flw ft0, 0(sp)     # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    lw t0, 0(sp)     # pop the address from the stack\n"
            "    addi sp, sp, 4\n"
            "    fsw ft0, 0(t0)     # save the value to the address of '%s'\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_assign, lvalName.c_str());
    } else {
        // clang-format off
        constexpr const char *const riscv_assembly_assign = 
            "    lw t0, 0(sp)     # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    lw t1, 0(sp)     # pop the address from the stack\n"
            "    addi sp, sp, 4\n"
            "    sw t0, 0(t1)     # save the value to the address of '%s'\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_assign, lvalName.c_str());
    }

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(ReadNode &p_read) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    // Call read function
    /// NOTE: There is no read string in this hw.
    bool isReal = p_read.getVarRef()->getTypeOfResult().scalarType == ScalarType::REAL;

    // clang-format off
    constexpr const char *const riscv_assembly_read = 
        "    jal ra, %s  # call function '%s'\n";
    // clang-format on
    const char *functionToCall = isReal ? "readReal" : "readInt";
    dumpInstructions(m_output_file.get(), riscv_assembly_read, functionToCall, functionToCall);

    // lvalue
    const SymbolEntry *varRefEntry =
        m_symbol_manager.findSymbol(p_read.getVarRef()->getNameCString());
    if (varRefEntry == nullptr) {
        perror("var ref not found IN read node\n");
        exit(1);
    }
    std::string lvalName = varRefEntry->name;

    if (varRefEntry->level == 0) {
        // Global
        // clang-format off
        constexpr const char *const riscv_assembly_global_var_ref = 
            "    la t0, %s\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_global_var_ref, lvalName.c_str());
    } else {
        // Local
        // clang-format off
        constexpr const char *const riscv_assembly_local_var_ref = 
            "    addi t0, s0, %d\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_local_var_ref,
                         varRefEntry->addrOfLocal);
    }

    if (isReal) {
        // clang-format off
        constexpr const char *const riscv_assembly_read_store = 
            "    fsw fa0, 0(t0)     # save the return value to '%s'\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_read_store, lvalName);
    } else {
        // clang-format off
        constexpr const char *const riscv_assembly_read_store = 
            "    sw a0, 0(t0)     # save the return value to '%s'\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_read_store, lvalName);
    }

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(IfNode &p_if) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_if.getCondition()->accept(*this);

    // [if]:

    bool hasElseBody = p_if.getElseBody();
    // Acquire enough labels
    int firstLabel = getNextL();
    int labelOfNext = firstLabel + 1;
    nextL_add(2);
    if (hasElseBody) {
        nextL_add(1);
        labelOfNext = firstLabel + 2;
    }

    // clang-format off
    constexpr const char *const riscv_assembly_if1 =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    beq t0, x0, .L%d\n"  // jump to else/exit when the condition is false
    // [then]:

        ".L%d:\n";  // not necessary, but spec adds this, so I do, too.
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_if1, firstLabel + 1, firstLabel);

    p_if.getBody()->accept(*this);

    // clang-format off
    constexpr const char *const riscv_assembly_if2 =
        "    j .L%d                  # jump to .L%d\n";  // jump to label NEXT
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_if2, labelOfNext, labelOfNext);

    // [else]:

    if (p_if.getElseBody() != nullptr) {
        // clang-format off
        constexpr const char *const riscv_assembly_if3 =
            ".L%d:\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_if3, firstLabel + 1);

        p_if.getElseBody()->accept(*this);
    }

    // [end if]
    // [NEXT]:

    // clang-format off
    constexpr const char *const riscv_assembly_if4 =
        ".L%d:\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_if4, labelOfNext);

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(WhileNode &p_while) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    // [while]:

    // Acquire enough labels
    int firstLabel = getNextL();
    nextL_add(3);

    // clang-format off
    constexpr const char *const riscv_assembly_while1 =
        ".L%d:\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_while1, firstLabel);

    p_while.getCondition()->accept(*this);

    // clang-format off
    constexpr const char *const riscv_assembly_while2 =
        "    lw t0, 0(sp)     # pop the value from the stack\n"
        "    addi sp, sp, 4\n"
        "    beq t0, x0, .L%d\n"  // exit loop once the condition becomes false
    // [do]:

        ".L%d:\n";  // not necessary, but spec adds this, so I do, too.
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_while2, firstLabel + 2, firstLabel + 1);

    p_while.getBody()->accept(*this);

    // clang-format off
    constexpr const char *const riscv_assembly_while3 =
        "    j .L%d                  # jump to .L%d\n";  // jump to the first label
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_while3, firstLabel, firstLabel);

    // [end do]
    // [NEXT]:

    // clang-format off
    constexpr const char *const riscv_assembly_while4 =
        ".L%d:\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_while4, firstLabel + 2);

    /* Step 4: Pop scope                                        */
    // x
}

void CodeGenerator::visit(ForNode &p_for) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */

    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(std::move(m_symbol_table_of_scoping_nodes.at(&p_for)));

    /* Step 3: Visit child nodes & Ouput assembly               */

    const SymbolEntry *loopVarEntry =
        m_symbol_manager.findSymbol(p_for.getLoopVar()->getNameCString());

    // [for]:

    // Acquire enough labels
    int firstLabel = getNextL();
    nextL_add(3);

    // - Init
    p_for.getInitStmt()->accept(*this);

    // - Condition
    // clang-format off
    constexpr const char *const riscv_assembly_for1 =
        ".L%d:\n"
        "    lw t0, %d(s0)        # load the value of '%s'\n"
        "    li t1, %s\n"
        "    bge t0, t1, .L%d        # if %s >= %s, exit the loop\n"
    // [do]:

        ".L%d:\n";  // not necessary, but spec adds this, so I do, too.
    // clang-format on
    const std::string condition = p_for.getCondition()->getConstVal().getConstValInString();
    dumpInstructions(m_output_file.get(), riscv_assembly_for1, firstLabel,
                     loopVarEntry->addrOfLocal, loopVarEntry->name, condition.c_str(),
                     firstLabel + 2, loopVarEntry->name, condition.c_str(), firstLabel + 1);

    p_for.getBody()->accept(*this);

    // - Routine
    // clang-format off
    constexpr const char *const riscv_assembly_for2 =
        "    lw t0, %d(s0)        # load the value of '%s'\n"
        "    addi t0, t0, 1       # %s + 1, always save the value in a certain register you choose\n"
        "    sw t0, %d(s0)        # save the value to '%s'\n"
        "    j .L%d               # jump back to loop condition\n"
    // [end do]
    // [NEXT]:
    
        ".L%d:\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_for2, loopVarEntry->addrOfLocal,
                     loopVarEntry->name, loopVarEntry->name, loopVarEntry->addrOfLocal,
                     loopVarEntry->name, firstLabel, firstLabel + 2);

    /* Step 4: Pop scope                                        */

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
}

void CodeGenerator::visit(ReturnNode &p_return) {
    /* Step 1: Ouput assembly                                   */
    // x

    /* Step 2: Push scope                                       */
    // x

    /* Step 3: Visit child nodes & Ouput assembly               */

    p_return.visitChildNodes(*this);
    bool isReal = p_return.getReturnVal()->getTypeOfResult().isSameType(ScalarType::REAL);

    if (isReal) {
        // clang-format off
        constexpr const char *const riscv_assembly_return = 
            "    flw fa0, 0(sp)     # pop the value from the stack\n"
            "    addi sp, sp, 4\n";
            //"    mv fa0, ft0\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_return);
    } else {
        // clang-format off
        constexpr const char *const riscv_assembly_return = 
            "    lw t0, 0(sp)     # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    mv a0, t0\n";
        // clang-format on
        dumpInstructions(m_output_file.get(), riscv_assembly_return);
    }

    // clang-format off
    constexpr const char *const riscv_assembly_return = 
        "    # in the function epilogue\n"
        "    lw ra, 124(sp)       # load return address saved in the current stack\n"
        "    lw s0, 120(sp)       # move frame pointer back to the bottom of the last stack\n"
        "    addi sp, sp, 128     # move stack pointer back to the top of the last stack\n"
        "    jr ra                # jump back to the caller function\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_return);

    /* in this hw, there seems to be no prevention from program after a return stmt. */

    /* Step 4: Pop scope                                        */
    // x
}
