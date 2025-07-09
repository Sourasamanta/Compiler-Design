#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>

#define sz 100


//Represent the grammar

#define maxProduction 10
#define maxSymbolPerProduction 10
#define maxSymbolLength 16
#define num_nonTerminal 22
#define num_terminal 23


int flatIndexCounter = 1;


int parsingTable[num_nonTerminal][num_terminal] = {
/*                       int    float  id     return =      +      -      *      /      <     <=      >     >=     ==     !=     &&     ||      (      )      ,      ;     literal $ */
/*         Program*/ {   1,     1,     1,     1,    -1,     1,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,     1,     1 },
/*   StatementList*/ {   2,     2,     2,     2,    -1,     2,     2,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     2,    -1,    -1,    -1,     2,     3 },
/*       Statement*/ {   4,     4,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1 },
/*      AssignStmt*/ {   8,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1 },
/*        DeclStmt*/ {   9,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1 },
/*        DeclTail*/ {  -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    11 },
/*    FuncCallStmt*/ {  -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1 },
/*         ArgList*/ {  13,    13,    13,    -1,    -1,    13,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    13,    14,    -1,    -1,    13,    14 },
/*         ArgTail*/ {  -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    15,    -1,    -1,    -1 },
/*      ReturnStmt*/ {  -1,    -1,    -1,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1 },
/*            Type*/ {  18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1 },
/*      Expression*/ {  20,    20,    20,    -1,    -1,    20,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    20,    -1 },
/* ExpressionPrime*/ {  -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    23,    23,    23,    -1,    23 },
/*         RelExpr*/ {  24,    24,    24,    -1,    -1,    24,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    24,    -1 },
/*       RelOpTail*/ {  -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    25,    25,    25,    25,    25,    -1,    -1,    -1,    26,    26,    26,    -1,    26 },
/*           RelOp*/ {  -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    30,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1 },
/*       ArithExpr*/ {  33,    33,    33,    -1,    -1,    33,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    33,    -1 },
/*  ArithExprPrime*/ {  -1,    -1,    -1,    -1,    -1,    34,    35,    -1,    -1,    36,    36,    36,    36,    36,    36,    36,    36,    -1,    36,    36,    36,    -1,    36 },
/*            Term*/ {  37,    37,    37,    -1,    -1,    37,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,    -1,    -1,    37,    -1 },
/*       TermPrime*/ {  -1,    -1,    -1,    -1,    -1,    40,    40,    38,    39,    40,    40,    40,    40,    40,    40,    40,    40,    -1,    40,    40,    40,    -1,    40 },
/*          Factor*/ {  41,    41,    42,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    43,    -1,    -1 },
/*         UnaryOp*/ {  -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1 }
};


typedef struct {
    char lhs[maxSymbolLength];
    int productionCount;
    char rhs[10][10][maxSymbolLength];
    int productionIndex[10];
} productionRule;


productionRule grammar[100];
int productionCount = 0;

// Token Types
typedef enum {
  TOKEN_KEYWORD,
  TOKEN_LITERAL,
  TOKEN_OPERATOR,
  TOKEN_SEPARATOR,
  TOKEN_IDENTIFIER
} TokenType;

// Token keyword
const char* keys_array[] = {
  "auto", "break", "case", "char",
  "const", "continue", "default", "do",
  "double", "else", "enum", "extern",
  "float", "for", "goto", "if",
  "int", "long", "register", "return",
  "short", "signed", "sizeof", "static",
  "struct", "switch", "typedef", "union",
  "unsigned", "void", "volatile", "while"
};

// Token Structures
typedef struct {
  char lexeme[32];
} TokenKeyword;

typedef struct {
  char lexeme[32];
} TokenIdentifier;

typedef struct {
  char value[64];  // Changed to hold string literals
} TokenLiteral;

typedef struct {
  char op[5];
} TokenOperator;

typedef struct {
  char symbol;
} TokenSeparator;

// Unified Token Structure
typedef struct {
  TokenType type;
  int lineNumber;
  union {
    TokenKeyword keyword;
    TokenIdentifier identifier;
    TokenLiteral literal;
    TokenOperator operator;
    TokenSeparator separator;
  } data;
} Token;


const char* nonterminals[] = {
    "Program", "StatementList", "Statement", "AssignStmt", "DeclStmt",
    "DeclTail", "FuncCallStmt", "ArgList", "ArgTail", "ReturnStmt",
    "Type", "Expression", "ExpressionPrime", "RelExpr", "RelOpTail",
    "RelOp", "ArithExpr", "ArithExprPrime", "Term", "TermPrime",
    "Factor", "UnaryOp", NULL  // NULL for end marker
};

const char* terminals[] = {
    "int", "float", "id", "return", "=", "+", "-", "*", "/",
    "<", "<=", ">", ">=", "==", "!=", "&&", "||",
    "(", ")", ",", ";", "literal", "$", NULL
};


typedef struct tokenNode{
  Token token;
  struct tokenNode* next;
}tokenNode;




// Function Prototypes
int getTerminalIndex(const char* sym);
int getNonTerminalIndex(const char* sym);
const char* getTokenString(Token t);
char (*getFlatIndex(productionRule* grammar, int prodNum))[maxSymbolLength];
int parsingTableChecking(tokenNode* head);

// Stack creation

typedef struct {
    char* character[sz];
    int top;
} stack;

int isEmpty(stack *p) {
    return p->top == -1;
}

int isFull(stack *p) {
    return p->top == sz - 1;
}

void push(stack *p, char* chars) {
    if (isFull(p)) {
        printf("Stack Overflow! Cannot push '%s'\n", chars);
        exit(EXIT_FAILURE);  // Exit on overflow
    }
    p->top++;
    p->character[p->top] = strdup(chars); // strdup allocates and copies string
}

char* pop(stack *p) {
    if (isEmpty(p)) {
        printf("Stack Underflow!\n");
        exit(EXIT_FAILURE);  // Exit on underflow
    }
    char* chars = p->character[p->top];
    p->top--;
    return chars;
}

char* peek(stack *p) {
    if (isEmpty(p)) {
        return NULL;
    }
    return p->character[p->top];
}

void initialize(stack *p) {
    p->top = -1;
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

tokenNode* append(Token T, tokenNode* head){
  tokenNode* tail;
  tail = head;
  tokenNode* newNode = (tokenNode*)malloc(sizeof(tokenNode));
  newNode->token = T;
  newNode->next = NULL;
  if (head == NULL) {
    return newNode;
  }
  while (tail->next != NULL) {
    tail = tail->next;
  }
  tail->next = newNode;
  return head;
}
//////////////////////////////////////////////////////////////////////////////

void freeTokenList(tokenNode* head) {
    tokenNode* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

/////////////////////////////////////////////////////////////////////////////

tokenNode* check(char *content, int lineNumber) {
  int i = 0, j = 0;  // Initialize i and j
  int val;
  int n;
  Token key;

  tokenNode* head = NULL; // Head Initialize

  // Loop till content ends
  while (content[i] != '\0') {

    // Check alphabet
    if (isalpha(content[i]) || content[i] == '_') {
      char compare[32];
      int flag = 0;
      j = i; n = 0;
      while (isalpha(content[i]) || isdigit(content[i]) || content[i] == '_') i++;
      for (int m = j; m < i; m++) {
        compare[n++] = content[m];  // Put it in another string
      }
      compare[n] = '\0';
      for (int cnt = 0; cnt <= 31; cnt++) {
        if (strcmp(compare, keys_array[cnt]) == 0) {
          key.type = TOKEN_KEYWORD;
          strcpy(key.data.keyword.lexeme, compare);
          flag = 1;
        }
      }
      if (flag == 0) {
        key.type = TOKEN_IDENTIFIER;
        strcpy(key.data.identifier.lexeme, compare);
      }
      key.lineNumber = lineNumber;
      head = append(key, head);
    }

    // Check digit
    else if (isdigit(content[i])) {
      j = i;
      key.data.literal.value[0] = '\0'; // Initialize value as empty string
      while (isdigit(content[i])) i++;
      key.type = TOKEN_LITERAL;
      for (int m = j; m < i; m++) {
        val = content[m] - '0';
        char numStr[2] = {content[m], '\0'};
        strcat(key.data.literal.value, numStr);
      }
      key.lineNumber = lineNumber;
      head = append(key, head);
    }

    // Check space
    else if (isspace(content[i])) {
      i++;
    }

    // Check for string literals
    else if (content[i] == '"') {
      key.type = TOKEN_LITERAL;
      char strVal[64];
      int m = 0;
      i++;
      while (content[i] != '"' && content[i] != '\0') {
        strVal[m++] = content[i++];
      }
      strVal[m] = '\0';
      if (content[i] == '"') i++;
      strcpy(key.data.literal.value, strVal);
      key.lineNumber = lineNumber;
      head = append(key, head);
    }

    // Check Operators (e.g. +, -, *, /, etc.)


    else if (content[i] == '+') {
      key.type = TOKEN_OPERATOR;
      strcpy(key.data.operator.op, "+");
      i++;
      key.lineNumber = lineNumber;
      head = append(key, head);
    }
    else if (content[i] == '-') {
      key.type = TOKEN_OPERATOR;
      strcpy(key.data.operator.op, "-");
      i++;
      key.lineNumber = lineNumber;
      head = append(key, head);
    }
    else if (content[i] == '=') {
      key.type = TOKEN_OPERATOR;
      if (content[i + 1] == '=') {
        strcpy(key.data.operator.op, "==");
        i += 2;
      }
      else {
        strcpy(key.data.operator.op, "=");
        i++;
      }
      key.lineNumber = lineNumber;
      head = append(key, head);
    }
    else if (content[i] == '*') {
      key.type = TOKEN_OPERATOR;
      strcpy(key.data.operator.op, "*");
      i++;
      key.lineNumber = lineNumber;
      head = append(key, head);
    }
    else if (content[i] == '/') {
      key.type = TOKEN_OPERATOR;
      strcpy(key.data.operator.op, "/");
      i++;
      key.lineNumber = lineNumber;
      head = append(key, head);
    }
    else if (content[i] == '<') {
      key.type = TOKEN_OPERATOR;
      if (content[i + 1] == '=') {
        strcpy(key.data.operator.op, "<=");
        i += 2;
      }
      else {
        strcpy(key.data.operator.op, "<");
        i++;
      }
      key.lineNumber = lineNumber;
      head = append(key, head);
    }
    else if (content[i] == '>') {
      key.type = TOKEN_OPERATOR;
      if (content[i + 1] == '=') {
        strcpy(key.data.operator.op, ">=");
        i += 2;
      }
      else {
        strcpy(key.data.operator.op, ">");
        i++;
      }
      key.lineNumber = lineNumber;
      head = append(key, head);
    }

    else if (content[i] == '!' && content[i+1] == '=') {
      key.type = TOKEN_OPERATOR;
      strcpy(key.data.operator.op, "!=");
      i += 2;
      key.lineNumber = lineNumber;
      head = append(key, head);
    }

    // Check separators (e.g. ;, (, ))
    else {
      switch(content[i]) {
        case '(':
        case ')':
        case ';':
        case ',':
          key.type = TOKEN_SEPARATOR;
          key.data.separator.symbol = content[i];
          key.lineNumber = lineNumber;
          head = append(key, head);
          i++;
          break;
        default:
          i++;
          break;
      }
    }
  }
  return head;
}

//////////////////////////////////////////////////////////////////////////////


//Parser
void ParsingTableInitialized() {
    int flatIndexCounter = 1;

    strcpy(grammar[productionCount].lhs, "Program");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "StatementList");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "StatementList");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "Statement");
    strcpy(grammar[productionCount].rhs[0][1], "StatementList");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "ε");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "Statement");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "DeclStmt");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "AssignStmt");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "ReturnStmt");
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "AssignStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "id");
    strcpy(grammar[productionCount].rhs[0][1], "=");
    strcpy(grammar[productionCount].rhs[0][2], "Expression");
    strcpy(grammar[productionCount].rhs[0][3], ";");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "DeclStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "Type");
    strcpy(grammar[productionCount].rhs[0][1], "id");
    strcpy(grammar[productionCount].rhs[0][2], "DeclTail");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "DeclTail");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], ";");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "=");
    strcpy(grammar[productionCount].rhs[1][1], "Expression");
    strcpy(grammar[productionCount].rhs[1][2], ";");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "FuncCallStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "id");
    strcpy(grammar[productionCount].rhs[0][1], "(");
    strcpy(grammar[productionCount].rhs[0][2], "ArgList");
    strcpy(grammar[productionCount].rhs[0][3], ")");
    strcpy(grammar[productionCount].rhs[0][4], ";");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "ArgList");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "Expression");
    strcpy(grammar[productionCount].rhs[0][1], "ArgTail");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "ε");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "ArgTail");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], ",");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], "ArgTail");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "ε");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "ReturnStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "return");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], ";");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "Type");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "int");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "float");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "Expression");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "RelExpr");
    strcpy(grammar[productionCount].rhs[0][1], "ExpressionPrime");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "ExpressionPrime");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "&&");
    strcpy(grammar[productionCount].rhs[0][1], "RelExpr");
    strcpy(grammar[productionCount].rhs[0][2], "ExpressionPrime");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "||");
    strcpy(grammar[productionCount].rhs[1][1], "RelExpr");
    strcpy(grammar[productionCount].rhs[1][2], "ExpressionPrime");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "ε");
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "RelExpr");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "ArithExpr");
    strcpy(grammar[productionCount].rhs[0][1], "RelOpTail");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "RelOpTail");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "RelOp");
    strcpy(grammar[productionCount].rhs[0][1], "ArithExpr");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "ε");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "RelOp");
    grammar[productionCount].productionCount = 6;
    strcpy(grammar[productionCount].rhs[0][0], "<");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "<=");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], ">");
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[3][0], ">=");
    grammar[productionCount].productionIndex[3] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[4][0], "==");
    grammar[productionCount].productionIndex[4] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[5][0], "!=");
    grammar[productionCount].productionIndex[5] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "ArithExpr");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "Term");
    strcpy(grammar[productionCount].rhs[0][1], "ArithExprPrime");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "ArithExprPrime");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "+");
    strcpy(grammar[productionCount].rhs[0][1], "Term");
    strcpy(grammar[productionCount].rhs[0][2], "ArithExprPrime");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "-");
    strcpy(grammar[productionCount].rhs[1][1], "Term");
    strcpy(grammar[productionCount].rhs[1][2], "ArithExprPrime");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "ε");
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "Term");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "Factor");
    strcpy(grammar[productionCount].rhs[0][1], "TermPrime");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "TermPrime");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "*");
    strcpy(grammar[productionCount].rhs[0][1], "Factor");
    strcpy(grammar[productionCount].rhs[0][2], "TermPrime");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "/");
    strcpy(grammar[productionCount].rhs[1][1], "Factor");
    strcpy(grammar[productionCount].rhs[1][2], "TermPrime");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "ε");
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "Factor");
    grammar[productionCount].productionCount = 4;
    strcpy(grammar[productionCount].rhs[0][0], "(");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], ")");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "id");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "literal");
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[3][0], "UnaryOp");
    strcpy(grammar[productionCount].rhs[3][1], "Factor");
    grammar[productionCount].productionIndex[3] = flatIndexCounter++;
    productionCount++;

    strcpy(grammar[productionCount].lhs, "UnaryOp");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "-");
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "+");
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "!");
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;
}

int parsingTableChecking(tokenNode* head){

  stack s;
  Token ptr = head->token;


  //parsing table formation

  int lookupValue = -1;

  initialize(&s);

  push(&s, "$");
  push(&s, "Program");


// LL(1) Parsing

while (strcmp(peek(&s), "$") != 0) {
    char* temp = pop(&s);

    if (strcmp(getTokenString(ptr), temp) == 0) {
        head = head->next;
        ptr = head->token;
    } else {
        if (getNonTerminalIndex(temp) != -1) {
            int nonTerminalIndex = getNonTerminalIndex(temp);
            int TerminalIndex = getTerminalIndex(getTokenString(ptr));
            lookupValue = parsingTable[nonTerminalIndex][TerminalIndex];

            if (lookupValue == -1) {
                printf("Syntax Error at line %d: unexpected token '%s' in '%s'\n", ptr.lineNumber, getTokenString(ptr), temp);
                return 0;
            }

            char (*rhs)[maxSymbolLength] = getFlatIndex(grammar, lookupValue);
            int count = 0;

            while (rhs[count][0] != '\0') {
                count++;
              }

                // Push RHS in reverse order (if not ε)
                if (count == 1 && strcmp(rhs[0], "ε") == 0) {
                    // do nothing, ε production
                } else {
                    for (int i = count - 1; i >= 0; i--) {
                        push(&s, rhs[i]);
                    }
                }

        } else {
            // temp is terminal, but it doesn't match input
            printf("Syntax Error at line %d: expected '%s' but found '%s'\n", ptr.lineNumber, temp, getTokenString(ptr));
            return 0;
        }
    }
}

  if (strcmp(peek(&s), "$") == 0 && head->token.type == TOKEN_SEPARATOR && head->token.data.separator.symbol == '$') {
    printf("Input Accepted by the Grammar\n");
    return 1;
} else {
    printf("Syntax Error: Input Rejected\n");
    return 0;
}

}

char (*getFlatIndex(productionRule* grammar, int prodNum))[maxSymbolLength] {
    int flatIndex = 0;
    for (int i = 0 ; i < 100 ; i++ )
        for (int j = 0; j < grammar[i].productionCount ; j++) {
            if (flatIndex == prodNum - 1) {
                return grammar[i].rhs[j];
            }
            flatIndex++;
        }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////

const char* getTokenString(Token t) {
    static char buffer[64];  // Safe size
    switch (t.type) {
        case TOKEN_KEYWORD:
            strcpy(buffer, t.data.keyword.lexeme);  // e.g. "int"
            return buffer;
        case TOKEN_OPERATOR:
            strcpy(buffer, t.data.operator.op);     // e.g. "+"
            return buffer;
        case TOKEN_IDENTIFIER:
            strcpy(buffer, "id");  // ← FIXED
            return buffer;
        case TOKEN_LITERAL:
            strcpy(buffer, "literal");  // ← FIXED
            return buffer;
        case TOKEN_SEPARATOR:
            buffer[0] = t.data.separator.symbol;
            buffer[1] = '\0';
            return buffer;
        default:
            return "UNKNOWN";
    }
}

//////////////////////////////////////////////////////////////////////////////



int getTerminalIndex(const char* sym){
for (int i = 0; i < num_terminal; i++) {
  if(strcmp(sym,terminals[i])==0)
    return i;
}
return -1;
}

//////////////////////////////////////////////////////////////////////////////


int getNonTerminalIndex(const char* sym){
for (int i = 0; i < num_nonTerminal; i++) {
  if(strcmp(sym,nonterminals[i])==0)
    return i;
}
return -1;
}


//////////////////////////////////////////////////////////////////////////////

void main() {
    tokenNode* head = NULL;
    tokenNode* temp;
    FILE* fptr;
    fptr = fopen("exit1.txt", "r");
    if (!fptr) {
        printf("Failed to open input file.\n");
        return;
    }

    char content[100];
    char fullContent[4096] = "";  // Buffer to store the entire file content
    int lineNumber = 1;

    ParsingTableInitialized();  // Initialize grammar and parsing table

    // Read and accumulate the full content of the file
    while (fgets(content, sizeof(content), fptr)) {
        strcat(fullContent, content);
    }
    fclose(fptr);

    // Tokenize the entire content
    head = check(fullContent, lineNumber);

    // Append the end-of-input marker ('$') only once
    Token endToken;
    endToken.type = TOKEN_SEPARATOR;
    endToken.data.separator.symbol = '$';
    head = append(endToken, head);

    // Print tokens (optional, for debugging)
    printf("\n----- Token List -----\n");
    temp = head;
    while (temp != NULL) {
        Token t = temp->token;
        switch (t.type) {
            case TOKEN_KEYWORD:
                printf("Keyword: %s\n", t.data.keyword.lexeme);
                break;
            case TOKEN_LITERAL:
                printf("Literal: %s\n", t.data.literal.value);
                break;
            case TOKEN_OPERATOR:
                printf("Operator: %s\n", t.data.operator.op);
                break;
            case TOKEN_SEPARATOR:
                if (t.data.separator.symbol == '$')
                    printf("End Marker: $\n");
                else
                    printf("Separator: %c\n", t.data.separator.symbol);
                break;
            case TOKEN_IDENTIFIER:
                printf("Identifier: %s\n", t.data.identifier.lexeme);
                break;
        }
        temp = temp->next;
    }

    printf("TokenString: %s\n", getTokenString(temp->token));


    // Run the parser
    parsingTableChecking(head);

    // Clean up memory
    freeTokenList(head);
}
