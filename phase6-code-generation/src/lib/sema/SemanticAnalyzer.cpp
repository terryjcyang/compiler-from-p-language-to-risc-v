#include "sema/Error.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"
#include <memory>

/* ---- debug ---- */

void debug_print(const char *s) {
    if (0) {
        fprintf(stderr, "%s\n", s);
    }
}

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    debug_print("program");

    /**
     * hw5 mod:
     * since ctor of SymbolManager changes
     */
    /* Step : New symbol table */
    m_symbolManager.pushScope(std::make_unique<SymbolTable>());

    /* Step 0: Check id redeclaration */
    /**
     * If there are multiple declarations with the same identifier in the same scope, 
     * only the first declaration will be placed in the symbol table.
     */
    bool isRedeclared = m_symbolManager.isRedeclared(p_program.getNameCString());
    if (isRedeclared) {
        m_error_printer.print(
            SymbolRedeclarationError(p_program.getLocation(), p_program.getNameCString()));
    }

    /* Step 1: Insert into symbol table */
    if (!isRedeclared) {
        Type t(ScalarType::VOID);
        m_symbolManager.pushEntry(p_program.getNameCString(), KindOfSymbol::PROGRAM, t);
    }
    /**
     * Whether a program/function is redeclared does not affect the return type of this scope
     */
    m_symbolManager.returnTypes.push(ScalarType::VOID);  // in hw, program returns void

    /* Step 2: New symbol table */
    // x
    /**
     * in this hw, program lies in the same scope as other global symbol
     */

    /* Step 3: Traverse child nodes */
    p_program.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    /**
     * 1.
     * There is only one semantic analysis that should be performed in pre-order, 
     * that is symbol redeclaration. 
     * Except for symbol redeclaration, 
     * all other semantic analyses are performed in post-order.
     * 
     * 2.
     * When your parser encounters a semantic error, 
     * the remaining semantic checks of the node should then be skipped. 
     *      (func decl is the only exception)
     */
    // x

    /* Step 5: Pop the symbol table */
    /**
     * hw5 mod:
     */
    m_symbol_table_of_scoping_nodes[&p_program] = m_symbolManager.popScope();
    m_symbolManager.returnTypes.pop();
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    debug_print("decl");
    p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    debug_print("var");
    /* Step 0: Check id redeclaration */
    bool isRedeclared = m_symbolManager.isRedeclared(p_variable.getNameCString());
    if (isRedeclared) {
        m_error_printer.print(
            SymbolRedeclarationError(p_variable.getLocation(), p_variable.getNameCString()));
    }

    /* Step 1: Insert into symbol table */
    //      case 1: loop var
    if (!isRedeclared && m_symbolManager.inLoopInit) {
        m_symbolManager.pushEntry(p_variable.getNameCString(), KindOfSymbol::LOOP_VAR,
                                  p_variable.getType());
    }
    //      case 2: parameter
    if (!isRedeclared && m_symbolManager.upperIsFunction) {
        // This variable is a parameter
        m_symbolManager.pushEntry(p_variable.getNameCString(), KindOfSymbol::PARAMETER,
                                  p_variable.getType());
    }
    //      case 3: variable/constant
    if (!isRedeclared && !m_symbolManager.inLoopInit && !m_symbolManager.upperIsFunction) {
        ConstantValueNode *constValNode_ptr = p_variable.getConstValueNode();
        if (constValNode_ptr) {
            m_symbolManager.pushEntry(p_variable.getNameCString(), KindOfSymbol::CONSTANT,
                                      p_variable.getType(), constValNode_ptr->getConstVal());
        } else {
            m_symbolManager.pushEntry(p_variable.getNameCString(), KindOfSymbol::VARIABLE,
                                      p_variable.getType());
        }
    }

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_variable.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    /**
     * In an array declaration, each dimension's size has to be greater than 0.
     */
    if (!isRedeclared) {
        Type t = p_variable.getType();
        for (int &dim : t.arrRefs) {
            if (dim <= 0) {
                // the location is the variable's location
                m_error_printer.print(NonPositiveArrayDimensionError(p_variable.getLocation(),
                                                                     p_variable.getNameCString()));
                m_symbolManager.setCurrEntryDeclErr();
                break;
            }
        }
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    debug_print("const");
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_constant_value.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    // x

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    debug_print("func");
    /* Step 0: Check id redeclaration */
    bool isRedeclared = m_symbolManager.isRedeclared(p_function.getNameCString());
    if (isRedeclared) {
        m_error_printer.print(
            SymbolRedeclarationError(p_function.getLocation(), p_function.getNameCString()));
    }

    /* Step 1: Insert into symbol table */
    if (!isRedeclared) {
        Type t(p_function.getReturnType());
        // parameters (to form attribute,
        // but not indep. entries which will be inserted when visiting variableNodes)
        std::vector<Type> paramTypes;
        for (auto &param : p_function.getParameters()) {
            for (auto &var : param->getVariables()) {
                paramTypes.emplace_back(var->getType());
            }
        }
        m_symbolManager.pushEntry(p_function.getNameCString(), KindOfSymbol::FUNCTION, t,
                                  paramTypes);
    }

    /* Step 2: New symbol table */
    m_symbolManager.pushScope(std::make_unique<SymbolTable>());
    m_symbolManager.upperIsFunction = true;
    /**
     * Whether a program/function is redeclared does not affect the return type of this scope
     */
    m_symbolManager.returnTypes.push(p_function.getReturnType());
    m_symbolManager.listOfAddrOfNextLocal.emplace_back(-12);  // s0 - 12 is the addr of first var

    /* Step 3: Traverse child nodes */
    p_function.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    /**
     * If the function name causes a redeclaration error, 
     * the remaining semantic checks should still be performed.
     * (This is the only exception.)
     * 
     * When the function name is redeclared, the ONLY skipped thing is inserting the function into the symbol table.
     * Everything else in the function – parameters and full body semantic analysis – still runs normally.
     */
    // x

    /* Step 5: Pop the symbol table(in step 2) */
    /**
     * hw5 mod:
     * store table to m_symbol_table_of_scoping_nodes 
     * for codegen
     */
    m_symbol_table_of_scoping_nodes[&p_function] = m_symbolManager.popScope();
    m_symbolManager.upperIsFunction = false;
    m_symbolManager.returnTypes.pop();
    m_symbolManager.listOfAddrOfNextLocal.pop_back();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    debug_print("cp-stmt");
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */

    if (m_symbolManager.currlvl == 0) {
        // this compound stmt is main() (serve as main() in C language)
        m_symbolManager.listOfAddrOfNextLocal.emplace_back(
            -12);  // s0 - 12 is the addr of first var
    }

    const bool upperIsFunction = m_symbolManager.upperIsFunction;
    if (upperIsFunction) {
        m_symbolManager.upperIsFunction = false;
    } else {
        m_symbolManager.pushScope(std::make_unique<SymbolTable>());
    }

    /* Step 3: Traverse child nodes */
    p_compound_statement.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    // x

    /* Step 5: Pop the symbol table(in step 2) */
    if (upperIsFunction) {
        m_symbolManager.upperIsFunction = true;
    } else {
        /**
     * hw5 mod:
     */
        m_symbol_table_of_scoping_nodes[&p_compound_statement] = m_symbolManager.popScope();
        if (m_symbolManager.currlvl == 0) {
            // this compound stmt is main() (serve as main() in C language)
            m_symbolManager.listOfAddrOfNextLocal.pop_back();
        }
    }
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_print.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    // Expr to print must be of scalar type
    auto expr = p_print.getExpression();
    Type type = expr->getTypeOfResult();
    bool isCorrupted = (type.scalarType == ScalarType::UNKNOWN);
    /**
     * Note:
     * scalar type does not include "VOID"
     */
    bool isScalarType = type.arrRefs.empty() && (type.scalarType != ScalarType::VOID);
    if (!isCorrupted &&
        !isScalarType) {  // (Skip the rest of semantic checks if there are any errors in the node of the expression(target))
        m_error_printer.print(PrintOutNonScalarTypeError(expr->getLocation()));
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_bin_op.visitChildNodes(*this);

    /* Step: Determine expr result type of b_op */
    /**
     * After types of child nodes have finalized,
     * the type of b_op can be determined,
     */
    p_bin_op.determineTypeOfResult();

    /* Step 4: Semantic analyses (of this node) */
    bool thisNodeHasErr = p_bin_op.isUnknownType();
    if (thisNodeHasErr)
        debug_print("thisNodeHasErr");
    bool leftOperandHasErr = p_bin_op.getLeftOperand()->isUnknownType();
    if (leftOperandHasErr)
        debug_print("leftNodeHasErr");
    bool rightOperandHasErr = p_bin_op.getRightOperand()->isUnknownType();
    if (rightOperandHasErr)
        debug_print("rightNodeHasErr");
    if (!(leftOperandHasErr || rightOperandHasErr) && thisNodeHasErr) {
        // Note: (Skip the rest of semantic checks if there are any errors in the nodes of operands)
        m_error_printer.print(
            InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.getOperator(),
                                      p_bin_op.getLeftOperand()->getTypeOfResult(),
                                      p_bin_op.getRightOperand()->getTypeOfResult()));
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_un_op.visitChildNodes(*this);

    /* Step: Determine expr result type of b_op */
    /**
     * After types of child nodes have finalized,
     * the type of b_op can be determined,
     */
    p_un_op.determineTypeOfResult();

    /* Step 4: Semantic analyses (of this node) */
    bool thisNodeHasErr = p_un_op.isUnknownType();
    if (thisNodeHasErr)
        debug_print("thisNodeHasErr");
    bool operandHasErr = p_un_op.getOperand()->isUnknownType();
    if (operandHasErr)
        debug_print("leftNodeHasErr");

    if (!operandHasErr && thisNodeHasErr) {
        // Note: (Skip the rest of semantic checks if there are any errors in the nodes of operands)
        m_error_printer.print(InvalidUnaryOperandError(p_un_op.getLocation(), p_un_op.getOperator(),
                                                       p_un_op.getOperand()->getTypeOfResult()));
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_func_invocation.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    bool hasErrInThisNode = false;
    // (a) in symbol table
    SymbolEntry *entryOfFunction = m_symbolManager.findSymbol(p_func_invocation.getNameCString());
    if (entryOfFunction) {
        // (b) kind
        if (entryOfFunction->kind != KindOfSymbol::FUNCTION) {
            m_error_printer.print(NonFunctionSymbolError(p_func_invocation.getLocation(),
                                                         p_func_invocation.getNameCString()));
            hasErrInThisNode = true;  ///
        } else {
            // (c) # of arguments(func invo) must be the same as # of the parameters(func decl).
            auto &arguments = p_func_invocation.getArguments();
            std::vector<Type> &typesOfParam = entryOfFunction->attribute.typesOfFormalParam;
            if (arguments.size() != typesOfParam.size()) {
                m_error_printer.print(ArgumentNumberMismatchError(
                    p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
                hasErrInThisNode = true;  ///
            }

            // (d) The type of the result of the expression (argument) must be the same type
            //     of the corresponding parameter after appropriate type coercion.
            // if (!hasErrInThisNode) {
            //     for (int i = 0; i < typesOfParam.size(); i++) {
            //         Type typeOfArg = arguments[i]->getTypeOfResult();
            //         // (Skip the rest of semantic checks if there are any errors in the node of the expression (argument))
            //         if (typeOfArg.scalarType == ScalarType::UNKNOWN) {
            //             hasErrInThisNode = true;
            //             break;
            //         }
            //     }
            // }
            if (!hasErrInThisNode) {
                for (int i = 0; i < typesOfParam.size(); i++) {
                    // (d) The type of the result of the expression (argument) must be the same type
                    //     of the corresponding parameter after appropriate type coercion.
                    Type typeOfArg = arguments[i]->getTypeOfResult();
                    typesOfParam[i].isSameType(typeOfArg);

                    // (Skip the rest of semantic checks if there are any errors in the node of the expression (argument))
                    if (typeOfArg.scalarType == ScalarType::UNKNOWN) {
                        hasErrInThisNode = true;
                        // break;
                        continue;
                    }

                    bool sameType = typesOfParam[i].isSameType(typeOfArg);
                    /**
                     * Integer type can be implicitly converted into the real type in several situations: 
                     * assignment, argument passing, arithmetic operation, relational operation, or return statement.
                     * 
                     * only the following condition will return true:
                     * should not be array and one INT, one REAL
                     */
                    bool canCoerce = false;
                    if (!sameType) {
                        bool bothVar = typesOfParam[i].arrRefs.empty() && typeOfArg.arrRefs.empty();
                        canCoerce = bothVar && (typesOfParam[i].scalarType == ScalarType::REAL &&
                                                typeOfArg.scalarType == ScalarType::INTEGER);
                    }
                    bool typeIncompatible = !sameType && !canCoerce;
                    if (typeIncompatible) {
                        m_error_printer.print(IncompatibleArgumentTypeError(
                            arguments[i]->getLocation(), typesOfParam[i], typeOfArg));
                        hasErrInThisNode = true;  ///
                        // break;
                    }
                }
            }
        }

    } else {
        m_error_printer.print(UndeclaredSymbolError(p_func_invocation.getLocation(),
                                                    p_func_invocation.getNameCString()));
        hasErrInThisNode = true;  ///
    }

    /* Step: fill the Type of var_ref_node (an Expression) */
    //fprintf(stderr, "s type: %s", ScalarTypeStrings[(int)entryOfFunction->type.scalarType]);
    if (hasErrInThisNode) {
        /**
         * From test case -- AdvancedSema,
         * we know if there is any error in func_invo node,
         * the type of result (return type of function) will have no need to examine in upper layer.
         */
        p_func_invocation.setTypeOfResult(Type(ScalarType::UNKNOWN));
    } else {
        p_func_invocation.setTypeOfResult(entryOfFunction->type);
    }
    //fprintf(stderr, "after-s type: %s", ScalarTypeStrings[(int)p_func_invocation.getTypeOfResult().scalarType]);

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_variable_ref.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    bool hasErrInThisNode = false;
    // (a) in symbol table
    SymbolEntry *entryOfVarDecl = m_symbolManager.findSymbol(p_variable_ref.getNameCString());
    if (entryOfVarDecl) {
        // (b) kind
        if (entryOfVarDecl->kind == KindOfSymbol::PROGRAM ||
            entryOfVarDecl->kind == KindOfSymbol::FUNCTION) {
            m_error_printer.print(NonVariableSymbolError(p_variable_ref.getLocation(),
                                                         p_variable_ref.getNameCString()));
            hasErrInThisNode = true;            ///
        } else if (!entryOfVarDecl->declErr) {  // skip if referred symbol has decl error
            // (c) Each index of an array reference must be of the integer type.
            //   (Bound checking is not performed at compile.)
            auto &indices = p_variable_ref.getIndices();
            for (auto &expressNode : indices) {
                Type typeOfIdx = expressNode->getTypeOfResult();
                bool exprHasCorrupted = (typeOfIdx.scalarType == ScalarType::UNKNOWN);
                // Skip further check if any expr child node has error
                if (exprHasCorrupted) {
                    hasErrInThisNode = true;
                    break;
                }
                // Note: index that is of type "int [n1][n2]..." is not integer type.
                bool isIntegerType =
                    (typeOfIdx.scalarType == ScalarType::INTEGER) && typeOfIdx.arrRefs.empty();
                if (!isIntegerType) {
                    m_error_printer.print(NonIntegerArrayIndexError(expressNode->getLocation()));
                    hasErrInThisNode = true;  ///
                    break;
                }
            }

            // (d) indices of arr ref should <= decl dim
            if (!hasErrInThisNode && indices.size() > entryOfVarDecl->type.arrRefs.size()) {
                m_error_printer.print(OverArraySubscriptError(p_variable_ref.getLocation(),
                                                              p_variable_ref.getNameCString()));
                hasErrInThisNode = true;  ///
            }
        } else {
            hasErrInThisNode = true;
        }
    } else {
        m_error_printer.print(
            UndeclaredSymbolError(p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
        hasErrInThisNode = true;  ///
    }

    /* Step: fill the Type of var_ref_node (an Expression) */
    if (hasErrInThisNode) {
        p_variable_ref.setTypeOfResult(Type(ScalarType::UNKNOWN));
    } else {
        /**
         * Example:
         * - Decl part
         * var vRealArr: array 10 of array 100 of array 2 of real;
         *      entryOfVarDecl->type.scalarType:        REAL
         *      entryOfVarDecl->type.arrRefs:           {10, 100, 2}
         * 
         * - Reference part
         * vRealArr[3][7];
         *      p_variable_ref.getIndices():            {3, 7}
         *      p_variable_ref.getIndices().size():     2
         *      t.scalarType:                           REAL
         *      t.arrRefs:                              {2}
         * 
         * - Finally the type of var_ref is:
         * array 2 of real
         */
        Type t(entryOfVarDecl->type);
        int numOfDimToRemove = p_variable_ref.getIndices().size();
        t.arrRefs.erase(t.arrRefs.begin(), t.arrRefs.begin() + numOfDimToRemove);
        p_variable_ref.setTypeOfResult(t);
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_assignment.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    auto lVal = p_assignment.getVarRef();
    Type lValType = lVal->getTypeOfResult();
    bool lValIsCorrupted = (lValType.scalarType == ScalarType::UNKNOWN);

    auto expr = p_assignment.getExpression();
    Type exprType = expr->getTypeOfResult();

    // (Skip the rest of semantic checks if there are any errors in the node of the variable reference (lvalue))
    if (!lValIsCorrupted) {
        // Here, we simply find the var decl again.
        SymbolEntry *entryOfVarDecl = m_symbolManager.findSymbol(lVal->getNameCString());

        // (a) The type of the result of the variable reference cannot be an array type.
        if (!lValType.arrRefs.empty()) {
            m_error_printer.print(AssignToArrayTypeError(lVal->getLocation()));
        }
        // (b) The variable reference cannot be a reference to a constant variable.
        else if (entryOfVarDecl->kind == KindOfSymbol::CONSTANT) {
            m_error_printer.print(
                AssignToConstantError(lVal->getLocation(), lVal->getNameCString()));
        }
        // (c) The variable reference cannot be a reference to a loop variable,
        //     unless the current context is for loop_var initialization.
        else if (entryOfVarDecl->kind == KindOfSymbol::LOOP_VAR && !m_symbolManager.inLoopInit) {
            m_error_printer.print(AssignToLoopVarError(lVal->getLocation()));
        }
        /* then Check expression */
        else if (exprType.scalarType != ScalarType::UNKNOWN) {  // expr node did not corrupt
            // (d) The type of the result of the expression cannot be an array type.
            if (!exprType.arrRefs.empty()) {
                m_error_printer.print(AssignByArrayTypeError(expr->getLocation()));
            }
            // (e) The type of the variable reference (lvalue) must be the same as the one of the expression after appropriate type coercion.
            else if (!lValType.isSameType(exprType) &&
                     !(lValType.scalarType == ScalarType::REAL &&
                       exprType.scalarType == ScalarType::INTEGER)) {
                m_error_printer.print(
                    IncompatibleAssignmentError(p_assignment.getLocation(), lValType, exprType));
            }
        }
    }
    m_symbolManager.inLoopInit = false;

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_read.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    // (a) The type of the variable reference must be scalar type.
    auto varRef = p_read.getVarRef();
    Type type = varRef->getTypeOfResult();
    bool isCorrupted = (type.scalarType == ScalarType::UNKNOWN);
    bool isScalarType = type.arrRefs.empty();
    if (!isCorrupted &&
        !isScalarType) {  // (Skip the rest of semantic checks if there are any errors in the node of the variable reference)
        m_error_printer.print(ReadToNonScalarTypeError(varRef->getLocation()));
    }

    // (b) The kind of symbol of the variable reference cannot be constant or loop_var.
    if (!isCorrupted) {
        // Here, we simply find the var decl again.
        SymbolEntry *entryOfVarDecl = m_symbolManager.findSymbol(varRef->getNameCString());
        KindOfSymbol kindOfVarRef = entryOfVarDecl->kind;
        if (kindOfVarRef == KindOfSymbol::CONSTANT || kindOfVarRef == KindOfSymbol::LOOP_VAR) {
            m_error_printer.print(ReadToConstantOrLoopVarError(varRef->getLocation()));
        }
    }
    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(IfNode &p_if) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_if.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    auto condition = p_if.getCondition();
    Type condType = condition->getTypeOfResult();

    // (Skip the rest of semantic checks if there are any errors in the node of the expression (condition))
    if (condType.scalarType != ScalarType::UNKNOWN) {
        // The type of the result of the expression (condition) must be boolean type.
        if (!condType.isSameType(Type(ScalarType::BOOLEAN))) {
            m_error_printer.print(NonBooleanConditionError(condition->getLocation()));
        }
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_while.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    /**
     * Same as that of ifNode
     */
    auto condition = p_while.getCondition();
    Type condType = condition->getTypeOfResult();

    // (Skip the rest of semantic checks if there are any errors in the node of the expression (condition))
    if (condType.scalarType != ScalarType::UNKNOWN) {
        // The type of the result of the expression (condition) must be boolean type.
        if (!condType.isSameType(Type(ScalarType::BOOLEAN))) {
            m_error_printer.print(NonBooleanConditionError(condition->getLocation()));
        }
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    /**
     */
    m_symbolManager.pushScope(std::make_unique<SymbolTable>());
    m_symbolManager.inLoopInit = true;

    /* Step 3: Traverse child nodes */
    p_for.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    // The initial value of the loop variable (lower bound) and the constant value of the condition (upper bound)
    // must be in the "strictly" incremental order.
    /**
     * Note that
     * The grammar(CFG) has guaranteed that the init value and condition value are integer.
     */
    auto initConstVal = p_for.getInitConstVal()->getConstVal();
    auto conditionConstVal = p_for.getCondition()->getConstVal();
    if (initConstVal.valContainer.integer >= conditionConstVal.valContainer.integer) {
        m_error_printer.print(NonIncrementalLoopVariableError(p_for.getLocation()));
    }

    /* Step 5: Pop the symbol table(in step 2) */
    /**
     * hw5 mod:
     */
    m_symbol_table_of_scoping_nodes[&p_for] = m_symbolManager.popScope();
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    /* Step 1: Insert into symbol table */
    // x

    /* Step 2: New symbol table */
    // x

    /* Step 3: Traverse child nodes */
    p_return.visitChildNodes(*this);

    /* Step 4: Semantic analyses (of this node) */
    ScalarType expectedReturnType = m_symbolManager.returnTypes.top();
    auto returnVal = p_return.getReturnVal();
    Type returnType = returnVal->getTypeOfResult();
    bool isCorrupted = (returnType.scalarType == ScalarType::UNKNOWN);

    // (a) The current context shouldn't be in the program or a procedure since their return type is void.
    /**
     * Recall in hw2 README(spec),
     *      A function that returns no value can be called as a "procedure".
     * 
     * "returns no value" = "VOID" return type
     */
    if (expectedReturnType == ScalarType::VOID) {
        m_error_printer.print(ReturnFromVoidError(p_return.getLocation()));
    }
    // (Skip the rest of semantic checks if there are any errors in the node of the expression (return value))
    else if (!isCorrupted) {
        // (b) The type of the result of the expression (return value) must be
        //     the same type as the return type of current function after appropriate type coercion.
        bool sameType = returnType.isSameType(Type(expectedReturnType));
        bool canCoerce =
            (expectedReturnType == ScalarType::REAL) &&
            (returnType.scalarType == ScalarType::INTEGER && returnType.arrRefs.empty());
        if (!sameType && !canCoerce) {
            m_error_printer.print(IncompatibleReturnTypeError(
                returnVal->getLocation(), Type(expectedReturnType), returnType));
        }
    }

    /* Step 5: Pop the symbol table(in step 2) */
    // x
}
