/* Declarations (optional) */

    /* C declarations and includes */
%{
#include "AST/AstDumper.hpp"    //      /// visitor pattern
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"          //  base class: ast node
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"
#include "util/astHelperTypes.hpp"      /// defined by me

#include "sema/SemanticAnalyzer.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#define YYLTYPE yyltype

/**
 * Bison provides a way to keep track of the textual locations of tokens and groupings.
 *  Defined by providing a data type, and actions to take when rules are matched.
 */
typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern uint32_t line_num;   /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

    /* Yacc definitions */
// This guarantees that headers do not conflict when included together.
%define api.token.prefix {TOK_}

/**
 * E.g., if you need class `ProgramNode` in the `%union`,
 * either   use forward declaration 
 * or       `#include` directive in the block of `%code requires`.
 */
%code requires {
    #include <vector>

    /* Type to store data in AST node */
    struct Type;      // in variable.hpp

    /* Helper types defined by me */
    #include "util/astHelperTypes.hpp"

    /* Node classes */
    class AstNode;
    class ExpressionNode;

    // class ProgramNode;
    class DeclNode;
    // class VariableNode;
    class ConstantValueNode;
    class FunctionNode;
    class CompoundStatementNode;
    class PrintNode;
    class BinaryOperatorNode;
    class UnaryOperatorNode;
    class FunctionInvocationNode;
    class VariableReferenceNode;
    class AssignmentNode;
    class ReadNode;
    class IfNode;
    class WhileNode;
    class ForNode;
    class ReturnNode;
}

/* Declare the possible data types of semantic values */
    /* For yylval */
%union {
    /**
     * for C++ non-POD types, you should use pointer to non-POD. (POD: Plain Old Data)
     * e.g. (vector< > *)
     */
    /* Basic semantic value */
    char                            *str_type;
    int32_t                         int_type;
    double                          float_type;
    bool                            bool_type;

    /* Self-defined types */
    ScalarType                      scalar_type_type;       // enum class (defined in ast.hpp)
    Type                            *type_type;             // struct (defined in ast.hpp)
    IdList                          *id_list_type;          //  (defined in util/astHelperTypes.hpp)

    /* Pointer of node */
    AstNode                         *ast_node_ptr;
    ExpressionNode                  *expr_ptr;

    DeclNode                        *decl_ptr;
    // VariableNode                    *var_ptr;
    ConstantValueNode               *const_val_ptr;
    FunctionNode                    *func_ptr;
    CompoundStatementNode           *compound_stmt_ptr;
    PrintNode                       *print_ptr;
    BinaryOperatorNode              *binary_op_ptr;
    UnaryOperatorNode               *unary_op_ptr;
    FunctionInvocationNode          *func_invocation_ptr;
    VariableReferenceNode *var_ref_ptr;
    AssignmentNode *assign_ptr;
    ReadNode *read_ptr;
    IfNode *if_ptr;
    WhileNode *while_ptr;
    ForNode *for_ptr;
    ReturnNode *return_ptr;

    /* Pointer of vector of node */
    std::vector<AstNode *>          *ast_node_list_ptr;
    std::vector<ExpressionNode *>   *expr_list_ptr;

    std::vector<DeclNode *>         *decl_list_ptr;     // for declarations and also parameters (parameterList)
    std::vector<FunctionNode *>     *func_list_ptr;
};

/* Terminals (token type names) */

/* delimiters */

/* operators */
/**
 * Note:
 * NEGATION and SUBTRACTION are pseudo tokens (not returned by scanner).
 * They represent the precedence of '-' in its two roles:
 *   - unary NEGATION (e.g., -a)
 *   - binary SUBTRACTION (e.g., a - b)
 * The scanner always returns '-', and the parser decides which meaning applies.
 */
%token NEGATION SUBTRACTION ASSIGN NOT MOD
%token LESS_THAN_OR_EQUAL NOT_EQUAL GREATER_THAN_OR_EQUAL AND OR

/* reserved words */
/* add KW_ prefix to macro name */
%token KW_VAR
%token KW_ARRAY KW_OF KW_BOOLEAN KW_INTEGER KW_REAL KW_STRING
%token KW_TRUE KW_FALSE
%token KW_WHILE KW_DO KW_IF KW_THEN KW_ELSE KW_FOR KW_TO
%token KW_BEGIN KW_END
%token KW_PRINT KW_READ KW_RETURN

/* numeric */
%token <int_type>       DECIMAL_INT OCTAL_INT
%token <float_type>     FLOAT_CONST SCIENTIFIC_NOTATION

/* string */
%token <str_type>       STRING_CONST

/* identifier */
%token <str_type>       IDENTIFIER

/* Associativity & Precedence */
/* Note: 
 * 1. Tokens declared on the same %left / %right / %nonassoc(no associativity) line
 *    share the same precedence and associativity.
 * 2. Associativity is for terminals (especially for operators)
 * 3. Listed in increasing precedence (lowest first)
 */
%left AND OR NOT
%left '<' LESS_THAN_OR_EQUAL NOT_EQUAL GREATER_THAN_OR_EQUAL '>' '='
/* 
 * IMPORTANT:
 * The token '-' MUST appear in a %left/%right precedence declaration.
 *
 * Reason:
 *   - %prec NEGATION and %prec SUBTRACTION only set the precedence of the
 *     *grammar rules*.
 *   - For Bison to apply precedence in shift/reduce conflicts, the lookahead
 *     token itself must also have a precedence.
 *   - If '-' is not listed in any %left/%right/%nonassoc line, it has NO
 *     precedence. In that case Bison cannot compare prec(rule) vs prec('-'),
 *     so it falls back to the default behavior (SHIFT), which produces the
 *     wrong parse for expressions like "-9-10" (parsed as -(9-10)).
 *
 * Summary:
 *   Put '-' in a precedence declaration so Bison assigns it a precedence.
 *   This enables correct precedence resolution between unary '-' and binary '-'.
 */
%left '-'
%left '+'
%left '/' MOD
%left '*'
%left NEGATION

/**
 * By default, start symbol is the head of first rule
 * but you can define manually: 
 * $start S
 * here, $start program
 */

/**
 * Non-terminals 
 * Declare the type of semantic values for a non-terminal symbol
 * here, all plural non-terminals could -> ε 
 */

/* program units */
%type program
%type <func_ptr>            function function_declaration function_definition function_header
%type <decl_ptr>            formal_argument

%type <func_list_ptr>       functions 
%type <decl_list_ptr>       parameters non_empty_parameters     /* those not specified in spec (so i use a line to separate) */

/* declarations */
%type <decl_ptr>            declaration
%type <decl_ptr>            variable_declaration
%type <decl_ptr>            constant_declaration
%type <id_list_type>        identifier_list

%type <decl_list_ptr>       declarations
%type <id_list_type>        non_empty_identifier_list

/* types */
%type <type_type>           type
%type <scalar_type_type>    scalar_type
%type <type_type>           array_type

/* statements */
%type <ast_node_ptr>        statement
%type <ast_node_ptr>        simple_statement 
%type <assign_ptr>          assignment 
%type <print_ptr>           print_statement
%type <read_ptr>            read_statement
%type <if_ptr>              conditional_statement 
%type <ast_node_ptr>        function_call_statement 
%type <ast_node_ptr>        loop_statement 
%type <while_ptr>           while_statement 
%type <for_ptr>             for_statement
%type <return_ptr>          return_statement
%type <compound_stmt_ptr>   compound_statement

%type <ast_node_list_ptr>   statements

/* expressions */
%type <expr_ptr>            expression
%type <const_val_ptr>       literal_constant
%type <const_val_ptr>       integer_literal
%type <const_val_ptr>       real_literal
%type <const_val_ptr>       string_literal
%type <const_val_ptr>       boolean_literal
%type <var_ref_ptr>         variable_reference array_reference
%type <func_invocation_ptr> function_call
%type <binary_op_ptr>       binary_operation
%type <unary_op_ptr>        unary_operation

%type <expr_list_ptr>       expressions non_empty_expressions


%%
/* Grammar rules (required) */
/** 
 * Conflict handling in Yacc/Bison
 *
 * Unless instructed, YACC will resolve all conflicts automatically.
 * So be careful whether you write an ambiguous grammar.
 *
 * Default (when no precedence applies):
 *  - shift/reduce:   choose SHIFT
 *  - reduce/reduce:  choose the production LISTED FIRST
 *
 * Operator precedence & associativity (from %left / %right / %nonassoc):
 *  - Precedence lines are listed LOWEST -> HIGHEST. (Same line, same precedence.)
 *  - Associativity applies to TERMINALS only.
 *  - They do NOT affect REDUCE/REDUCE conflicts.
 *  - A rule’s precedence is:
 *      1. precedence of the TOKEN named by %prec on that rule, i.e, %prec TOKEN
 *      2. precedence of its RIGHTMOST "TERMINAL"
 *      Other conditions: not defined.
 *
 * During a shift/reduce conflict with lookahead token T:
 *  1) If rule-precedence  <  T-precedence,  SHIFT (bind tighter lookahead).
 *  2) If rule-precedence  >  T-precedence,  REDUCE (finish stronger rule).
 *  3) If equal precedence, check associativity:
 *       -- %left      -> REDUCE
 *       -- %right     -> SHIFT
 *       -- %nonassoc  -> SYNTAX ERROR (disallow chaining at this level)
 *
 * Tips:
 *  - Precedence only helps if the rule ends in a TERMINAL (or uses %prec).
 */


/* program units */
program
    : IDENTIFIER ';' declarations functions compound_statement KW_END
    {
        root = new ProgramNode(@1.first_line, @1.first_column, $1/* id */, $3/* decl's */, $4/* func's */, $5/* CP_stmt */);
        free($1);   // since it is (char *)
        delete $3;
        delete $4;
    }
    ;

function
    : function_declaration
    {
        $$ = $1;        // "$$ = $1;" is the default action (if you don't write action in a production).
    }
    | function_definition
    {
        $$ = $1;
    }
    ;
function_header
    : IDENTIFIER '(' parameters ')' ':' scalar_type
    {
        $$ = new FunctionNode(@1.first_line, @1.first_column, $1, $3, $6);
        delete $3;
    }
    | IDENTIFIER '(' parameters ')'
    {
        $$ = new FunctionNode(@1.first_line, @1.first_column, $1, $3);
        delete $3;
    }
    ;
function_declaration
    : function_header ';'
    {
        $$ = $1;
    }
    ;
function_definition
    : function_header compound_statement KW_END
    {
        $$ = $1;
        $$->setCompoundStatement($2);
    }
    ;
formal_argument
    : identifier_list ':' type
    {
        $$ =  new DeclNode(@1.first_line, @1.first_column, $1/* IdList * */, $3);
    }
    ;

functions
    : functions function
    {
        $$ = $1;
        $$->emplace_back($2);
    }
    | /* empty */
    {
        $$ = new std::vector<FunctionNode *>();
    }
    ;
parameters
    : non_empty_parameters
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = new std::vector<DeclNode *>();
    }
    ;
non_empty_parameters
    : formal_argument
    {
        $$ = new std::vector<DeclNode *>();
        $$->emplace_back($1);
    }
    | non_empty_parameters ';' formal_argument
    {
        $$ = $1;
        $$->emplace_back($3);
    }
    ;

/* declarations */
declaration
    : variable_declaration
    {
        $$ = $1;
    }
    | constant_declaration
    {
        $$ = $1;
    }
    ;

variable_declaration
    : KW_VAR identifier_list ':' type ';'
    {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, $4);
        delete $2;
    }
    ;

constant_declaration
    : KW_VAR identifier_list ':' integer_literal ';'
    {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, true, $4);
        delete $2;
    }
    | KW_VAR identifier_list ':' '-' integer_literal ';'
    {
        /**
         * Note: the location.col of ConstValNode should be 
         *      1 more in front of `integer_literal`'s original col
        */
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, false, $5);
        delete $2;
    }
    | KW_VAR identifier_list ':' real_literal ';'
    {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, true, $4);
        delete $2;
    }
    | KW_VAR identifier_list ':' '-' real_literal ';'
    {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, false, $5);
        delete $2;
    }
    | KW_VAR identifier_list ':' string_literal ';'
    {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, $4);
        delete $2;
    }
    | KW_VAR identifier_list ':' boolean_literal ';'
    {
        $$ = new DeclNode(@1.first_line, @1.first_column, $2, $4);
        delete $2;
    }
    ;
identifier_list
    : non_empty_identifier_list
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = new IdList();
    }
    ;

declarations
    : declarations declaration
    {
        $$ = $1;
        $$->emplace_back($2);
    }
    | /* empty */
    {
        $$ = new std::vector<DeclNode *>();
    }
    ;
non_empty_identifier_list
    : IDENTIFIER
    {
        $$ = new IdList();
        $$->emplace_back(@1.first_line, @1.first_column, $1);
        free($1);
    }
    | non_empty_identifier_list ',' IDENTIFIER
    {
        $$ = $1;
        $$->emplace_back(@3.first_line, @3.first_column, $3);
        free($3);
    }
    ;

/* types */
type
    : scalar_type
    {
        $$ = new Type($1);
    }
    | array_type
    {
        $$ = $1;
    }
    ;

scalar_type
    : KW_INTEGER
    {
        $$ = ScalarType::INTEGER;
    }
    | KW_REAL
    {
        $$ = ScalarType::REAL;
    }
    | KW_STRING
    {
        $$ = ScalarType::STRING;
    }
    | KW_BOOLEAN
    {
        $$ = ScalarType::BOOLEAN;
    }
    ;

array_type
    : KW_ARRAY integer_literal KW_OF type
    {
        $$ = $4/* type */;
        /**
         * Bug:
         * $2 should be int32_t
         * but it already is a ConstValNode *
         * 
         * dilemma: Const Decl needs ConstValNode *
         * 
         * (need seriously & thoroughly think about situation)
         */
        $$->add_outer_arr(std::stoi($2->getConstVal().getConstValInString()));
        delete $2;
    }
    ;

/* statements */
statement
    : simple_statement
    {
        $$ = $1;    // upcasting (by dynamic cast
    }
    | conditional_statement
    {
        $$ = $1;
    }
    | function_call_statement
    {
        $$ = $1;
    }
    | loop_statement
    {
        $$ = $1;
    }
    | return_statement
    {
        $$ = $1;
    }
    | compound_statement
    {
        $$ = $1;    // upcasting (by dynamic cast
    }
    ;

simple_statement
    : assignment
    {
        $$ = $1;
    }
    | print_statement
    {
        $$ = $1;    // upcasting
    }
    | read_statement
    {
        $$ = $1;
    }
    ;
assignment
    : variable_reference ASSIGN expression ';'
    {
        $$ = new AssignmentNode(@2.first_line, @2.first_column, $1, $3);
    }
    ;
print_statement
    : KW_PRINT expression ';'
    {
        $$ = new PrintNode(@1.first_line, @1.first_column, $2);
    }
    ;
read_statement
    : KW_READ variable_reference ';'
    {
        $$ = new ReadNode(@1.first_line, @1.first_column, $2);
    }
    ;

conditional_statement
    : KW_IF expression KW_THEN compound_statement KW_ELSE compound_statement KW_END KW_IF
    {
        $$ = new IfNode(@1.first_line, @1.first_column, $2/* expr */, $4/* cp_stmt */, $6/* cp_stmt */);
    }
    | KW_IF expression KW_THEN compound_statement KW_END KW_IF
    {
        $$ = new IfNode(@1.first_line, @1.first_column, $2/* expr */, $4/* cp_stmt */);
    }
    ;

function_call_statement
    : function_call ';'
    {
        $$ = $1;
    }
    ;

loop_statement
    : while_statement
    {
        $$ = $1;
    }
    | for_statement
    {
        $$ = $1;
    }
    ;
while_statement
    : KW_WHILE expression KW_DO compound_statement KW_END KW_DO
    {
        $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4);
    }
    ;
for_statement
    : KW_FOR IDENTIFIER ASSIGN integer_literal KW_TO integer_literal KW_DO compound_statement KW_END KW_DO
    {
        /**
         * the most complicated part I think.
         */
            // for DeclNode (Note: here, the VariableNode in DeclNode has no ConstValNode.)
        Id i(@2.first_line, @2.first_column, $2);
                // for IdList
        IdList *il = new IdList();
        il->emplace_back(i);
                // for Type
        Type *t = new Type(ScalarType::INTEGER);
        DeclNode *dn = new DeclNode(@2.first_line, @2.first_column, il, t);
            // for AssignmentNode
                // for VariableReferenceNode
        VariableReferenceNode *vrn = new VariableReferenceNode(@2.first_line, @2.first_column, $2);
        AssignmentNode *an = new AssignmentNode(@3.first_line, @3.first_column, vrn, $4);
        // build ForNode
        $$ = new ForNode(@1.first_line, @1.first_column, dn, an, $6, $8);
        // delete & free
        free($2);
        delete il;
    }
    ;

return_statement
    : KW_RETURN expression ';'
    {
        $$ = new ReturnNode(@1.first_line, @1.first_column, $2);
    }
    ;

compound_statement
    : KW_BEGIN declarations statements KW_END
    {
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, $2, $3);
    }
    ;

statements
    : statements statement
    {
        $$ = $1;
        $$->emplace_back($2);
    }
    | /* empty */
    {
        $$ = new std::vector<AstNode *>();
    }
    ;

/* expressions */
expression
    : literal_constant
    {
        $$ = $1;    // upcasting: from   ConstValNode *   to   ExpressionNode *
    }
    | variable_reference
    {
        $$ = $1;
    }
    | function_call
    {
        $$ = $1;
    }
    | binary_operation
    {
        $$ = $1;
    }
    | unary_operation
    {
        $$ = $1;
    }
    | '(' expression ')'
    {
        $$ = $2;
    }
    ;

literal_constant
    : integer_literal
    {
        $$ = $1;
    }
    | real_literal
    {
        $$ = $1;
    }
    | string_literal
    {
        $$ = $1;
    }
    | boolean_literal
    {
        $$ = $1;
    }
    ;
integer_literal
    : OCTAL_INT
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);
    }
    | DECIMAL_INT
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);
    }
    ;
real_literal
    : FLOAT_CONST
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);
    }
    | SCIENTIFIC_NOTATION
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);
    }
    ;
string_literal
    : STRING_CONST
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, std::string($1));    // scanner returns (char *), but ctor wants (std::string)
    }
    ;
boolean_literal
    : KW_TRUE
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, true);
    }
    | KW_FALSE
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, false);
    }
    ;

variable_reference
    : IDENTIFIER
    {
        $$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1);
    }
    | array_reference
    {
        $$ = $1;
    }
    ;
array_reference
    : IDENTIFIER '[' expression ']'
    {
        $$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1, $3);
    }
    | array_reference '[' expression ']'
    {
        $$ = $1;
        $$->addInnerIndex($3);
    }
    ;

function_call
    : IDENTIFIER '(' expressions ')'
    {
        $$ = new FunctionInvocationNode(@1.first_line, @1.first_column, $1, $3);
        free($1);
    }
    ;

binary_operation
    : expression '+' expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::PLUS, $3);
    }
    | expression '-' expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::SUBTRACTION, $3);
    }
    | expression '*' expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::MULTIPLICATION, $3);
    }
    | expression '/' expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::DIVISION, $3);
    }
    | expression MOD expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::MOD, $3);
    }
    | expression '<' expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::LESS_THAN, $3);
    }
    | expression LESS_THAN_OR_EQUAL expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::LESS_THAN_OR_EQUAL, $3);
    }
    | expression NOT_EQUAL expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::NOT_EQUAL, $3);
    }
    | expression GREATER_THAN_OR_EQUAL expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::GREATER_THAN_OR_EQUAL, $3);
    }
    | expression '>' expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::GREATER_THAN, $3);
    }
    | expression '=' expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::EQUAL, $3);
    }
    | expression AND expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::AND, $3);
    }
    | expression OR expression
    {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, $1, OperatorType::OR, $3);
    }
    ;

unary_operation
    : '-' expression %prec NEGATION
    {
        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, OperatorType::NEGATION, $2);
    }
    | NOT expression
    {
        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, OperatorType::NOT, $2);
    }
    ;

expressions
    : non_empty_expressions
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = new std::vector<ExpressionNode *>();
    }
    ;
non_empty_expressions
    : expression
    {
        $$ = new std::vector<ExpressionNode *>();
        $$->emplace_back($1);
    }
    | non_empty_expressions ',' expression
    {
        $$ = $1;
        $$->emplace_back($3);
    }
    ;


%%
/* User subroutines (optional)*/

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [--dump-ast]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        ///
        AstDumper ast_dumper;
        root->accept(ast_dumper);

        // not visitor pattern:
        //root->print();
    }
    
    SemanticAnalyzer sema_analyzer;
    root->accept(sema_analyzer);

    // DONE: do not print this if there's any semantic error
    if (!sema_analyzer.hasSemanticError()){
        printf("\n"
            "|---------------------------------------------------|\n"
            "|  There is no syntactic error and semantic error!  |\n"
            "|---------------------------------------------------|\n");
    }

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
