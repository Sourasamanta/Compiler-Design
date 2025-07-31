# 🔍 LL(1) Parser in C

A fully functional **LL(1) predictive parser** implemented in C for a simplified C-like programming language. It includes a hand-written lexer, a predictive parsing table, parse tree construction, semantic analysis with a symbol table, and an interpreter for executing programs.

---

## 🚀 Features

- ✅ **Declarations** (`int`, `float`), **assignments**, and **expressions**
- ✅ Supports **logical** (`&&`, `||`), **relational** (`<`, `<=`, `>`, `>=`, `==`, `!=`), **arithmetic** (`+`, `-`, `*`, `/`), and **unary** (`-`, `+`, `!`) operators
- ✅ **Control flow**: `if-else`, `while` loops with **nested scopes**
- ✅ `return` statements and **custom `printf` syntax**:
```

printf @"format \~id"@;

```
- ✅ **Hand-written lexer**: supports single-line (`//`) and multi-line (`/* ... */`) comments
- ✅ **LL(1) parsing** using a stack and a **predictive parsing table**
- ✅ **Grammar with 53 productions**, including FIRST and FOLLOW sets
- ✅ **Parse tree generation** with token and type tracking
- ✅ **Detailed syntax and semantic error reporting** with line numbers
- ✅ **Symbol table** for variable declarations, types, values
- ✅ **Interpreter** that executes parsed programs
- ✅ Handles **epsilon (ε) productions** with lookahead

---

## 📂 Project Structure

```

├── try1.2.c       # Full lexer, parser, grammar, semantic analysis, interpreter
├── exit1.txt      # Input source code for parsing and execution
├── README.md      # Documentation (this file)

````

---

## 🛠️ How to Compile and Run

### 🧱 Compile
```bash
gcc try1.2.c -o parser
````

### ▶️ Run

```bash
./parser
```

Ensure `exit1.txt` is in the same directory.

---

## 🧪 Test Cases (`exit1.txt`)

### ✅ Test Case 1: Full program with all constructs

```c
int x, y, z, result;
x = 10;
y = 20;
if (x < y || x == y) {
    result = x + y;
}
return result;
```

**Output**: `Assigns result = 30, returns 30`

---

### 🖨️ Test Case 2: `printf` statement

```c
int a;
a = 5;
printf @"Value is ~a"@;
return a;
```

**Output**: `Prints "Value is 5", returns 5`

---

### 🔁 Test Case 3: Loops and unary

```c
int x, y, z;
y = 2;
z = 0;
x = -(10 * (y + !z));
while (x < -15) {
    x = x + 1;
}
return x;
```

**Output**: `x = -30`, loop runs to `x = -15`, returns `-15`

---

### ❌ Test Case 4: Invalid syntax (missing semicolon)

```c
int x;
x = 10
return x;
```

**Output**: `Syntax Error at line 2: Expected terminal ';', got EOF`

---

### 🕳️ Test Case 5: Empty file

```c
// empty
```

**Output**: `Returns 0 with empty parse tree`

---

### ⚠️ Test Case 6: Semantic error (undeclared variable)

```c
x = 10;
return x;
```

**Output**: `Semantic Error at line 1: Undeclared variable 'x'`

---

## 🧾 Sample Output (Test Case 2)

```
----- Full Token List -----
Token 0: int (type 0, line 1)
Token 1: a (type 4, line 1)
...
Parsing Successful!

--------------- Parse Tree ---------------
Program (children: 1)
  StatementList (children: 3)
    Statement (children: 3)
      Type (children: 1)
        int [token: int, line: 1] (children: 0)
...

----- Symbol Table -----
Symbol 0: a (type: int, line: 1, value: 5, hasValue: 1)
------------------------

----- Executing Program -----
Value is 5

----- Execution Complete -----
Return value: 5.000000
```

## ⚠️ Limitations

* Custom `printf` syntax (`@"...~id"@`) is non-standard
* Only `int` and `float` types are supported
* No interactive shell — input only via `exit1.txt`
* No support for **function declarations** or **calls**
* Limited type checking (mixed-type ops allowed)
* Executes directly — no intermediate code generation

---

## 👨‍💻 Author

**Sourajit Samanta**
Compiler Design Enthusiast | BTech CSE

---
