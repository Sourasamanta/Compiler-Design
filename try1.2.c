#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define sz 100
#define maxProduction 100
#define maxSymbolPerProduction 10
#define maxSymbolLength 16
#define num_nonTerminal 24
#define num_terminal 28

typedef enum {
    T_INT, T_FLOAT, T_ID, T_RETURN, T_IF, T_ELSE, T_ASSIGN, T_PLUS, T_MINUS,
    T_MUL, T_DIV, T_LT, T_LE, T_GT, T_GE, T_EQ, T_NEQ, T_AND, T_OR,
    T_LPAREN, T_RPAREN, T_COMMA, T_SEMICOLON, T_LITERAL, T_DOLLAR,
    T_LBRACE, T_RBRACE, T_WHILE,
    T_COUNT
} Terminal;

typedef enum {
    NT_PROGRAM, NT_STATEMENTLIST, NT_STATEMENT, NT_STATEMENTSUFFIX, NT_DECLTAIL,
    NT_FUNCCALLSTMT, NT_ARGLIST, NT_ARGTAIL, NT_ELSEPART, NT_RETURNSTMT,
    NT_TYPE, NT_EXPRESSION, NT_EXPRESSIONPRIME, NT_RELEXPR, NT_RELOPTAIL,
    NT_RELOP, NT_ARITHEXPR, NT_ARITHEXPRPRIME, NT_TERM, NT_TERMPRIME,
    NT_FACTOR, NT_UNARYOP, NT_BLOCK, NT_FUNCCALL,
    NT_COUNT
} NonTerminal;

typedef struct {
    char lhs[maxSymbolLength];
    int productionCount;
    char rhs[10][10][maxSymbolLength];
    int productionIndex[10];
} productionRule;

int flatIndexCounter = 1;
int productionCount = 0;
productionRule grammar[100];

typedef enum {
    TOKEN_KEYWORD, TOKEN_LITERAL, TOKEN_OPERATOR, TOKEN_SEPARATOR, TOKEN_IDENTIFIER
} TokenType;

typedef struct {
    char lexeme[32];
} TokenKeyword;

typedef struct {
    char lexeme[32];
} TokenIdentifier;

typedef struct {
    char value[64];
} TokenLiteral;

typedef struct {
    char op[5];
} TokenOperator;

typedef struct {
    char symbol;
} TokenSeparator;

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

typedef struct tokenNode {
    Token token;
    struct tokenNode* next;
} tokenNode;

typedef struct tree {
    char symbol[maxSymbolLength];
    int childCount;
    struct tree* children[10];
    Token token;
    int isTokenPresent;
} node;

typedef struct {
    char* symbol;
    int isTerminal;
    node* treeNode;
} StackItem;

typedef struct {
    StackItem items[sz];
    int top;
} stack;

const char* nonterminals[] = {
    "Program", "StatementList", "Statement", "StatementSuffix", "DeclTail",
    "FuncCallStmt", "ArgList", "ArgTail", "ElsePart", "ReturnStmt", "Type",
    "Expression", "ExpressionPrime", "RelExpr", "RelOpTail", "RelOp",
    "ArithExpr", "ArithExprPrime", "Term", "TermPrime", "Factor", "UnaryOp", "Block", "FuncCall", NULL
};

const char* terminals[] = {
    "int", "float", "id", "return", "if", "else", "=", "+", "-", "*", "/",
    "<", "<=", ">", ">=", "==", "!=", "&&", "||", "(", ")", ",", ";",
    "literal", "$", "{", "}", "while", NULL
};

int parsingTable[num_nonTerminal][num_terminal] = {
    /* Program */         {  1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1,  1},
    /* StatementList */   {  2,  2,  2,  2,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  3, -1,  3,  3,  3,  2},
    /* Statement */       {  4,  4,  5,  6,  7, -1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1,  9},
    /* StatementSuffix */ { -1, -1, -1, -1, -1, -1, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 12, -1, -1, -1, -1, -1},
    /* DeclTail */       { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 14, -1, -1, -1, -1, -1},
    /* FuncCallStmt */   { -1, -1, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* ArgList */        { 16, 16, 16, -1, -1, -1, -1, 16, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16, 17, -1, -1, 16, -1, -1, -1, -1},
    /* ArgTail */        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 18, 19, 18, -1, -1, -1, -1, -1, -1},
    /* ElsePart */       { -1, -1, -1, -1, -1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 21, -1, 21, 21, 21, 21},
    /* ReturnStmt */     { -1, -1, -1, 22, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* Type */           { 23, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* Expression */     { 25, 25, 25, -1, -1, -1, -1, 25, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25, -1, -1, -1, 25, -1, -1, -1, -1},
    /* ExpressionPrime */ { -1, -1, -1, -1, -1, 26, 27, -1, -1, -1, -1, 26, 26, 26, 26, 26, -1, 26, 27, -1, 28, 28, 28, -1, -1, -1, 28, -1},
    /* RelExpr */        { 29, 29, 29, -1, -1, -1, -1, 29, 29, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 29, -1, -1, -1, 29, -1, -1, -1, -1},
    /* RelOpTail */      { -1, -1, -1, -1, -1, 30, -1, -1, -1, -1, -1, 30, 30, 30, 30, 30, 31, 31, -1, 31, 31, 31, 31, -1, 31, -1, 31, -1},
    /* RelOp */          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 32, 34, 33, 35, 36, 37, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* ArithExpr */      { 38, 38, 38, -1, -1, -1, -1, 38, 38, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 38, -1, -1, -1, 38, -1, -1, -1, -1},
    /* ArithExprPrime */ { -1, -1, -1, -1, -1, -1, -1, 39, 40, -1, -1, 41, 41, 41, 41, 41, 41, 41, -1, 41, 41, 41, 41, -1, 41, -1, 41, -1},
    /* Term */           { 42, 42, 42, -1, -1, -1, -1, 42, 42, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 42, -1, -1, -1, 42, -1, -1, -1, -1},
    /* TermPrime */      { -1, -1, -1, -1, -1, -1, -1, 45, 44, 43, 45, 45, 45, 45, 45, 45, 45, 45, -1, 45, 45, 45, 45, -1, 45, -1, 45, -1},
    /* Factor */         { 46, 46, 47, -1, -1, -1, -1, 48, 49, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 46, -1, -1, -1, 48, -1, -1, -1, -1},
    /* UnaryOp */        { -1, -1, -1, -1, -1, -1, -1, 51, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    /* Block */          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 54, -1, -1},
    /* FuncCall */       { -1, -1, 55, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};

void initStack(stack* s);
Token peekNextToken(tokenNode** current);
Terminal getTerminalEnum(const char* sym, Token token);
NonTerminal getNonTerminalEnum(const char* sym);
const char* getTokenString(Token t);
char* cleanTokenString(const char* str);
char (*getFlatIndex(productionRule* grammar, int prodNum))[maxSymbolLength];
int parsingTableChecking(tokenNode* head, node** p);
Token getNextToken(tokenNode** current);
void ungetToken(Token token);
int isNonTerminal(const char* symbol);
void ParsingTableInitialized(void);

int isEmpty(stack* p) {
    return p->top == -1;
}

int isFull(stack* p) {
    return p->top == sz - 1;
}

void push(stack* p, char* chars, node* treeNode) {
    if (isFull(p)) {
        printf("Stack Overflow! Cannot push '%s'\n", chars);
        exit(EXIT_FAILURE);
    }
    p->items[++(p->top)].symbol = strdup(chars);
    p->items[p->top].treeNode = treeNode;
    p->items[p->top].isTerminal = isNonTerminal(chars) ? 0 : 1;
}

void freeNode(node* n) {
    if (n == NULL) return;
    for (int i = 0; i < n->childCount; i++) {
        freeNode(n->children[i]);
    }
    free(n);
}

StackItem pop(stack* p) {
    if (isEmpty(p)) {
        printf("Stack Underflow!\n");
        exit(EXIT_FAILURE);
    }
    StackItem item = p->items[p->top--];
    return item;
}

StackItem peek(stack* p) {
    StackItem empty = {NULL, 0, NULL};
    if (isEmpty(p)) {
        return empty;
    }
    return p->items[p->top];
}

void initStack(stack* p) {
    p->top = -1;
}

int isNonTerminal(const char* symbol) {
    for (int i = 0; nonterminals[i] != NULL; i++) {
        if (strcmp(symbol, nonterminals[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

node* create(const char* temp) {
    node* p = (node*)malloc(sizeof(node));
    if (p == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(p->symbol, temp);
    p->childCount = 0;
    for (int i = 0; i < 10; i++) {
        p->children[i] = NULL;
    }
    p->isTokenPresent = 0;
    memset(&(p->token), 0, sizeof(Token));
    p->token.lineNumber = -1;
    return p;
}

tokenNode* append(Token T, tokenNode* head) {
    tokenNode* newNode = (tokenNode*)malloc(sizeof(tokenNode));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->token = T;
    newNode->next = NULL;
    if (head == NULL) {
        return newNode;
    }
    tokenNode* tail = head;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = newNode;
    return head;
}

void freeTokenList(tokenNode* head) {
    tokenNode* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

tokenNode* check(const char* content, int lineNumber) {
    tokenNode* head = NULL;
    int i = 0;

    while (content[i] != '\0') {
        if (isspace(content[i])) {
            if (content[i] == '\n') lineNumber++;
            i++;
            continue;
        }

        Token t;
        t.lineNumber = lineNumber;

        if (isalpha(content[i]) || content[i] == '_') {
            int start = i;
            while (isalnum(content[i]) || content[i] == '_') i++;
            int len = i - start;

            char buffer[64];
            strncpy(buffer, &content[start], len);
            buffer[len] = '\0';

            if (strcmp(buffer, "int") == 0 || strcmp(buffer, "float") == 0 ||
                strcmp(buffer, "return") == 0 || strcmp(buffer, "if") == 0 ||
                strcmp(buffer, "else") == 0 || strcmp(buffer, "while") == 0) {
                t.type = TOKEN_KEYWORD;
                strcpy(t.data.keyword.lexeme, buffer);
                } else {
                t.type = TOKEN_IDENTIFIER;
                strcpy(t.data.identifier.lexeme, buffer);
                }
            head = append(t, head);
        } else if (isdigit(content[i])) {
            int start = i;
            int hasDot = 0;

            while (isdigit(content[i])) i++;
            if (content[i] == '.' && isdigit(content[i + 1])) {
                hasDot = 1;
                i++;
                while (isdigit(content[i])) i++;
            }

            int len = i - start;
            strncpy(t.data.literal.value, &content[start], len);
            t.data.literal.value[len] = '\0';
            t.type = TOKEN_LITERAL;
            head = append(t, head);
        } else if ((content[i] == '>' || content[i] == '<' ||
                    content[i] == '=' || content[i] == '!' ||
                    content[i] == '&' || content[i] == '|') &&
                   content[i + 1] == '=') {
            t.type = TOKEN_OPERATOR;
            t.data.operator.op[0] = content[i];
            t.data.operator.op[1] = '=';
            t.data.operator.op[2] = '\0';
            i += 2;
            head = append(t, head);
        } else if ((content[i] == '&' && content[i + 1] == '&') ||
                   (content[i] == '|' && content[i + 1] == '|')) {
            t.type = TOKEN_OPERATOR;
            t.data.operator.op[0] = content[i];
            t.data.operator.op[1] = content[i + 1];
            t.data.operator.op[2] = '\0';
            i += 2;
            head = append(t, head);
        } else if (strchr("+-*/<>=!", content[i])) {
            t.type = TOKEN_OPERATOR;
            t.data.operator.op[0] = content[i];
            t.data.operator.op[1] = '\0';
            i++;
            head = append(t, head);
        } else if (strchr("();{},", content[i])) {
            t.type = TOKEN_SEPARATOR;
            t.data.separator.symbol = content[i];
            i++;
            head = append(t, head);
        } else {
            i++;
        }
    }
    return head;
}

void ParsingTableInitialized(void) {
    flatIndexCounter = 1;
    productionCount = 0;

    // 1: Program -> StatementList
    strcpy(grammar[productionCount].lhs, "Program");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "StatementList");
    grammar[productionCount].rhs[0][1][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 2-3: StatementList -> Statement StatementList | epsilon
    strcpy(grammar[productionCount].lhs, "StatementList");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "Statement");
    strcpy(grammar[productionCount].rhs[0][1], "StatementList");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "epsilon");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 4-10: Statement -> Type id StatementSuffix | FuncCallStmt | ReturnStmt | if ( Expression ) Statement ElsePart | id = Expression ; | while ( Expression ) Block | Block
    strcpy(grammar[productionCount].lhs, "Statement");
    grammar[productionCount].productionCount = 7;
    strcpy(grammar[productionCount].rhs[0][0], "Type");
    strcpy(grammar[productionCount].rhs[0][1], "id");
    strcpy(grammar[productionCount].rhs[0][2], "StatementSuffix");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "FuncCallStmt");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "ReturnStmt");
    grammar[productionCount].rhs[2][1][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[3][0], "if");
    strcpy(grammar[productionCount].rhs[3][1], "(");
    strcpy(grammar[productionCount].rhs[3][2], "Expression");
    strcpy(grammar[productionCount].rhs[3][3], ")");
    strcpy(grammar[productionCount].rhs[3][4], "Statement");
    strcpy(grammar[productionCount].rhs[3][5], "ElsePart");
    grammar[productionCount].rhs[3][6][0] = '\0';
    grammar[productionCount].productionIndex[3] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[4][0], "id");
    strcpy(grammar[productionCount].rhs[4][1], "=");
    strcpy(grammar[productionCount].rhs[4][2], "Expression");
    strcpy(grammar[productionCount].rhs[4][3], ";");
    grammar[productionCount].rhs[4][4][0] = '\0';
    grammar[productionCount].productionIndex[4] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[5][0], "while");
    strcpy(grammar[productionCount].rhs[5][1], "(");
    strcpy(grammar[productionCount].rhs[5][2], "Expression");
    strcpy(grammar[productionCount].rhs[5][3], ")");
    strcpy(grammar[productionCount].rhs[5][4], "Block");
    grammar[productionCount].rhs[5][5][0] = '\0';
    grammar[productionCount].productionIndex[5] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[6][0], "Block");
    grammar[productionCount].rhs[6][1][0] = '\0';
    grammar[productionCount].productionIndex[6] = flatIndexCounter++;
    productionCount++;

    // 11-12: StatementSuffix -> = Expression ; | DeclTail ;
    strcpy(grammar[productionCount].lhs, "StatementSuffix");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "=");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], ";");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "DeclTail");
    strcpy(grammar[productionCount].rhs[1][1], ";");
    grammar[productionCount].rhs[1][2][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 13-14: DeclTail -> , id DeclTail | epsilon
    strcpy(grammar[productionCount].lhs, "DeclTail");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], ",");
    strcpy(grammar[productionCount].rhs[0][1], "id");
    strcpy(grammar[productionCount].rhs[0][2], "DeclTail");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "epsilon");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 15: FuncCallStmt -> FuncCall ;
    strcpy(grammar[productionCount].lhs, "FuncCallStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "FuncCall");
    strcpy(grammar[productionCount].rhs[0][1], ";");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 16-17: ArgList -> Expression ArgTail | epsilon
    strcpy(grammar[productionCount].lhs, "ArgList");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "Expression");
    strcpy(grammar[productionCount].rhs[0][1], "ArgTail");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "epsilon");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 18-19: ArgTail -> , Expression ArgTail | epsilon
    strcpy(grammar[productionCount].lhs, "ArgTail");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], ",");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], "ArgTail");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "epsilon");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 20-21: ElsePart -> else Statement | epsilon
    strcpy(grammar[productionCount].lhs, "ElsePart");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "else");
    strcpy(grammar[productionCount].rhs[0][1], "Statement");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "epsilon");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 22: ReturnStmt -> return Expression ;
    strcpy(grammar[productionCount].lhs, "ReturnStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "return");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], ";");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 23-24: Type -> int | float
    strcpy(grammar[productionCount].lhs, "Type");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "int");
    grammar[productionCount].rhs[0][1][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "float");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 25: Expression -> RelExpr ExpressionPrime
    strcpy(grammar[productionCount].lhs, "Expression");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "RelExpr");
    strcpy(grammar[productionCount].rhs[0][1], "ExpressionPrime");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 26-28: ExpressionPrime -> && RelExpr ExpressionPrime | || RelExpr ExpressionPrime | epsilon
    strcpy(grammar[productionCount].lhs, "ExpressionPrime");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "&&");
    strcpy(grammar[productionCount].rhs[0][1], "RelExpr");
    strcpy(grammar[productionCount].rhs[0][2], "ExpressionPrime");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "||");
    strcpy(grammar[productionCount].rhs[1][1], "RelExpr");
    strcpy(grammar[productionCount].rhs[1][2], "ExpressionPrime");
    grammar[productionCount].rhs[1][3][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "epsilon");
    grammar[productionCount].rhs[2][1][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    // 29: RelExpr -> ArithExpr RelOpTail
    strcpy(grammar[productionCount].lhs, "RelExpr");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "ArithExpr");
    strcpy(grammar[productionCount].rhs[0][1], "RelOpTail");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 30-31: RelOpTail -> RelOp ArithExpr RelOpTail | epsilon
    strcpy(grammar[productionCount].lhs, "RelOpTail");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "RelOp");
    strcpy(grammar[productionCount].rhs[0][1], "ArithExpr");
    strcpy(grammar[productionCount].rhs[0][2], "RelOpTail");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "epsilon");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 32-37: RelOp -> < | > | <= | >= | == | !=
    strcpy(grammar[productionCount].lhs, "RelOp");
    grammar[productionCount].productionCount = 6;
    strcpy(grammar[productionCount].rhs[0][0], "<");
    grammar[productionCount].rhs[0][1][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], ">");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "<=");
    grammar[productionCount].rhs[2][1][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[3][0], ">=");
    grammar[productionCount].rhs[3][1][0] = '\0';
    grammar[productionCount].productionIndex[3] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[4][0], "==");
    grammar[productionCount].rhs[4][1][0] = '\0';
    grammar[productionCount].productionIndex[4] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[5][0], "!=");
    grammar[productionCount].rhs[5][1][0] = '\0';
    grammar[productionCount].productionIndex[5] = flatIndexCounter++;
    productionCount++;

    // 38: ArithExpr -> Term ArithExprPrime
    strcpy(grammar[productionCount].lhs, "ArithExpr");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "Term");
    strcpy(grammar[productionCount].rhs[0][1], "ArithExprPrime");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 39-41: ArithExprPrime -> + Term ArithExprPrime | - Term ArithExprPrime | epsilon
    strcpy(grammar[productionCount].lhs, "ArithExprPrime");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "+");
    strcpy(grammar[productionCount].rhs[0][1], "Term");
    strcpy(grammar[productionCount].rhs[0][2], "ArithExprPrime");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "-");
    strcpy(grammar[productionCount].rhs[1][1], "Term");
    strcpy(grammar[productionCount].rhs[1][2], "ArithExprPrime");
    grammar[productionCount].rhs[1][3][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "epsilon");
    grammar[productionCount].rhs[2][1][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    // 42: Term -> Factor TermPrime
    strcpy(grammar[productionCount].lhs, "Term");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "Factor");
    strcpy(grammar[productionCount].rhs[0][1], "TermPrime");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 43-45: TermPrime -> * Factor TermPrime | / Factor TermPrime | epsilon
    strcpy(grammar[productionCount].lhs, "TermPrime");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "*");
    strcpy(grammar[productionCount].rhs[0][1], "Factor");
    strcpy(grammar[productionCount].rhs[0][2], "TermPrime");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "/");
    strcpy(grammar[productionCount].rhs[1][1], "Factor");
    strcpy(grammar[productionCount].rhs[1][2], "TermPrime");
    grammar[productionCount].rhs[1][3][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "epsilon");
    grammar[productionCount].rhs[2][1][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    // 46-50: Factor -> ( Expression ) | id | literal | UnaryOp Factor | FuncCall
    strcpy(grammar[productionCount].lhs, "Factor");
    grammar[productionCount].productionCount = 5;
    strcpy(grammar[productionCount].rhs[0][0], "(");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], ")");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "id");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "literal");
    grammar[productionCount].rhs[2][1][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[3][0], "UnaryOp");
    strcpy(grammar[productionCount].rhs[3][1], "Factor");
    grammar[productionCount].rhs[3][2][0] = '\0';
    grammar[productionCount].productionIndex[3] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[4][0], "FuncCall");
    grammar[productionCount].rhs[4][1][0] = '\0';
    grammar[productionCount].productionIndex[4] = flatIndexCounter++;
    productionCount++;

    // 51-53: UnaryOp -> - | ! | +
    strcpy(grammar[productionCount].lhs, "UnaryOp");
    grammar[productionCount].productionCount = 3;
    strcpy(grammar[productionCount].rhs[0][0], "-");
    grammar[productionCount].rhs[0][1][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "!");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "+");
    grammar[productionCount].rhs[2][1][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    productionCount++;

    // 54: Block -> { StatementList }
    strcpy(grammar[productionCount].lhs, "Block");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "{");
    strcpy(grammar[productionCount].rhs[0][1], "StatementList");
    strcpy(grammar[productionCount].rhs[0][2], "}");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 55: FuncCall -> id ( ArgList )
    strcpy(grammar[productionCount].lhs, "FuncCall");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "id");
    strcpy(grammar[productionCount].rhs[0][1], "(");
    strcpy(grammar[productionCount].rhs[0][2], "ArgList");
    strcpy(grammar[productionCount].rhs[0][3], ")");
    grammar[productionCount].rhs[0][4][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;
}

Token pushedBackToken;
int hasPushedBack = 0;

Token peekNextToken(tokenNode** current) {
    if (*current == NULL) {
        Token empty = {0};
        empty.lineNumber = -1;
        return empty;
    }
    Token next = (*current)->token;
    return next;
}

Token getNextToken(tokenNode** current) {
    if (hasPushedBack) {
        hasPushedBack = 0;
        return pushedBackToken;
    }
    if (*current == NULL) {
        Token empty = {0};
        empty.lineNumber = -1;
        return empty;
    }
    Token token = (*current)->token;
    *current = (*current)->next;
    return token;
}

void ungetToken(Token token) {
    pushedBackToken = token;
    hasPushedBack = 1;
}

char* cleanTokenString(const char* str) {
    static char buffer[64];
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    int len = strlen(buffer);
    while (len > 0 && isspace(buffer[len - 1])) {
        buffer[len - 1] = '\0';
        len--;
    }
    return buffer;
}

int terminalMatches(const char* terminal, Token token) {
    if (strcmp(terminal, "int") == 0)
        return token.type == TOKEN_KEYWORD && strcmp(cleanTokenString(token.data.keyword.lexeme), "int") == 0;
    if (strcmp(terminal, "float") == 0)
        return token.type == TOKEN_KEYWORD && strcmp(cleanTokenString(token.data.keyword.lexeme), "float") == 0;
    if (strcmp(terminal, "return") == 0)
        return token.type == TOKEN_KEYWORD && strcmp(cleanTokenString(token.data.keyword.lexeme), "return") == 0;
    if (strcmp(terminal, "if") == 0)
        return token.type == TOKEN_KEYWORD && strcmp(cleanTokenString(token.data.keyword.lexeme), "if") == 0;
    if (strcmp(terminal, "else") == 0)
        return token.type == TOKEN_KEYWORD && strcmp(cleanTokenString(token.data.keyword.lexeme), "else") == 0;
    if (strcmp(terminal, "while") == 0)
        return token.type == TOKEN_KEYWORD && strcmp(cleanTokenString(token.data.keyword.lexeme), "while") == 0;
    if (strcmp(terminal, "id") == 0) {
        return token.type == TOKEN_IDENTIFIER;
    }
    if (strcmp(terminal, "literal") == 0) {
        return token.type == TOKEN_LITERAL;
    }
    if (strcmp(terminal, "=") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "=") == 0;
    if (strcmp(terminal, "+") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "+") == 0;
    if (strcmp(terminal, "-") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "-") == 0;
    if (strcmp(terminal, "*") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "*") == 0;
    if (strcmp(terminal, "/") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "/") == 0;
    if (strcmp(terminal, "<") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "<") == 0;
    if (strcmp(terminal, "<=") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "<=") == 0;
    if (strcmp(terminal, ">") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), ">") == 0;
    if (strcmp(terminal, ">=") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), ">=") == 0;
    if (strcmp(terminal, "==") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "==") == 0;
    if (strcmp(terminal, "!=") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "!=") == 0;
    if (strcmp(terminal, "&&") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "&&") == 0;
    if (strcmp(terminal, "||") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "||") == 0;
    if (strcmp(terminal, "!") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "!") == 0;
    if (strcmp(terminal, ";") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == ';';
    if (strcmp(terminal, "(") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == '(';
    if (strcmp(terminal, ")") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == ')';
    if (strcmp(terminal, ",") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == ',';
    if (strcmp(terminal, "{") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == '{';
    if (strcmp(terminal, "}") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == '}';
    if (strcmp(terminal, "$") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == '$';
    return 0;
}

int parsingTableChecking(tokenNode* head, node** p) {
    stack s;
    initStack(&s);
    node* root = create("Program");
    *p = root;
    push(&s, "$", NULL);
    push(&s, "Program", root);
    tokenNode* currentTokenPtr = head;
    Token currentToken = {0};
    if (currentTokenPtr) {
        currentToken = getNextToken(&currentTokenPtr);
    } else {
        printf("Syntax Error: Empty input\n");
        return 0;
    }

    while (!isEmpty(&s)) {
        StackItem item = pop(&s);
        char* topSymbol = item.symbol;
        node* currentNode = item.treeNode;


        int isTerminal = 0;
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(topSymbol, terminals[i]) == 0) {
                isTerminal = 1;
                break;
            }
        }

        if (isTerminal && terminalMatches(topSymbol, currentToken)) {
            if (strcmp(topSymbol, "$") == 0) {
                if (currentToken.type == TOKEN_SEPARATOR && currentToken.data.separator.symbol == '$') {
                    if (currentTokenPtr != NULL) {
                        printf("Syntax Error at line %d: Unexpected tokens after $\n",
                               currentToken.lineNumber);
                        free(topSymbol);
                        return 0;
                    }
                    free(topSymbol);
                    return 1;
                } else {
                    printf("Syntax Error at the end of input: Expected $, got %s\n",
                           getTokenString(currentToken));
                    free(topSymbol);
                    return 0;
                }
            }
            if (currentNode) {
                currentNode->token = currentToken;
                currentNode->isTokenPresent = 1;
            }
            currentToken = getNextToken(&currentTokenPtr);
            free(topSymbol);
            continue;
        }

        if (isNonTerminal(topSymbol)) {
            NonTerminal ntIndex = getNonTerminalEnum(topSymbol);
            Terminal tIndex = -1;
            Token nextToken = {0};
            int useLookahead = 0;
            int production = -1;

            if (currentToken.type == TOKEN_SEPARATOR && currentToken.data.separator.symbol == '(') {
                tIndex = T_LPAREN;
                if (ntIndex == NT_FACTOR) {
                    production = 46; // Factor -> ( Expression )
                }
            } else if (currentToken.type == TOKEN_IDENTIFIER) {
                nextToken = peekNextToken(&currentTokenPtr);
                useLookahead = 1;
                tIndex = T_ID;
            } else if (currentToken.type == TOKEN_LITERAL) {
                tIndex = T_LITERAL;
            } else {
                tIndex = getTerminalEnum(getTokenString(currentToken), currentToken);
            }

            if (tIndex == -1) {
                printf("Syntax Error at line %d: Invalid token %s\n",
                       currentToken.lineNumber, getTokenString(currentToken));
                free(topSymbol);
                return 0;
            }

            if (ntIndex == NT_STATEMENT && tIndex == T_ID && useLookahead) {
                if (nextToken.type == TOKEN_OPERATOR && strcmp(nextToken.data.operator.op, "=") == 0) {
                    production = 8; // Statement -> id = Expression ;
                    printf("Debug: Chose production 8 (Statement -> id = Expression ;)\n");
                } else if (nextToken.type == TOKEN_SEPARATOR && nextToken.data.separator.symbol == '(') {
                    production = 5; // Statement -> FuncCallStmt
                    printf("Debug: Chose production 5 (Statement -> FuncCallStmt)\n");
                }
            }
            if (ntIndex == NT_FACTOR && tIndex == T_ID && useLookahead) {
                if (nextToken.type == TOKEN_SEPARATOR && nextToken.data.separator.symbol == '(') {
                    production = 50; // Factor -> FuncCall
                } else {
                    production = 47; // Factor -> id
                }
            }

            if (production == -1) {
                production = parsingTable[ntIndex][tIndex];
            }

            if (production == -1) {
                printf("Syntax Error at line %d: No production for %s on token %s. Expected one of: ",
                       currentToken.lineNumber, topSymbol, getTokenString(currentToken));
                for (int i = 0; i < num_terminal; i++) {
                    if (parsingTable[ntIndex][i] != -1) {
                        printf("%s ", terminals[i]);
                    }
                }
                printf("\n");
                free(topSymbol);
                return 0;
            }

            char (*rhs)[maxSymbolLength] = getFlatIndex(grammar, production);
            if (!rhs) {
                printf("Syntax Error: Invalid production index %d for non-terminal %s\n", production, topSymbol);
                free(topSymbol);
                return 0;
            }

            int found = 0;
            for (int i = 0; i < productionCount; i++) {
                for (int j = 0; j < grammar[i].productionCount; j++) {
                    if (grammar[i].productionIndex[j] == production) {
                        if (strcmp(topSymbol, grammar[i].lhs) != 0) {
                            printf("Syntax Error: Production %d does not match non-terminal %s\n", production, topSymbol);
                            free(topSymbol);
                            return 0;
                        }
                        found = 1;
                        break;
                    }
                }
                if (found) break;
            }
            if (!found) {
                printf("Syntax Error: Production %d not found for non-terminal %s\n", production, topSymbol);
                free(topSymbol);
                return 0;
            }

            printf("Using production %d for %s -> ", production, topSymbol);
            for (int k = 0; rhs[k][0] != '\0'; k++) printf("%s ", rhs[k]);
            printf("\n");

            if (strcmp(rhs[0], "epsilon") == 0) {
                free(topSymbol);
                continue;
            }

            // Create child nodes and push in reverse order
            int symbolCount = 0;
            while (rhs[symbolCount][0] != '\0') symbolCount++;
            // Store children temporarily to ensure correct order in parse tree
            node* tempChildren[10];
            int tempChildCount = 0;
            for (int i = 0; i < symbolCount; i++) {
                node* child = create(rhs[i]);
                tempChildren[tempChildCount++] = child;
            }
            // Assign children to currentNode in correct order
            for (int i = 0; i < tempChildCount; i++) {
                currentNode->children[i] = tempChildren[i];
                currentNode->childCount++;
            }
            // Push symbols onto stack in reverse order
            for (int i = symbolCount - 1; i >= 0; i--) {
                push(&s, rhs[i], tempChildren[i]);
            }
            free(topSymbol);
        } else {
            printf("Syntax Error at line %d: Expected terminal %s, got %s\n",
                   currentToken.lineNumber, topSymbol, getTokenString(currentToken));
            free(topSymbol);
            return 0;
        }
    }

    if (currentTokenPtr != NULL) {
        printf("Syntax Error at line %d: Unexpected tokens remaining\n",
               currentToken.lineNumber);
        freeTokenList(currentTokenPtr);
        return 0;
    }
    return 1;
}

char (*getFlatIndex(productionRule* grammar, int prodNum))[maxSymbolLength] {
    for (int i = 0; i < productionCount; i++) {
        for (int j = 0; j < grammar[i].productionCount; j++) {
            if (grammar[i].productionIndex[j] == prodNum) {
                return grammar[i].rhs[j];
            }
        }
    }
    return NULL;
}

const char* getTokenString(Token t) {
    static char buffer[64];
    if (t.lineNumber == -1) return "EOF";
    switch (t.type) {
        case TOKEN_KEYWORD:
            strcpy(buffer, t.data.keyword.lexeme);
            return buffer;
        case TOKEN_OPERATOR:
            strcpy(buffer, t.data.operator.op);
            return buffer;
        case TOKEN_IDENTIFIER:
            strcpy(buffer, t.data.identifier.lexeme);
            return buffer;
        case TOKEN_LITERAL:
            strcpy(buffer, t.data.literal.value);
            return buffer;
        case TOKEN_SEPARATOR:
            buffer[0] = t.data.separator.symbol;
            buffer[1] = '\0';
            return buffer;
        default:
            return "UNKNOWN";
    }
}

Terminal getTerminalEnum(const char* sym, Token token) {
    if (token.type == TOKEN_LITERAL) {
        return T_LITERAL;
    }
    if (token.type == TOKEN_IDENTIFIER && strcmp(sym, "id") == 0) {
        return T_ID;
    }
    if (token.type == TOKEN_OPERATOR) {
        if (strcmp(sym, token.data.operator.op) == 0) {
            for (int i = 0; terminals[i] != NULL; i++) {
                if (strcmp(sym, terminals[i]) == 0) {
                    return (Terminal)i;
                }
            }
        }
    }
    if (token.type == TOKEN_SEPARATOR) {
        char temp[2] = {token.data.separator.symbol, '\0'};
        if (strcmp(sym, temp) == 0) {
            for (int i = 0; terminals[i] != NULL; i++) {
                if (strcmp(sym, terminals[i]) == 0) {
                    return (Terminal)i;
                }
            }
        }
    }
    for (int i = 0; terminals[i] != NULL; i++) {
        if (strcmp(sym, terminals[i]) == 0) {
            return (Terminal)i;
        }
    }
    return -1;
}

NonTerminal getNonTerminalEnum(const char* sym) {
    for (int i = 0; nonterminals[i] != NULL; i++) {
        if (strcmp(sym, nonterminals[i]) == 0) {
            return (NonTerminal)i;
        }
    }
    return -1;
}

void printParseTree(node* root, int depth) {
    if (!root) return;
    for (int i = 0; i < depth; i++) printf("  ");
    printf("%s", root->symbol);
    if (root->isTokenPresent) {
        printf(" [token: %s]", getTokenString(root->token));
    }
    printf("\n");
    for (int i = 0; i < root->childCount; i++) {
        printParseTree(root->children[i], depth + 1);
    }
}

int main() {
    tokenNode* head = NULL;
    FILE* fptr = fopen("exit1.txt", "r");
    if (!fptr) {
        printf("Failed to open input file.\n");
        return 1;
    }

    char content[100];
    char fullContent[4096] = "";
    int lineNumber = 1;

    ParsingTableInitialized();

    while (fgets(content, sizeof(content), fptr)) {
        strcat(fullContent, content);
        if (strchr(content, '\n')) lineNumber++;
    }
    fclose(fptr);

    head = check(fullContent, 1);
    tokenNode* headCopy = head;

    printf("\n----- Full Token List -----\n");
    tokenNode* temp = head;
    int tokenCount = 0;
    while (temp != NULL) {
        Token t = temp->token;
        printf("Token %d: %s (type %d, line %d)\n", tokenCount++, getTokenString(t), t.type, t.lineNumber);
        temp = temp->next;
    }

    Token endToken;
    endToken.type = TOKEN_SEPARATOR;
    endToken.data.separator.symbol = '$';
    endToken.lineNumber = lineNumber - 1;
    head = append(endToken, head);

    printf("\n----- Token List -----\n");
    temp = head;
    while (temp != NULL) {
        Token t = temp->token;
        switch (t.type) {
            case TOKEN_KEYWORD:
                printf("Keyword: %s (line %d)\n", t.data.keyword.lexeme, t.lineNumber);
                break;
            case TOKEN_LITERAL:
                printf("Literal: %s (line %d)\n", t.data.literal.value, t.lineNumber);
                break;
            case TOKEN_OPERATOR:
                printf("Operator: %s (line %d)\n", t.data.operator.op, t.lineNumber);
                break;
            case TOKEN_SEPARATOR:
                if (t.data.separator.symbol == '$')
                    printf("End Marker: $ (line %d)\n", t.lineNumber);
                else
                    printf("Separator: %c (line %d)\n", t.data.separator.symbol, t.lineNumber);
                break;
            case TOKEN_IDENTIFIER:
                printf("Identifier: %s (line %d)\n", t.data.identifier.lexeme, t.lineNumber);
                break;
        }
        temp = temp->next;
    }

    node* root = NULL;
    int result = parsingTableChecking(head, &root);

    if (result) {
        printf("\nParsing Successful!\n\n");
        printf("--------------- Parse Tree ---------------\n");
        printParseTree(root, 0);
        printf("------------------------------------------\n");
    } else {
        printf("\nParsing Failed.\n");
    }

    freeTokenList(headCopy);
    freeNode(root);
    return 0;
}
