/* Declarations (optional) */

    /* C declarations and includes */
%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

    /* Yacc definitions */

/* Declare the possible data types of semantic values */
%union {
    /* not finished (wait for hw3) */
    int     val;
    char*   str;
    int     opT;
}

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
%token NEGATION SUBTRACTION ASSIGN NOT MOD AND OR
%token <str> RELATIONAL_OPERATOR

/* reserved words */
/* add KW_ prefix to macro name */
%token KW_VAR
%token KW_ARRAY KW_OF KW_BOOLEAN KW_INTEGER KW_REAL KW_STRING
%token KW_TRUE KW_FALSE
%token KW_WHILE KW_DO KW_IF KW_THEN KW_ELSE KW_FOR KW_TO
%token KW_BEGIN KW_END
%token KW_PRINT KW_READ KW_RETURN

/* numeric */
%token DECIMAL_INT OCTAL_INT 
%token FLOAT_CONST
%token SCIENTIFIC_NOTATION

/* string */
%token <str> STRING_CONST

/* identifier */
%token <str> IDENTIFIER

/* Associativity & Precedence */
/* Note: 
 * 1. Tokens declared on the same %left / %right / %nonassoc(no associativity) line
 *    share the same precedence and associativity.
 * 2. Associativity is for terminals (especially for operators)
 * 3. Listed in increasing precedence (lowest first)
 */
%left AND OR NOT
%left RELATIONAL_OPERATOR
%left SUBTRACTION
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
%type function function_declaration function_definition function_header formal_argument 

%type functions parameters non_empty_parameters     /* those not specified in spec (so i use a line to separate) */

/* declarations */
%type declaration
%type variable_declaration 
%type constant_declaration identifier_list

%type declarations non_empty_identifier_list

/* types */
%type type
%type scalar_type 
%type array_type

/* statements */
%type statement
%type simple_statement assignment print_statement read_statement 
%type conditional_statement 
%type function_call_statement 
%type loop_statement while_statement for_statement 
%type return_statement
%type compound_statement

%type statements

/* expressions */
%type expression 
%type literal_constant integer_literal real_literal string_literal boolean_literal
%type variable_reference array_reference
%type function_call
%type binary_operation binary_operator
%type unary_operation unary_operator

%type expressions non_empty_expressions


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
    ;

function
    : function_declaration
    | function_definition
    ;
function_header
    : IDENTIFIER '(' parameters ')' ':' scalar_type
    | IDENTIFIER '(' parameters ')'
    ;
function_declaration
    : function_header ';'
    ;
function_definition
    : function_header compound_statement KW_END
    ;
formal_argument
    : identifier_list ':' type
    ;

functions
    : function functions
    | /* empty */
    ;
parameters
    : non_empty_parameters
    | /* empty */
    ;
non_empty_parameters
    : formal_argument
    | formal_argument ';' non_empty_parameters
    ;

/* declarations */
declaration
    : variable_declaration
    | constant_declaration
    ;

variable_declaration
    : KW_VAR identifier_list ':' type ';'
    ;

constant_declaration
    : KW_VAR identifier_list ':' integer_literal ';'
    | KW_VAR identifier_list ':' '-' integer_literal ';'
    | KW_VAR identifier_list ':' real_literal ';'
    | KW_VAR identifier_list ':' '-' real_literal ';'
    | KW_VAR identifier_list ':' string_literal ';'
    | KW_VAR identifier_list ':' boolean_literal ';'
    ;
identifier_list
    : non_empty_identifier_list
    | /* empty */
    ;

declarations
    : declaration declarations
    | /* empty */
    ;
non_empty_identifier_list
    : IDENTIFIER
    | IDENTIFIER ',' non_empty_identifier_list
    ;

/* types */
type
    : scalar_type
    | array_type
    ;

scalar_type
    : KW_INTEGER
    | KW_REAL
    | KW_STRING
    | KW_BOOLEAN
    ;

array_type
    : KW_ARRAY integer_literal KW_OF type
    ;

/* statements */
statement
    : simple_statement
    | conditional_statement
    | function_call_statement
    | loop_statement
    | return_statement
    | compound_statement
    ;

simple_statement
    : assignment
    | print_statement
    | read_statement
    ;
assignment
    : variable_reference ASSIGN expression ';'
    ;
print_statement
    : KW_PRINT expression ';'
    ;
read_statement
    : KW_READ variable_reference ';'
    ;

conditional_statement
    : KW_IF expression KW_THEN compound_statement KW_ELSE compound_statement KW_END KW_IF
    | KW_IF expression KW_THEN compound_statement KW_END KW_IF
    ;

function_call_statement
    : function_call ';'
    ;

loop_statement
    : while_statement
    | for_statement
    ;
while_statement
    : KW_WHILE expression KW_DO compound_statement KW_END KW_DO
    ;
for_statement
    : KW_FOR IDENTIFIER ASSIGN integer_literal KW_TO integer_literal KW_DO compound_statement KW_END KW_DO
    ;

return_statement
    : KW_RETURN expression ';'
    ;

compound_statement
    : KW_BEGIN declarations statements KW_END
    ;

statements
    : statement statements
    | /* empty */
    ;

/* expressions */
expression
    : literal_constant
    | variable_reference
    | function_call
    | binary_operation
    | unary_operation
    | '(' expression ')'
    ;

literal_constant
    : integer_literal
    | real_literal
    | string_literal
    | boolean_literal
    ;
integer_literal
    : OCTAL_INT
    | DECIMAL_INT
    ;
real_literal
    : FLOAT_CONST
    | SCIENTIFIC_NOTATION
    ;
string_literal
    : STRING_CONST
    ;
boolean_literal
    : KW_TRUE
    | KW_FALSE
    ;

variable_reference
    : IDENTIFIER
    | array_reference
    ;
array_reference
    : IDENTIFIER '[' expression ']'
    | array_reference '[' expression ']'
    ;

function_call
    : IDENTIFIER '(' expressions ')'
    ;

binary_operation
    : expression '+' expression
    | expression '-' expression %prec SUBTRACTION     /* make precedence of this rule the same as SUBTRACTION */
    | expression '*' expression
    | expression '/' expression
    | expression MOD expression
    | expression RELATIONAL_OPERATOR expression
    | expression AND expression
    | expression OR expression
    ;

unary_operation
    : '-' expression %prec NEGATION
    | NOT expression
    ;

expressions
    : non_empty_expressions
    | /* empty */
    ;
non_empty_expressions
    : expression
    | expression ',' non_empty_expressions
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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
