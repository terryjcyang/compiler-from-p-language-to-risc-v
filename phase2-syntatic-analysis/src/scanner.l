/* Definition section */

%option never-interactive
/* no unput: faster if don't need yyless() or unput() */
%option nounput
/* no input: to reduce code size */
%option noinput

    /* Included code */
%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/** hw2 mod:
 * the token macro defined in the parser.y
 */
#include "parser.h"

#define MAX_LINE_LEN 512
/* Code runs each time a token is matched. */
#define YY_USER_ACTION updateCurrentLine(yytext);

uint32_t line_num = 1;
uint32_t col_num = 1;
char current_line[MAX_LINE_LEN];    /* hw2 mod: make current_line visible to parser */
static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
// static char current_line[MAX_LINE_LEN];  /* hw2 mod */

static void updateCurrentLine(const char *source);
static void listToken(const char *name1, const char *name2);
static void listLiteral(const char *name, const char *literal);

%}

    /* RE macro */

/** Rules of writing REs:
 * 1.
 * operating char
 * “ \ [ ] ^ - ? . * + | ( ) $ / { } % < > 
 * listed above need escaping
 * since they have special meanings in REs
 *
 * 2.
 * character class [...]
 * ], -, ^, \ must be escaped
 *
 * 3. Can use "" to quote literals(cannot suppress '\' still)
 *
 * 4.
 * flex will just paste your macro to replace {macro} you write
 * e.g.
 *      re1 dsf|dbf
 *      re2 {re1}*
 *      
 *      re2 = dsf|dbf*, but not (dsf|dbf)*
 *      So you need grouping (), e.g. re1 (dsf|dbf)
 */

/* Tokens passed to parser */
delimiter               [,;:()[\]]

arith_oper              ([+\-*/]|"mod"|":=")
relational_oper           ("<"|"<="|"<>"|">="|">"|"=")
logic_oper              (and|or|not)
operator                ({arith_oper}|{relational_oper}|{logic_oper})

declaration             (var|def)
type                    (array|of|boolean|integer|real|string)
value                   (true|false)
flowOfControl           (while|do|if|then|else|for|to)
block                   (begin|end)
statement               (print|read|return)
reservedWord            ({declaration}|{type}|{value}|{flowOfControl}|{block}|{statement})

/* In hw1, letter does not contain '_' */
letter                  [a-zA-Z]
digit                   [0-9]

/* In hw1, identifiers cannot include '_' */
identifier              ({letter}({letter}|{digit})*)

octalInt                (0[0-7]+)
nonZeroDecimalInt       ([1-9]{digit}*)
decimalInt              (0|{nonZeroDecimalInt})
intConst                ({octalInt}|{decimalInt})

nonZeroFloatConst       (({decimalInt}\.{digit}*[1-9])|({nonZeroDecimalInt}\.0))
floatConst              ({decimalInt}\.({digit}*[1-9]|0))

scientificNotation      (({nonZeroDecimalInt}|{nonZeroFloatConst})[Ee][+-]?{decimalInt})

/** here, stringConst(a macro) includes
 * 1.   beginning and end double quotes(")
 * 2.   internal ""(double quotes, which represents a single quote)
 * 3.   \t, \n(as two char '\' and 'n')
 * 4.   \\(which represents a single back slash), \"
 */
stringConst             (\"([^"]|\"\")*\")

/* Tokens discarded */
whitespace              [ \t\n]+

cppStyleComment         (\/\/.*)


    /* Start conditions */
/** In Flex, there are two types of Start Conditions: %s and %x. 
 * %s: inclusive. %x: exclusive(when it starts, only rules for the state are active.)
 */
%x C_STYLE_COMMENT


%%
    /* Rule section */
    /** In rule section, comments must be led by space char.
     *
     * syntax:
     * regexp      action(c code)
     */

    /* discarded */

<C_STYLE_COMMENT>.|\n                       ;
<C_STYLE_COMMENT>"*/"                       {
    BEGIN INITIAL;
}
<INITIAL>"/*"                               {
    BEGIN C_STYLE_COMMENT;
}

    /* Pseudocomment */
"//&"[ST][+-].*                             {
    if(yytext[3] == 'S'){
        if(yytext[4] == '+'){
            opt_src = 1;
        }
        else if(yytext[4] == '-'){
            opt_src = 0;
        }
    }
    else if(yytext[3] == 'T'){
        if(yytext[4] == '+'){
            opt_tok = 1;
        }
        else if(yytext[4] == '-'){
            opt_tok = 0;
        }
    }
}

{cppStyleComment}                           ;

{whitespace}                                ;


    /* tokens passed to parser */

    /** hw2 mod:
     * symbols(tokens) have diff. roles on syntactic aspect, 
     * so need to match by diff. token types
     */

    /* {delimiter}|{operator}                      {
        listToken(yytext, NULL);
    } */
","     { listToken(yytext, NULL); return ','; }
";"     { listToken(yytext, NULL); return ';'; }
":"     { listToken(yytext, NULL); return ':'; }
"("     { listToken(yytext, NULL); return '('; }
")"     { listToken(yytext, NULL); return ')'; }
"["     { listToken(yytext, NULL); return '['; }
"]"     { listToken(yytext, NULL); return ']'; }

"-"     { listToken(yytext, NULL); return '-'; }
":="    { listToken(yytext, NULL); return ASSIGN; }

"not"   { listToken(yytext, NULL); return NOT; }

"+"     { listToken(yytext, NULL); return '+'; }
"*"     { listToken(yytext, NULL); return '*'; }
"/"     { listToken(yytext, NULL); return '/'; }
"mod"   { listToken(yytext, NULL); return MOD; }
{relational_oper} { listToken(yytext, NULL); return RELATIONAL_OPERATOR; }
"and"   { listToken(yytext, NULL); return AND; }
"or"    { listToken(yytext, NULL); return OR; }


    /** hw2 mod:
     * reserved words
     * use KW_ prefix to avoid conflict with macros used by bison, 
     * e.g. BEGIN 
     */
"var"       { listToken("KW", yytext); return KW_VAR; }

"array"     { listToken("KW", yytext); return KW_ARRAY; }
"of"        { listToken("KW", yytext); return KW_OF; }
"boolean"   { listToken("KW", yytext); return KW_BOOLEAN; }
"integer"   { listToken("KW", yytext); return KW_INTEGER; }
"real"      { listToken("KW", yytext); return KW_REAL; }
"string"    { listToken("KW", yytext); return KW_STRING; }

"true"      { listToken("KW", yytext); return KW_TRUE; }
"false"     { listToken("KW", yytext); return KW_FALSE; }

"while"     { listToken("KW", yytext); return KW_WHILE; }
"do"        { listToken("KW", yytext); return KW_DO; }
"if"        { listToken("KW", yytext); return KW_IF; }
"then"      { listToken("KW", yytext); return KW_THEN; }
"else"      { listToken("KW", yytext); return KW_ELSE; }
"for"       { listToken("KW", yytext); return KW_FOR; }
"to"        { listToken("KW", yytext); return KW_TO; }

"begin"     { listToken("KW", yytext); return KW_BEGIN; }
"end"       { listToken("KW", yytext); return KW_END; }

"print"     { listToken("KW", yytext); return KW_PRINT; }
"read"      { listToken("KW", yytext); return KW_READ; }
"return"    { listToken("KW", yytext); return KW_RETURN; }

    /* {reservedWord}                              {
        listToken("KW", yytext); return RESERVED_WORD;
    } */

{decimalInt}                                {
    listLiteral("integer", yytext); return DECIMAL_INT;
}
{octalInt}                                  {
    listLiteral("oct_integer", yytext); return OCTAL_INT;
}

{floatConst}                                {
    listLiteral("float", yytext); return FLOAT_CONST;
}

{scientificNotation}                        {
    listLiteral("scientific", yytext); return SCIENTIFIC_NOTATION;
}

{stringConst}                               {
    /* here, we don't convert \t, \n, or \\ inside a string.(As the hw1 says no) */

    size_t sizeOfYytext = sizeof(char) * (yyleng +1);
    char *strContent = (char *)malloc(sizeOfYytext -2);
    /**
     * Deal with internal(inside string) ""
     * --make them single "
    */
    char *contentPtr = strContent;

    /** Caution:
     * Flex does not prevent you from 
     * using the same { variable name(identifier name) in the produced scanner.c code }
     * as your self-defined variable.
     * But this may cause error.
     * E.g. yytext_ptr is used in the produced scanner.c code,
     *      so here you shouldn't use it.
     */
    /* Start from yytext+1 to exclude beginning ", end if yytextPtr[1] is '\0'(so that *yytextPtr == '"' won't be included) */
    for(char *yytextPtr = yytext +1; yytextPtr[1] != '\0'; contentPtr++, yytextPtr++){
        *contentPtr = *yytextPtr;
        if(*yytextPtr == '"'){
            /* Assume inside the str, '"' is followed by '"'. i.e. Rule works correctly. */
            yytextPtr++;
        }
    }
    *contentPtr = 0;                        /* add EOF to str */

    listLiteral("string", strContent);
    free(strContent);

    return STRING_CONST;
}

{identifier}                                {
    listLiteral("id", yytext); return IDENTIFIER;
}


    /* Catch the character which is not accepted by rules above */
.                                           {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%
/* Code section */

/** @note The line is printed out and flushed when a newline character is encountered. */
static void updateCurrentLine(const char *source) {
    /* col_num is one-based */
    for (const char *c = source; *c; ++c) {
        if (*c == '\n') {
            if (opt_src) {
                printf("%d: %s\n", line_num, current_line);
            }
            ++line_num;
            col_num = 1;
            current_line[0] = '\0';
        } else {
            if (col_num < MAX_LINE_LEN) {
                current_line[col_num - 1] = *c;
                current_line[col_num] = '\0';
            } else {
                /* Truncate silently; doesn't affect the program's correctness. */
            }
            ++col_num;
        }
    }
}

static void listToken(const char *name1, const char *name2) {
    if (opt_tok) {
        printf("<%s%s>\n", name1, (name2)?name2:"");
    }
}

static void listLiteral(const char *name, const char *literal) {
    if (opt_tok) {
        printf("<%s: %s>\n", name, literal);
    }
}

/** @note This function is not required if the input file is guaranteed to end
 * with a newline. However, students may find it useful to handle the case where
 * the input file does not end with a newline, as it has been reported several
 * times in the past.
 */
int yywrap(void) {
    /* If the file is not ended with a newline, fake it to print out the last line. */
    if (col_num > 1) {
        updateCurrentLine("\n");
    }
    /* no more input file */
    return 1;
}

/** hw2 mod:
 * remove the main() in scanner.l
 */
