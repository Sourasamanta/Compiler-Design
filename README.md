# ⚙️ C Compiler Project (LL(1) Based)

A compiler built in pure C that performs lexical and syntax analysis using an LL(1) parser.

---

## ✅ Features

### 🔹 Lexical Analyzer
- Scans source code and converts it into a stream of tokens
- Supports:
  - Keywords (e.g., `int`, `float`, `return`, `if`, `else`, `while`)
  - Identifiers, numeric literals
  - Operators (e.g., `+`, `-`, `*`, `/`, `=`, `<`, `<=`, `>`, `>=`, `==`, `!=`, `&&`, `||`, `!`)
  - Separators (e.g., `(`, `)`, `,`, `;`, `{`, `}`)
- Each token includes its type and line number
- Stored as a linked list for parsing

### 🔹 LL(1) Parser
- Implements a predictive parsing engine using:
  - Stack-based parsing
  - Precomputed parsing table
  - Flat-indexed production rules
- Detects syntax errors with precise messages, e.g.:
  ```
  Syntax Error at line 5: expected IDENTIFIER but found ;
  ```

### 🔹 Syntax Error Reporting
- Displays expected vs. actual tokens
- Highlights line numbers for debugging

---

## 🚀 How to Run

1. Place your source code in `exit1.txt`
2. Compile the compiler:
   ```bash
   gcc compiler.c -o compiler
   ```
3. Run the executable:
   ```bash
   ./compiler
   ```

---

## 📁 Project Structure

```
.
├── compiler.c       # Core compiler implementation (lexer + parser)
├── exit1.txt       # Input source code file
└── README.md       # Project documentation
```

---

## 🧪 Sample `exit1.txt` (Editable)

```c
int a, b;
a = 10 + 20;
return a;
```

---

## 🧾 Sample Output

```
----- Full Token List -----
Token 0: int (type 0, line 1)
Token 1: a (type 4, line 1)
Token 2: , (type 3, line 1)
Token 3: b (type 4, line 1)
Token 4: ; (type 3, line 1)
Token 5: a (type 4, line 2)
Token 6: = (type 2, line 2)
Token 7: 10 (type 1, line 2)
Token 8: + (type 2, line 2)
Token 9: 20 (type 1, line 2)
Token 10: ; (type 3, line 2)
Token 11: return (type 0, line 3)
Token 12: a (type 4, line 3)
Token 13: ; (type 3, line 3)
Token 14: $ (type 3, line 3)

----- Token List -----
Keyword: int (line 1)
Identifier: a (line 1)
Separator: , (line 1)
Identifier: b (line 1)
Separator: ; (line 1)
Identifier: a (line 2)
Operator: = (line 2)
Literal: 10 (line 2)
Operator: + (line 2)
Literal: 20 (line 2)
Separator: ; (line 2)
Keyword: return (line 3)
Identifier: a (line 3)
Separator: ; (line 3)
End Marker: $ (line 3)

Parsing Successful!

--------------- Parse Tree ---------------
Program
  StatementList
    Statement
      Type
        int [token: int]
      id [token: a]
      StatementSuffix
        DeclTail
          , [token: ,]
          id [token: b]
          DeclTail
            ; [token: ;]
    StatementList
      Statement
        id [token: a]
        = [token: =]
        Expression
          RelExpr
            ArithExpr
              Term
                Factor
                  id [token: 10]
                TermPrime
                  * [token: +]
                  Factor
                    id [token: 20]
                  TermPrime
              ArithExprPrime
          ExpressionPrime
        ; [token: ;]
      StatementList
        Statement
          ReturnStmt
            return [token: return]
            Expression
              RelExpr
                ArithExpr
                  Term
                    Factor
                      id [token: a]
                    TermPrime
                  ArithExprPrime
                RelOpTail
              ExpressionPrime
            ; [token: ;]
        StatementList
          $ [token: $]
------------------------------------------
```

---

## 📌 Note

The grammar is LL(1)-compliant and supports basic C-style syntax. A complete grammar specification will be added upon project completion.

---

## 🛠️ In Progress

- [ ] Intermediate Code Generation
- [ ] Code Optimization (e.g., constant folding, dead code elimination)
- [ ] Symbol Table Implementation
- [ ] Grammar Documentation

---

## 👨‍💻 Author

Built with ❤️ in C by Sourajit Samanta
