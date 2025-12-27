# hw1 report

|Field|Value|
|-:|:-|
|Name|楊睿宸|
|ID|112550049|

## How much time did you spend on this project

**~9 hours total:**

- **First 2/3 (~6 hours):** Researching flex documentation, understanding terminology, functions, and syntax patterns like:
  ```flex
  #define YY_USER_ACTION updateCurrentLine(yytext);
  ```
- **Last 1/3 (~3 hours):** Debugging and validating against test cases

## Project overview

This project implements a lexical analyzer(scanner) for the **P language** using flex, structured in three main sections:

### 1. Definitions Section

I implemented regex patterns using a **hierarchical structure** for enhanced readability and modularity:

```
operator
├── arith_oper     ([+\-*/]|"mod"|":=")
├── rational_oper  ("<"|"<="|"<>"|">="|">"|"=")
└── logic_oper     (and|or|not)
```

**Key components:**
- **Token macros:** Patterns for tokens passed to the parser
- **Discard macros:** Patterns for whitespace and comments
- **Start condition:** `<C_STYLE_COMMENT>` - An exclusive state(*%x*) for consuming comments


### 2. Rules Section

> **Precedence of Rules:**
> 1. Exclusive start condition
> 2. Longest match
> 3. Near front

The scanner matches tokens using prioritized rules:

#### a) C-Style Comment Handling
```flex
<C_STYLE_COMMENT>.|\n        ;
<C_STYLE_COMMENT>"*/"        { BEGIN INITIAL; }
<INITIAL>"/*"                { BEGIN C_STYLE_COMMENT; }
```
The scanner enters `COMMENT` state upon encountering `"/*"` and consumes all characters without output until `"*/"` is found.

#### b) Pseudocomment Processing
Controls global flags (`opt_src`, `opt_tok`) for source and token listing:
```c
if(yytext[3] == 'S'){
    if(yytext[4] == '+'){
        opt_src = 1;
    }
    else if(yytext[4] == '-'){
        opt_src = 0;
    }
}
```

#### c) String Constant Processing
Special handling for:
- Removing boundary quotes (`"`)
- Converting internal `""` -> `"` (escape mechanism)

```c
// Process string content, handling escaped quotes
for(char *yytextPtr = yytext +1; yytextPtr[1] != '\0'; contentPtr++, yytextPtr++){
    *contentPtr = *yytextPtr;
    if(*yytextPtr == '"'){
        yytextPtr++;
    }
}
*contentPtr = 0;
```

#### d) Error Handling
Catches unmatched input and reports errors with line numbers.

#### e) Others not mentioned are common rules

### 3. Code Section

**Modified functions:**
- `listToken()`: Enhanced with optional `name2` parameter for reserved words (e.g., `"KW"` + `<lexeme>`)

**Key functionalities:**
- Processes input file via `yylex()`
- `YY_USER_ACTION`: Invoked for each matched token
- `updateCurrentLine()`: Maintains source line buffer for listing and error reporting


>For more, please see the comments beside code. I wrote comments to explain the considerations of several implementations.


## What is the hardest you think in this project

### Primary Challenge: Learning Flex

The steepest learning curve was understanding flex's architecture and capabilities:

**Fear of unknown unknowns:** Not knowing what features existed or how to leverage them effectively though some are taught in class


## Feedback to T.A.s

The assignment infrastructure is excellent:
- ✅ GitHub platform for submission
- ✅ Docker environment for consistency
- ✅ Clear, editable specifications

*No significant improvements needed - this is a well-designed assignment!*
