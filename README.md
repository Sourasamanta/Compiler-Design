# 🔍 LL(1) Parser in C

A fully functional LL(1) recursive descent parser implemented in C that handles a wide range of C-like language constructs such as declarations, assignments, function calls, conditionals, loops, and expressions. It includes:

* A hand-written lexer that generates tokens from input
* LL(1) parsing logic using a stack and a predictive parsing table
* Complete grammar with FIRST and FOLLOW sets
* Parse tree generation for syntactic structure
* Clear syntax error reporting with line numbers

---

## 🚀 Features

* ✅ Supports declarations (`int`, `float`), assignments, expressions
* ✅ Logical, relational, arithmetic, and unary operators
* ✅ `if`-`else`, `while` blocks with nested scopes
* ✅ `return` statements and custom `printf` syntax
* ✅ LL(1) Parsing Table with flat indexed grammar rules
* ✅ Detailed error reporting (line number, expected vs actual token)
* ✅ Parse tree construction with each node linked to tokens
* ✅ Epsilon (`ε`) productions and lookahead handling

---

## 📂 Project Structure

```bash
├── try1.2.c             # Full lexer, parser, grammar and parse tree
├── exit1.txt            # Test input program (tokenized and parsed)
├── README.md            # This documentation
```

---

## 🛠️ How to Compile and Run

### 🧱 Compile

```bash
gcc try1.2.c -o parser
```

### ▶️ Run

```bash
./parser
```

Make sure `exit1.txt` is present in the same directory.

---

## 🧪 Test Cases (`exit1.txt`)

Includes a variety of real-world scenarios:

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

### 🖨️ Test Case 2: `printf` statement

```c
int a;
a = 5;
printf @"Value is ~a"@;
return a;
```

### 🔁 Test Case 3: Loops and unary

```c
int x, y, z;
x = -(10 * (y + !z));
return x;
```

### ❌ Test Case 4: Invalid syntax (missing semicolon)

```c
int x;
x = 10      // <- Missing semicolon here
return x;
```

### 🕳️ Test Case 5: Empty file (should be handled gracefully)

```c
// empty
```

More included in `exit1.txt`.

---

## 🧾 Output

* ✅ **On Success**: Outputs parse tree traversal or confirmation
* ❌ **On Failure**: Prints line-wise syntax errors with expected token(s)

---

## ⚠️ Limitations

* No semantic analysis or symbol table (planned)
* Custom printf syntax (`@...~id@`) is non-standard
* Only integer and float types supported
* Single `exit1.txt` input file (no interactive shell)

---

## 📌 Future Work

* ✅ Add symbol table construction
* ✅ Generate intermediate code (3AC)
* ✅ Semantic checks (type safety, redeclaration, etc.)
* ✅ Code optimization passes

---

## 👨‍💻 Author

Sourajit Samanta
