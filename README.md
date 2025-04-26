# 🛠️ Compiler Project

Hello guys 👋  
I am building a **Compiler** from scratch in C.

The compiler has 3 main parts:

1. **Lexer**
2. **Parser**
3. **Code Optimization**

---

## ✅ Lexer Module (Completed)

You can test the lexer by writing any C code inside the `exit1.txt` file.

---

### 🧪 Example

#### 🔤 Input and Output (`exit1.txt`):
<br><br>
**INPUT**

int a, b;<br>
float division;

printf("Enter two integers: ");<br>
scanf("%d %d", &a, &b);

printf("Sum = %d\n", a + b);<br>
printf("Difference = %d\n", a - b);<br>
printf("Product = %d\n", a * b);

if (b != 0) {<br>
    division = (float)a / b;<br>
    printf("Division = %.2f\n", division);<br>
} else {<br>
    printf("Cannot divide by zero!\n");<br>
}

return 0;
<br><br>

**OUTPUT**

----- Token List -----<br>
Keyword: int<br>
Keyword: a<br>
Keyword: b<br>
Separator: ;<br>

----- Token List -----<br>
Keyword: float<br>
Keyword: division<br>
Separator: ;<br>

----- Token List -----<br>

----- Token List -----<br>
Keyword: printf<br>
Separator: (<br>
Keyword: Enter two integers:<br>
Separator: )<br>
Separator: ;<br>

----- Token List -----<br>
Keyword: scanf<br>
Separator: (<br>
Keyword: %d %d<br>
Keyword: &a<br>
Keyword: &b<br>
Separator: )<br>
Separator: ;<br>

----- Token List -----<br>

----- Token List -----<br>
Keyword: printf<br>
Separator: (<br>
Keyword: Sum = %d\n<br>
Keyword: a<br>
Operator: +<br>
Keyword: b<br>
Separator: )<br>
Separator: ;<br>

----- Token List -----<br>
Keyword: printf<br>
Separator: (<br>
Keyword: Difference = %d\n<br>
Keyword: a<br>
Operator: -<br>
Keyword: b<br>
Separator: )<br>
Separator: ;<br>

----- Token List -----<br>
Keyword: printf<br>
Separator: (<br>
Keyword: Product = %d\n<br>
Keyword: a<br>
Operator: *<br>
Keyword: b<br>
Separator: )<br>
Separator: ;<br>

----- Token List -----<br>

----- Token List -----<br>
Keyword: if<br>
Separator: (<br>
Keyword: b<br>
Operator: =<br>
Literal: 0<br>
Separator: )<br>

----- Token List -----<br>
Keyword: division<br>
Operator: =<br>
Separator: (<br>
Keyword: float<br>
Separator: )<br>
Keyword: a<br>
Operator: /<br>
Keyword: b<br>
Separator: ;<br>

----- Token List -----<br>
Keyword: printf<br>
Separator: (<br>
Keyword: Division = %.2f\n<br>
Keyword: division<br>
Separator: )<br>
Separator: ;<br>

----- Token List -----<br>
Keyword: else<br>

----- Token List -----<br>
Keyword: printf<br>
Separator: (<br>
Keyword: Cannot divide by zero!\n<br>
Separator: )<br>
Separator: ;<br>

----- Token List -----<br>

----- Token List -----<br>

----- Token List -----<br>
Keyword: return<br>
Literal: 0<br>
Separator: ;<br>

Press any key to continue . . .

## 🔜 Upcoming Features

- ✅ Lexer: Done
- 🔄 Parser: In Progress
- 🚀 Code Optimization: Coming Soon

