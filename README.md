
# ⚙️ C Compiler Project (LL(1) Based)

A compiler built in pure C that performs lexical and syntax analysis using an LL(1) parser.

---

## ✅ Features

### 🔹 Lexical Analyzer
- Scans the source code and converts it into a stream of tokens
- Supports:
  - Keywords, identifiers, literals
  - Operators, delimiters, separators
- Each token includes its type and the line number
- Stored as a linked list for easy traversal during parsing

### 🔹 LL(1) Parser
- Implements a predictive parsing engine using:
  - Stack-based parsing
  - A precomputed parsing table
  - Flat-indexed production rules
- Detects syntax errors with precise messages:
```

Syntax Error at line 5: expected IDENTIFIER but found ;

````

### 🔹 Syntax Error Reporting
- Clearly displays expected vs. actual tokens
- Highlights the line number of the error for easy debugging

---

## 🚀 How to Run

1. Place your source code in `exit1.txt`
2. Compile the compiler:
 ```bash
 gcc compiler.c -o compiler
````

3. Run the executable:

   ```bash
   ./compiler
   ```

---

## 📁 Project Structure

```
.
├── compiler.c       # Complete implementation (lexer + parser)
├── exit1.txt        # Input source code to be compiled
└── README.md        # Project documentation
```

---

## 🧪 Sample `exit1.txt` (You Can Modify)

```c
int a, b;
a = 10 + 20;
return a;
```

---

## 🧾 Output (Example)

```
----- Token Stream -----
[int] [ID: a] [,] [ID: b] [;]
[ID: a] [=] [NUM: 10] [+] [NUM: 20] [;]
[return] [ID: a] [;]

Parsing started...
Input Accepted by the Grammar.
```

---

## 📌 Note

The grammar used in this compiler is LL(1)-compliant and supports basic C-style syntax.
**A complete grammar section will be added when the project is finalized.**

---

## 🛠️ In Progress

* [ ] Intermediate Code Generation
* [ ] Code Optimization (Constant folding, dead code elimination)
* [ ] Symbol Table
* [ ] Grammar Documentation

---

## 👨‍💻 Author

Built with ❤️ in C by Sourajit Samanta

---
