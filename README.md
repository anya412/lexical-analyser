# Lexical Analyzer in C

A command-line lexical analyzer written in C that reads a C source file, classifies every token, and produces a detailed token-by-token report along with a summary of token counts.

---

## What it does

The analyzer scans a `.c` source file line by line and identifies the following token types:

| Token Type | Examples |
|---|---|
| Keywords | `int`, `char`, `return`, `switch`, `case` |
| Identifiers | `result`, `num1`, `sample` |
| Operators | `+`, `==`, `&&`, `+=`, `>>=` |
| Integer constants | `42`, `0`, `-7` |
| Real number constants | `3.14`, `-0.5` |
| Hex / Octal constants | `0xFF`, `0755` |
| Character literals | `'+'`, `'\n'` |
| String literals | `"Enter 2 nos: "`, `"%d %d"` |
| Delimiters | `(`, `)`, `{`, `}`, `;`, `,` |
| Preprocessor directives | `#include`, `#define`, `#pragma` |
| Single-line comments | `// this is a comment` |
| Multi-line comments | `/* spans multiple lines */` |

Results are written to `output.txt`, ending with a summary table of counts for each category.

---

## Project structure

```
.
├── main.h       # Shared structs (TokenStats, LexerState) and prototypes
├── main.c       # Entry point — file I/O, line loop, summary report
├── lexer.c      # Token classification and line tokenization logic
└── sample.c     # Example C program used to demonstrate the analyzer
```

---

## Building

Requires any C99-compatible compiler (GCC or Clang).

```bash
gcc -std=c99 -Wall -Wextra -o lexer main.c lexer.c
```

---

## Usage

```bash
./lexer <input_file.c>
```

The analyzer reads `<input_file.c>` and writes the token report to `output.txt` in the current directory.

**Example:**

```bash
./lexer sample.c
cat output.txt
```

---

## Sample output

Given this input in `sample.c`:

```c
switch (option) // This is a single line comment
{ /* Multi
     line
     comment */
    case '+':
        result = num1 + num2;
```

The analyzer produces:

```
switch is a keyword
( is a delimiter
option is an identifier
) is a delimiter
//This is a single line comment is a single line comment
{ is a delimiter
/*Multi
line
comment */ is a multi-line comment
case is a keyword
'+' is a character literal
: is a delimiter
result is an identifier
= is an operator
num1 is an identifier
+ is an operator
num2 is an identifier
; is a delimiter
```

And the summary at the end of `output.txt` looks like:

```
Summary:
Keywords:                15
Identifiers:             34
Operators:                9
Integers:                 0
Real Numbers:             0
String Literals:          9
Delimiters:              60
Preprocessor Directives:  1
Hex/Octal Numbers:        0
Character Literals:       4
Single Line Comments:     1
Multi Line Comments:      2
```

---

## Implementation notes

**`lexer.c` — token classification**

Each token type has a dedicated validator function (`is_keyword`, `is_identifier`, `is_operator`, etc.) that `tokenize_line()` calls after extracting a candidate token. Notable design decisions:

- `is_hex_octal()` is checked before `is_integer()` so that octal literals like `0755` are not misclassified as integers.
- `+` and `-` are handled exclusively as operators; signed numeric literals are recognised internally by `is_integer()` and `is_real_number()` rather than being pulled into the digit branch of the tokenizer.
- Preprocessor directives are matched with `strncmp` anchored at position 0 to avoid false positives from `strstr`.
- Three-character operators (`>>=`, `<<=`) are attempted before two-character and one-character matches.

**`LexerState` struct**

Multi-line comments and string literals that span multiple lines require state to be preserved across calls to `tokenize_line()`. Rather than using `static` local variables (which would corrupt processing if multiple files were analyzed in one run), all mutable state is held in a `LexerState` struct that the caller owns and passes in. The internal character buffer grows dynamically via `realloc` so there is no fixed length limit on comments or string literals.

---

## Limitations

- The input line buffer is 4096 bytes; lines longer than this will be truncated by `fgets`.
- The analyzer does not build a parse tree or perform any semantic analysis — it is purely lexical.
- Only standard C keywords up to C99 are recognized (`_Bool`, `_Complex`, etc. from later standards are not included).
