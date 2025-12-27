# hw2 report

|||
|-:|:-|
|Name|楊睿宸|
|ID|112550049|

## How much time did you spend on this project

**~11 hours total** (design —> implementation —> tests)

## Project overview

> This project extends HW1 by integrating a **scanner (flex)** with a **parser (bison)** for the P language.

### Files

  * **`scanner.l`** — adapted from HW1 to match parser tokens/types and to supply richer error context.

  * **`parser.y`** — grammar, token/type defs, precedence, and driver.
### Scanner (changes from HW1 —> HW2)

#### **Token interface alignment**

  * Return **`ASSIGN (:=)`**.
  
  * Use a single **`RELATIONAL_OPERATOR`** token that carries the operator lexeme (`<`, `<=`, `<>`, `>=`, `>`, `=`).

  * Return arithmetic ops as **single-char tokens**: `'+'`, `'-'`, `'*'`, `'/'` (no separate `MINUS` token).

  * Identifiers and strings pass text via **typed yylval** (e.g., `<str>` used by the parser).
#### **Error context / globals**

  * Make **`current_line` a global `char[]`** so `parser.y` can `extern` it for `yyerror` snapshots.
#### **Driver cleanup**

  * Remove scanner-side `main()`; parser owns **entry**, scanner only provides `yylex()`.

### Parser

#### **Token & type defs (subset)**

  * Operators: `NEGATION` (unary -), `SUBTRACTION` (binary -), `ASSIGN`, ...
  
  * Typed tokens: `<str> RELATIONAL_OPERATOR`, `<str> STRING_CONST`, ...

  * Keywords: e.g. `KW_VAR`
#### **Precedence / associativity (low —> high)**
  
  Worth-mentioned:
  * **Binary `-`** via `%prec SUBTRACTION`
  
  * **Unary `-`** via `%prec NEGATION`
  
#### **Grammar features**

  * Zero or more items `{ items }`: declarations, functions, statements , and expressions.
  
  * **Array reference** is recursive to support `a[i][j][k]`.

### Scanner <—> Parser integration

  * Shared state via `extern int32_t line_num;` and **`extern char current_line[];`**.
    
  * Token/value types are consistent (`<str>` where needed).
    
  * Parser assumes scanner returns **raw `'-'`**; unary vs binary handled by `%prec` with `NEGATION`/`SUBTRACTION`.

## What is the hardest you think in this project

  * Disambiguating **unary vs binary `-`** cleanly (keep scanner simple, resolve in parser with `%prec`). Originally, I wrote 
      ```yacc
      binary_operation
          : expression binary_operator expression
          ;
      binary_operator
          : '+'
          | '-'
          | '*'
          | '/'
          | MOD

          ...

          ;
      ```
      But it could not solve **shift/reduce conflict** between NEGATION and SUBTRACTION.

  * Changing almost all regex rules in scanner.l. I don't like **hard-coding** so I wrote lots of regex macros in scanner.l, and use them in rules. But I need to revise them and hard code.
  
## Feedback to T.A.s

#### The spec is clear and workable. Minor suggestions:

  1. Specify that `current_line` is a **global array** used by parser.y. (So change in scanner.l is needed if you use hw1 code.)

  2. Clarify **variable vs array reference** since `_variable_reference_ -> **identifier**` and `_array_reference_ -> **identifier**`.
