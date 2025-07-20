#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define sz 100
#define maxProduction 100
#define maxSymbolPerProduction 10
#define maxSymbolLength 16
#define num_nonTerminal 23
#define num_terminal 32
#define MAX_SYMBOLS 100 // Maximum number of symbols in the symbol table

typedef enum {
    T_INT, T_FLOAT, T_ID, T_RETURN, T_IF, T_ELSE, T_ASSIGN, T_PLUS, T_MINUS,
    T_MUL, T_DIV, T_LT, T_LE, T_GT, T_GE, T_EQ, T_NEQ, T_AND, T_OR,
    T_LPAREN, T_RPAREN, T_COMMA, T_SEMICOLON, T_LITERAL, T_DOLLAR,
    T_LBRACE, T_RBRACE, T_EXCLAIM, T_WHILE, T_PRINTF, T_AT, T_TILDE, T_STRING
} Terminal;

typedef enum {
    NT_PROGRAM, NT_STATEMENTLIST, NT_STATEMENT, NT_STATEMENTSUFFIX, NT_DECLTAIL,
    NT_ELSEPART, NT_RETURNSTMT, NT_TYPE, NT_EXPRESSION, NT_EXPRESSIONPRIME,
    NT_RELEXPR, NT_RELOPTAIL, NT_RELOP, NT_ARITHEXPR, NT_ARITHEXPRPRIME,
    NT_TERM, NT_TERMPRIME, NT_FACTOR, NT_UNARYOP, NT_BLOCK, NT_PRINTFSTMT,
    NT_PRINTTAIL, NT_COUNT
} NonTerminal;

typedef enum { TYPE_INT, TYPE_FLOAT } VarType;

typedef struct {
    char* name;
    VarType type;
    int lineNumber;
    int hasValue; // Flag to indicate if a value is set
    union {
        int intValue;
        float floatValue;
    } value; // Union to store int or float values
} SymbolEntry;

typedef struct {
    SymbolEntry entries[MAX_SYMBOLS];
    int count;
} SymbolTable;

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
    TOKEN_KEYWORD, TOKEN_LITERAL, TOKEN_OPERATOR, TOKEN_SEPARATOR, TOKEN_IDENTIFIER,
    TOKEN_STRING
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
    char value[128];
} TokenString;

typedef struct {
    TokenType type;
    int lineNumber;
    union {
        TokenKeyword keyword;
        TokenIdentifier identifier;
        TokenLiteral literal;
        TokenOperator operator;
        TokenSeparator separator;
        TokenString string;
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
    struct tree* parent; // Added parent pointer
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
    "ElsePart", "ReturnStmt", "Type", "Expression", "ExpressionPrime",
    "RelExpr", "RelOpTail", "RelOp", "ArithExpr", "ArithExprPrime",
    "Term", "TermPrime", "Factor", "UnaryOp", "Block", "PrintfStmt",
    "PrintTail", NULL
};

const char* terminals[] = {
    "int", "float", "id", "return", "if", "else", "=", "+", "-", "*", "/",
    "<", "<=", ">", ">=", "==", "!=", "&&", "||", "(", ")", ",", ";",
    "literal", "$", "{", "}", "!", "while", "printf", "@", "~", "string", NULL
};

int parsingTable[num_nonTerminal][num_terminal] = {0};

void initStack(stack* s);
Token peekNextToken(tokenNode** current);
Terminal getTerminalEnum(const char* sym, Token token);
NonTerminal getNonTerminalEnum(const char* sym);
const char* getTokenString(Token t);
char* cleanTokenString(const char* str);
char (*getFlatIndex(productionRule* grammar, int prodNum))[maxSymbolLength];
int parsingTableChecking(tokenNode* head, node** p, SymbolTable* table);
Token getNextToken(tokenNode** current);
void ungetToken(Token token);
int isNonTerminal(const char* symbol);
void ParsingTableInitialized(void);
void initSymbolTable(SymbolTable* table);
int addSymbol(SymbolTable* table, const char* name, VarType type, int lineNumber);
int findSymbol(SymbolTable* table, const char* name);

// Symbol Table Functions
void initSymbolTable(SymbolTable* table) {
    table->count = 0;
}

int addSymbol(SymbolTable* table, const char* name, VarType type, int lineNumber) {
    if (findSymbol(table, name) != -1) {
        printf("Semantic Error at line %d: Variable '%s' already declared\n", lineNumber, name);
        return 0;
    }
    if (table->count >= MAX_SYMBOLS) {
        printf("Error: Symbol table is full\n");
        return 0;
    }
    SymbolEntry* entry = &table->entries[table->count];
    entry->name = strdup(name);
    entry->type = type;
    entry->lineNumber = lineNumber;
    entry->hasValue = 0; // Initialize as uninitialized
    table->count++;
    printf("Debug: Added symbol '%s' (type: %s, line: %d) to symbol table\n",
           name, entry->type == TYPE_INT ? "int" : "float", lineNumber);
    return 1;
}

int findSymbol(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void printSymbolTable(SymbolTable* table) {
    printf("\n----- Symbol Table -----\n");
    for (int i = 0; i < table->count; i++) {
        SymbolEntry* entry = &table->entries[i];
        printf("Symbol %d: %s (type: %s, line: %d", i, entry->name,
               entry->type == TYPE_INT ? "int" : "float", entry->lineNumber);
        if (entry->hasValue) {
            if (entry->type == TYPE_INT) {
                printf(", value: %d, hasValue: %d", entry->value.intValue, entry->hasValue);
            } else {
                printf(", value: %f, hasValue: %d", entry->value.floatValue, entry->hasValue);
            }
        } else {
            printf(", value: uninitialized");
        }
        printf(")\n");
    }
    printf("-----------------------\n");
}

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

void initStack(stack* s) {
    s->top = -1;
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

    printf("Debug: Starting lexer with content:\n%s\n", content);
    while (content[i] != '\0') {
        printf("Debug: Processing char '%c' at position %d, line %d\n", content[i], i, lineNumber);
        if (isspace(content[i])) {
            if (content[i] == '\n') lineNumber++;
            i++;
            continue;
        }

        if (content[i] == '/' && content[i + 1] == '/') {
            printf("Debug: Found comment start at position %d, line %d\n", i, lineNumber);
            while (content[i] != '\n' && content[i] != '\0') i++;
            if (content[i] == '\n') lineNumber++;
            i++;
            printf("Debug: Skipped comment, now at position %d, line %d\n", i, lineNumber);
            continue;
        }

        Token t;
        t.lineNumber = lineNumber;
        printf("Debug: Creating token at position %d, line %d\n", i, lineNumber);

        if (isalpha(content[i]) || content[i] == '_') {
            int start = i;
            while (isalnum(content[i]) || content[i] == '_') i++;
            int len = i - start;

            char buffer[64];
            strncpy(buffer, &content[start], len);
            buffer[len] = '\0';
            printf("Debug: Extracted lexeme '%s' (length %d)\n", buffer, len);

            if (strcmp(buffer, "int") == 0 || strcmp(buffer, "float") == 0 ||
                strcmp(buffer, "return") == 0 || strcmp(buffer, "if") == 0 ||
                strcmp(buffer, "else") == 0 || strcmp(buffer, "while") == 0 ||
                strcmp(buffer, "printf") == 0) {
                t.type = TOKEN_KEYWORD;
                strcpy(t.data.keyword.lexeme, buffer);
                printf("Debug: Token is KEYWORD: %s\n", buffer);
            } else {
                t.type = TOKEN_IDENTIFIER;
                strcpy(t.data.identifier.lexeme, buffer);
                printf("Debug: Token is IDENTIFIER: %s\n", buffer);
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
            printf("Debug: Token is LITERAL: %s\n", t.data.literal.value);
            head = append(t, head);
        } else if (content[i] == '"') {
            i++;
            int start = i;
            while (content[i] != '"' && content[i] != '\0') i++;
            if (content[i] == '\0') {
                printf("Syntax Error at line %d: Unclosed string\n", lineNumber);
                freeTokenList(head);
                return NULL;
            }
            int len = i - start;
            strncpy(t.data.string.value, &content[start], len);
            t.data.string.value[len] = '\0';
            t.type = TOKEN_STRING;
            i++;
            printf("Debug: Token is STRING: %s\n", t.data.string.value);
            head = append(t, head);
        } else if ((content[i] == '>' || content[i] == '<' ||
                    content[i] == '=' || content[i] == '!') &&
                   content[i + 1] == '=') {
            t.type = TOKEN_OPERATOR;
            t.data.operator.op[0] = content[i];
            t.data.operator.op[1] = '=';
            t.data.operator.op[2] = '\0';
            i += 2;
            printf("Debug: Token is OPERATOR: %s\n", t.data.operator.op);
            head = append(t, head);
        } else if ((content[i] == '&' && content[i + 1] == '&') ||
                   (content[i] == '|' && content[i + 1] == '|')) {
            t.type = TOKEN_OPERATOR;
            t.data.operator.op[0] = content[i];
            t.data.operator.op[1] = content[i + 1];
            t.data.operator.op[2] = '\0';
            i += 2;
            printf("Debug: Token is OPERATOR: %s\n", t.data.operator.op);
            head = append(t, head);
        } else if (strchr("+-*/<>=!", content[i])) {
            t.type = TOKEN_OPERATOR;
            t.data.operator.op[0] = content[i];
            t.data.operator.op[1] = '\0';
            i++;
            printf("Debug: Token is OPERATOR: %c\n", t.data.operator.op[0]);
            head = append(t, head);
        } else if (strchr("();{},", content[i])) {
            t.type = TOKEN_SEPARATOR;
            t.data.separator.symbol = content[i];
            i++;
            printf("Debug: Token is SEPARATOR: %c\n", t.data.separator.symbol);
            head = append(t, head);
        } else if (content[i] == '@' || content[i] == '~') {
            t.type = TOKEN_SEPARATOR;
            t.data.separator.symbol = content[i];
            i++;
            printf("Debug: Token is SEPARATOR: %c\n", t.data.separator.symbol);
            head = append(t, head);
        } else {
            printf("Debug: Skipping unrecognized character '%c' at position %d\n", content[i], i);
            i++;
        }
    }
    printf("Debug: Lexer finished, returning token list\n");
    return head;
}

void ParsingTableInitialized(void) {
    flatIndexCounter = 1;
    productionCount = 0;

    for (int i = 0; i < num_nonTerminal; i++) {
        for (int j = 0; j < num_terminal; j++) {
            parsingTable[i][j] = -1;
        }
    }

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

    // 4-10: Statement -> Type id StatementSuffix | ReturnStmt | if ( Expression ) Statement ElsePart | id = Expression ; | while ( Expression ) Block | Block | PrintfStmt
    strcpy(grammar[productionCount].lhs, "Statement");
    grammar[productionCount].productionCount = 7;
    strcpy(grammar[productionCount].rhs[0][0], "Type");
    strcpy(grammar[productionCount].rhs[0][1], "id");
    strcpy(grammar[productionCount].rhs[0][2], "StatementSuffix");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "ReturnStmt");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[2][0], "if");
    strcpy(grammar[productionCount].rhs[2][1], "(");
    strcpy(grammar[productionCount].rhs[2][2], "Expression");
    strcpy(grammar[productionCount].rhs[2][3], ")");
    strcpy(grammar[productionCount].rhs[2][4], "Statement");
    strcpy(grammar[productionCount].rhs[2][5], "ElsePart");
    grammar[productionCount].rhs[2][6][0] = '\0';
    grammar[productionCount].productionIndex[2] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[3][0], "id");
    strcpy(grammar[productionCount].rhs[3][1], "=");
    strcpy(grammar[productionCount].rhs[3][2], "Expression");
    strcpy(grammar[productionCount].rhs[3][3], ";");
    grammar[productionCount].rhs[3][4][0] = '\0';
    grammar[productionCount].productionIndex[3] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[4][0], "while");
    strcpy(grammar[productionCount].rhs[4][1], "(");
    strcpy(grammar[productionCount].rhs[4][2], "Expression");
    strcpy(grammar[productionCount].rhs[4][3], ")");
    strcpy(grammar[productionCount].rhs[4][4], "Block");
    grammar[productionCount].rhs[4][5][0] = '\0';
    grammar[productionCount].productionIndex[4] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[5][0], "Block");
    grammar[productionCount].rhs[5][1][0] = '\0';
    grammar[productionCount].productionIndex[5] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[6][0], "PrintfStmt");
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

    // 15-16: ElsePart -> else Statement | epsilon
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

    // 17: ReturnStmt -> return Expression ;
    strcpy(grammar[productionCount].lhs, "ReturnStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "return");
    strcpy(grammar[productionCount].rhs[0][1], "Expression");
    strcpy(grammar[productionCount].rhs[0][2], ";");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 18-19: Type -> int | float
    strcpy(grammar[productionCount].lhs, "Type");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "int");
    grammar[productionCount].rhs[0][1][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "float");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // 20: Expression -> RelExpr ExpressionPrime
    strcpy(grammar[productionCount].lhs, "Expression");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "RelExpr");
    strcpy(grammar[productionCount].rhs[0][1], "ExpressionPrime");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 21-24: ExpressionPrime -> && RelExpr ExpressionPrime | || RelExpr ExpressionPrime | epsilon | RelOp RelExpr ExpressionPrime
    strcpy(grammar[productionCount].lhs, "ExpressionPrime");
    grammar[productionCount].productionCount = 4;
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
    strcpy(grammar[productionCount].rhs[3][0], "RelOp");
    strcpy(grammar[productionCount].rhs[3][1], "RelExpr");
    strcpy(grammar[productionCount].rhs[3][2], "ExpressionPrime");
    grammar[productionCount].rhs[3][3][0] = '\0';
    grammar[productionCount].productionIndex[3] = flatIndexCounter++;
    productionCount++;

    // 25: RelExpr -> ArithExpr RelOpTail
    strcpy(grammar[productionCount].lhs, "RelExpr");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "ArithExpr");
    strcpy(grammar[productionCount].rhs[0][1], "RelOpTail");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 26-27: RelOpTail -> RelOp ArithExpr RelOpTail | epsilon
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

    // 28-33: RelOp -> < | > | <= | >= | == | !=
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

    // 34: ArithExpr -> Term ArithExprPrime
    strcpy(grammar[productionCount].lhs, "ArithExpr");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "Term");
    strcpy(grammar[productionCount].rhs[0][1], "ArithExprPrime");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 35-37: ArithExprPrime -> + Term ArithExprPrime | - Term ArithExprPrime | epsilon
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

    // 38: Term -> Factor TermPrime
    strcpy(grammar[productionCount].lhs, "Term");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "Factor");
    strcpy(grammar[productionCount].rhs[0][1], "TermPrime");
    grammar[productionCount].rhs[0][2][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 39-41: TermPrime -> * Factor TermPrime | / Factor TermPrime | epsilon
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

    // 42-45: Factor -> ( Expression ) | id | literal | UnaryOp Factor
    strcpy(grammar[productionCount].lhs, "Factor");
    grammar[productionCount].productionCount = 4;
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
    productionCount++;

    // 46-48: UnaryOp -> - | ! | +
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

    // 49: Block -> { StatementList }
    strcpy(grammar[productionCount].lhs, "Block");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "{");
    strcpy(grammar[productionCount].rhs[0][1], "StatementList");
    strcpy(grammar[productionCount].rhs[0][2], "}");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 50: PrintfStmt -> printf @ string PrintTail @ ;
    strcpy(grammar[productionCount].lhs, "PrintfStmt");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "printf");
    strcpy(grammar[productionCount].rhs[0][1], "@");
    strcpy(grammar[productionCount].rhs[0][2], "string");
    strcpy(grammar[productionCount].rhs[0][3], "PrintTail");
    strcpy(grammar[productionCount].rhs[0][4], "@");
    strcpy(grammar[productionCount].rhs[0][5], ";");
    grammar[productionCount].rhs[0][6][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    productionCount++;

    // 51-52: PrintTail -> ~ id PrintTail | epsilon
    strcpy(grammar[productionCount].lhs, "PrintTail");
    grammar[productionCount].productionCount = 2;
    strcpy(grammar[productionCount].rhs[0][0], "~");
    strcpy(grammar[productionCount].rhs[0][1], "id");
    strcpy(grammar[productionCount].rhs[0][2], "PrintTail");
    grammar[productionCount].rhs[0][3][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
    strcpy(grammar[productionCount].rhs[1][0], "epsilon");
    grammar[productionCount].rhs[1][1][0] = '\0';
    grammar[productionCount].productionIndex[1] = flatIndexCounter++;
    productionCount++;

    // Update parsing table
    parsingTable[NT_PROGRAM][T_INT] = 1;
    parsingTable[NT_PROGRAM][T_FLOAT] = 1;
    parsingTable[NT_PROGRAM][T_ID] = 1;
    parsingTable[NT_PROGRAM][T_RETURN] = 1;
    parsingTable[NT_PROGRAM][T_IF] = 1;
    parsingTable[NT_PROGRAM][T_LBRACE] = 1;
    parsingTable[NT_PROGRAM][T_WHILE] = 1;
    parsingTable[NT_PROGRAM][T_PRINTF] = 1;

    parsingTable[NT_STATEMENTLIST][T_INT] = 2;
    parsingTable[NT_STATEMENTLIST][T_FLOAT] = 2;
    parsingTable[NT_STATEMENTLIST][T_ID] = 2;
    parsingTable[NT_STATEMENTLIST][T_RETURN] = 2;
    parsingTable[NT_STATEMENTLIST][T_IF] = 2;
    parsingTable[NT_STATEMENTLIST][T_LBRACE] = 2;
    parsingTable[NT_STATEMENTLIST][T_WHILE] = 2;
    parsingTable[NT_STATEMENTLIST][T_PRINTF] = 2;
    parsingTable[NT_STATEMENTLIST][T_RBRACE] = 3;
    parsingTable[NT_STATEMENTLIST][T_DOLLAR] = 3;

    parsingTable[NT_STATEMENT][T_INT] = 4;
    parsingTable[NT_STATEMENT][T_FLOAT] = 4;
    parsingTable[NT_STATEMENT][T_ID] = 7;
    parsingTable[NT_STATEMENT][T_RETURN] = 5;
    parsingTable[NT_STATEMENT][T_IF] = 6;
    parsingTable[NT_STATEMENT][T_WHILE] = 8;
    parsingTable[NT_STATEMENT][T_LBRACE] = 9;
    parsingTable[NT_STATEMENT][T_PRINTF] = 10;

    parsingTable[NT_STATEMENTSUFFIX][T_ASSIGN] = 11;
    parsingTable[NT_STATEMENTSUFFIX][T_COMMA] = 12;
    parsingTable[NT_STATEMENTSUFFIX][T_SEMICOLON] = 12;

    parsingTable[NT_DECLTAIL][T_COMMA] = 13;
    parsingTable[NT_DECLTAIL][T_SEMICOLON] = 14;

    parsingTable[NT_ELSEPART][T_ELSE] = 15;
    parsingTable[NT_ELSEPART][T_SEMICOLON] = 16;
    parsingTable[NT_ELSEPART][T_DOLLAR] = 16;
    parsingTable[NT_ELSEPART][T_LBRACE] = 16;
    parsingTable[NT_ELSEPART][T_RBRACE] = 16;
    parsingTable[NT_ELSEPART][T_WHILE] = 16;
    parsingTable[NT_ELSEPART][T_IF] = 16;
    parsingTable[NT_ELSEPART][T_INT] = 16;
    parsingTable[NT_ELSEPART][T_FLOAT] = 16;
    parsingTable[NT_ELSEPART][T_ID] = 16;
    parsingTable[NT_ELSEPART][T_RETURN] = 16;
    parsingTable[NT_ELSEPART][T_PRINTF] = 16;

    parsingTable[NT_RETURNSTMT][T_RETURN] = 17;

    parsingTable[NT_TYPE][T_INT] = 18;
    parsingTable[NT_TYPE][T_FLOAT] = 19;

    parsingTable[NT_EXPRESSION][T_INT] = 20;
    parsingTable[NT_EXPRESSION][T_FLOAT] = 20;
    parsingTable[NT_EXPRESSION][T_ID] = 20;
    parsingTable[NT_EXPRESSION][T_PLUS] = 20;
    parsingTable[NT_EXPRESSION][T_MINUS] = 20;
    parsingTable[NT_EXPRESSION][T_EXCLAIM] = 20;
    parsingTable[NT_EXPRESSION][T_LPAREN] = 20;
    parsingTable[NT_EXPRESSION][T_LITERAL] = 20;

    parsingTable[NT_EXPRESSIONPRIME][T_AND] = 21;
    parsingTable[NT_EXPRESSIONPRIME][T_OR] = 22;
    parsingTable[NT_EXPRESSIONPRIME][T_RPAREN] = 23;
    parsingTable[NT_EXPRESSIONPRIME][T_COMMA] = 23;
    parsingTable[NT_EXPRESSIONPRIME][T_SEMICOLON] = 23;
    parsingTable[NT_EXPRESSIONPRIME][T_ELSE] = 23;
    parsingTable[NT_EXPRESSIONPRIME][T_LT] = 24;
    parsingTable[NT_EXPRESSIONPRIME][T_LE] = 24;
    parsingTable[NT_EXPRESSIONPRIME][T_GT] = 24;
    parsingTable[NT_EXPRESSIONPRIME][T_GE] = 24;
    parsingTable[NT_EXPRESSIONPRIME][T_EQ] = 24;
    parsingTable[NT_EXPRESSIONPRIME][T_NEQ] = 24;

    parsingTable[NT_RELEXPR][T_INT] = 25;
    parsingTable[NT_RELEXPR][T_FLOAT] = 25;
    parsingTable[NT_RELEXPR][T_ID] = 25;
    parsingTable[NT_RELEXPR][T_PLUS] = 25;
    parsingTable[NT_RELEXPR][T_MINUS] = 25;
    parsingTable[NT_RELEXPR][T_EXCLAIM] = 25;
    parsingTable[NT_RELEXPR][T_LPAREN] = 25;
    parsingTable[NT_RELEXPR][T_LITERAL] = 25;

    parsingTable[NT_RELOPTAIL][T_LT] = 26;
    parsingTable[NT_RELOPTAIL][T_LE] = 26;
    parsingTable[NT_RELOPTAIL][T_GT] = 26;
    parsingTable[NT_RELOPTAIL][T_GE] = 26;
    parsingTable[NT_RELOPTAIL][T_EQ] = 26;
    parsingTable[NT_RELOPTAIL][T_NEQ] = 26;
    parsingTable[NT_RELOPTAIL][T_RPAREN] = 27;
    parsingTable[NT_RELOPTAIL][T_COMMA] = 27;
    parsingTable[NT_RELOPTAIL][T_SEMICOLON] = 27;
    parsingTable[NT_RELOPTAIL][T_ELSE] = 27;
    parsingTable[NT_RELOPTAIL][T_AND] = 27;
    parsingTable[NT_RELOPTAIL][T_OR] = 27;

    parsingTable[NT_RELOP][T_LT] = 28;
    parsingTable[NT_RELOP][T_GT] = 29;
    parsingTable[NT_RELOP][T_LE] = 30;
    parsingTable[NT_RELOP][T_GE] = 31;
    parsingTable[NT_RELOP][T_EQ] = 32;
    parsingTable[NT_RELOP][T_NEQ] = 33;

    parsingTable[NT_ARITHEXPR][T_INT] = 34;
    parsingTable[NT_ARITHEXPR][T_FLOAT] = 34;
    parsingTable[NT_ARITHEXPR][T_ID] = 34;
    parsingTable[NT_ARITHEXPR][T_PLUS] = 34;
    parsingTable[NT_ARITHEXPR][T_MINUS] = 34;
    parsingTable[NT_ARITHEXPR][T_EXCLAIM] = 34;
    parsingTable[NT_ARITHEXPR][T_LPAREN] = 34;
    parsingTable[NT_ARITHEXPR][T_LITERAL] = 34;

    parsingTable[NT_ARITHEXPRPRIME][T_PLUS] = 35;
    parsingTable[NT_ARITHEXPRPRIME][T_MINUS] = 36;
    parsingTable[NT_ARITHEXPRPRIME][T_RPAREN] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_COMMA] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_SEMICOLON] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_LT] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_LE] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_GT] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_GE] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_EQ] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_NEQ] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_AND] = 37;
    parsingTable[NT_ARITHEXPRPRIME][T_OR] = 37;

    parsingTable[NT_TERM][T_INT] = 38;
    parsingTable[NT_TERM][T_FLOAT] = 38;
    parsingTable[NT_TERM][T_ID] = 38;
    parsingTable[NT_TERM][T_PLUS] = 38;
    parsingTable[NT_TERM][T_MINUS] = 38;
    parsingTable[NT_TERM][T_EXCLAIM] = 38;
    parsingTable[NT_TERM][T_LPAREN] = 38;
    parsingTable[NT_TERM][T_LITERAL] = 38;

    parsingTable[NT_TERMPRIME][T_MUL] = 39;
    parsingTable[NT_TERMPRIME][T_DIV] = 40;
    parsingTable[NT_TERMPRIME][T_PLUS] = 41;
    parsingTable[NT_TERMPRIME][T_MINUS] = 41;
    parsingTable[NT_TERMPRIME][T_LT] = 41;
    parsingTable[NT_TERMPRIME][T_LE] = 41;
    parsingTable[NT_TERMPRIME][T_GT] = 41;
    parsingTable[NT_TERMPRIME][T_GE] = 41;
    parsingTable[NT_TERMPRIME][T_EQ] = 41;
    parsingTable[NT_TERMPRIME][T_NEQ] = 41;
    parsingTable[NT_TERMPRIME][T_AND] = 41;
    parsingTable[NT_TERMPRIME][T_OR] = 41;
    parsingTable[NT_TERMPRIME][T_RPAREN] = 41;
    parsingTable[NT_TERMPRIME][T_COMMA] = 41;
    parsingTable[NT_TERMPRIME][T_SEMICOLON] = 41;

    parsingTable[NT_FACTOR][T_LPAREN] = 42;
    parsingTable[NT_FACTOR][T_ID] = 43;
    parsingTable[NT_FACTOR][T_LITERAL] = 44;
    parsingTable[NT_FACTOR][T_MINUS] = 45;
    parsingTable[NT_FACTOR][T_PLUS] = 45;
    parsingTable[NT_FACTOR][T_EXCLAIM] = 45;

    parsingTable[NT_UNARYOP][T_MINUS] = 46;
    parsingTable[NT_UNARYOP][T_EXCLAIM] = 47;
    parsingTable[NT_UNARYOP][T_PLUS] = 48;

    parsingTable[NT_BLOCK][T_LBRACE] = 49;

    parsingTable[NT_PRINTFSTMT][T_PRINTF] = 50;
    parsingTable[NT_PRINTTAIL][T_TILDE] = 51;
    parsingTable[NT_PRINTTAIL][T_AT] = 52;

    printf("Debug: Parsing table initialized, productionCount=%d, flatIndexCounter=%d\n", productionCount, flatIndexCounter);
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
    if (strcmp(terminal, "id") == 0) {
        return token.type == TOKEN_IDENTIFIER;
    }
    if (strcmp(terminal, "string") == 0) {
        return token.type == TOKEN_STRING;
    }
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
    if (strcmp(terminal, "printf") == 0)
        return token.type == TOKEN_KEYWORD && strcmp(cleanTokenString(token.data.keyword.lexeme), "printf") == 0;
    if (strcmp(terminal, "literal") == 0)
        return token.type == TOKEN_LITERAL;
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
    if (strcmp(terminal, "!") == 0)
        return token.type == TOKEN_OPERATOR && strcmp(cleanTokenString(token.data.operator.op), "!") == 0;
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
    if (strcmp(terminal, "@") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == '@';
    if (strcmp(terminal, "~") == 0)
        return token.type == TOKEN_SEPARATOR && token.data.separator.symbol == '~';
    return 0;
}

void printStack(stack* s) {
    printf("Debug: Stack contents: ");
    if (isEmpty(s)) {
        printf("(empty)");
    } else {
        for (int i = 0; i <= s->top; i++) {
            printf("%s ", s->items[i].symbol);
        }
    }
    printf("\n");
}

// Add a structure to hold expression values
typedef struct {
    int isValid;
    VarType type;
    union {
        int intValue;
        float floatValue;
    } value;
} ExprValue;

// Function to evaluate an expression node
ExprValue evaluateExpression(node* exprNode, SymbolTable* table) {
    ExprValue result = {0, TYPE_INT, {0}};
    if (!exprNode) {
        printf("Debug: evaluateExpression: NULL node\n");
        return result;
    }
    printf("Debug: Evaluating expression node: %s (children: %d)\n", exprNode->symbol, exprNode->childCount);

    // Handle leaf nodes (Factor -> id | literal)
    if (strcmp(exprNode->symbol, "Factor") == 0 && exprNode->childCount == 3 &&
        strcmp(exprNode->children[0]->symbol, "(") == 0) {
        printf("Debug: Factor with parentheses, evaluating Expression\n");
        return evaluateExpression(exprNode->children[1], table); // Expression inside ()
    }
    if (strcmp(exprNode->symbol, "Factor") == 0 && exprNode->childCount == 1) {
        node* child = exprNode->children[0];
        printf("Debug: Factor with single child: %s\n", child->symbol);
        if (strcmp(child->symbol, "id") == 0 && child->isTokenPresent) {
            int index = findSymbol(table, child->token.data.identifier.lexeme);
            printf("Debug: ID %s, found at index %d\n", child->token.data.identifier.lexeme, index);
            if (index != -1) {
                SymbolEntry* entry = &table->entries[index];
                result.isValid = entry->hasValue;
                result.type = entry->type;
                if (entry->type == TYPE_INT) {
                    result.value.intValue = entry->value.intValue;
                } else {
                    result.value.floatValue = entry->value.floatValue;
                }
                printf("Debug: ID %s value: %s, hasValue: %d\n",
                       child->token.data.identifier.lexeme,
                       entry->type == TYPE_INT ? "%d" : "%f",
                       entry->type == TYPE_INT ? entry->value.intValue : entry->value.floatValue,
                       entry->hasValue);
            } else {
                printf("Debug: ID %s not found in symbol table\n", child->token.data.identifier.lexeme);
            }
        } else if (strcmp(child->symbol, "literal") == 0 && child->isTokenPresent) {
            result.isValid = 1;
            result.type = strchr(child->token.data.literal.value, '.') ? TYPE_FLOAT : TYPE_INT;
            if (result.type == TYPE_INT) {
                result.value.intValue = atoi(child->token.data.literal.value);
                printf("Debug: Literal %s evaluated to int: %d\n", child->token.data.literal.value, result.value.intValue);
            } else {
                result.value.floatValue = atof(child->token.data.literal.value);
                printf("Debug: Literal %s evaluated to float: %f\n", child->token.data.literal.value, result.value.floatValue);
            }
        } else {
            printf("Debug: Invalid Factor child: %s, isTokenPresent: %d\n", child->symbol, child->isTokenPresent);
        }
        return result;
    }

    // Handle ArithExpr (ArithExpr -> Term ArithExprPrime)
    if (strcmp(exprNode->symbol, "ArithExpr") == 0 && exprNode->childCount >= 1) {
        printf("Debug: Evaluating ArithExpr\n");
        ExprValue left = evaluateExpression(exprNode->children[0], table); // Term
        if (!left.isValid) {
            printf("Debug: ArithExpr: Invalid left operand\n");
            return result;
        }
        if (exprNode->childCount == 2) {
            node* prime = exprNode->children[1]; // ArithExprPrime
            if (prime->childCount == 0) {
                printf("Debug: ArithExprPrime is epsilon\n");
                return left; // epsilon
            }
            ExprValue current = left;
            while (prime->childCount >= 2) {
                node* opNode = prime->children[0];
                ExprValue right = evaluateExpression(prime->children[1], table); // Term
                if (!right.isValid) {
                    printf("Debug: ArithExprPrime: Invalid right operand\n");
                    return result;
                }
                result.isValid = 1;
                result.type = (current.type == TYPE_FLOAT || right.type == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                printf("Debug: ArithExpr operation: %s, type: %s\n", opNode->symbol, result.type == TYPE_INT ? "int" : "float");
                if (result.type == TYPE_INT) {
                    if (strcmp(opNode->symbol, "+") == 0) {
                        current.value.intValue += right.value.intValue;
                    } else if (strcmp(opNode->symbol, "-") == 0) {
                        current.value.intValue -= right.value.intValue;
                    }
                    printf("Debug: ArithExpr result: %d\n", current.value.intValue);
                } else {
                    float currentVal = (current.type == TYPE_INT) ? current.value.intValue : current.value.floatValue;
                    float rightVal = (right.type == TYPE_INT) ? right.value.intValue : right.value.floatValue;
                    if (strcmp(opNode->symbol, "+") == 0) {
                        current.value.floatValue = currentVal + rightVal;
                    } else if (strcmp(opNode->symbol, "-") == 0) {
                        current.value.floatValue = currentVal - rightVal;
                    }
                    printf("Debug: ArithExpr result: %f\n", current.value.floatValue);
                }
                prime = prime->children[2]; // Move to next ArithExprPrime
                if (prime->childCount == 0) break; // epsilon
            }
            result = current;
        } else {
            result = left;
        }
        return result;
    }

    // Handle Term (Term -> Factor TermPrime)
    if (strcmp(exprNode->symbol, "Term") == 0 && exprNode->childCount >= 1) {
        printf("Debug: Evaluating Term\n");
        ExprValue left = evaluateExpression(exprNode->children[0], table); // Factor
        if (!left.isValid) {
            printf("Debug: Term: Invalid left operand\n");
            return result;
        }
        if (exprNode->childCount == 2) {
            node* prime = exprNode->children[1]; // TermPrime
            if (prime->childCount == 0) {
                printf("Debug: TermPrime is epsilon\n");
                return left; // epsilon
            }
            ExprValue current = left;
            while (prime->childCount >= 2) {
                node* opNode = prime->children[0];
                ExprValue right = evaluateExpression(prime->children[1], table); // Factor
                if (!right.isValid) {
                    printf("Debug: TermPrime: Invalid right operand\n");
                    return result;
                }
                result.isValid = 1;
                result.type = (current.type == TYPE_FLOAT || right.type == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                printf("Debug: Term operation: %s, type: %s\n", opNode->symbol, result.type == TYPE_INT ? "int" : "float");
                if (result.type == TYPE_INT) {
                    if (strcmp(opNode->symbol, "*") == 0) {
                        current.value.intValue *= right.value.intValue;
                    } else if (strcmp(opNode->symbol, "/") == 0 && right.value.intValue != 0) {
                        current.value.intValue /= right.value.intValue;
                    } else {
                        printf("Debug: Term: Invalid operation or division by zero\n");
                        result.isValid = 0;
                        return result;
                    }
                    printf("Debug: Term result: %d\n", current.value.intValue);
                } else {
                    float currentVal = (current.type == TYPE_INT) ? current.value.intValue : current.value.floatValue;
                    float rightVal = (right.type == TYPE_INT) ? right.value.intValue : right.value.floatValue;
                    if (strcmp(opNode->symbol, "*") == 0) {
                        current.value.floatValue = currentVal * rightVal;
                    } else if (strcmp(opNode->symbol, "/") == 0 && rightVal != 0.0) {
                        current.value.floatValue = currentVal / rightVal;
                    } else {
                        printf("Debug: Term: Invalid operation or division by zero\n");
                        result.isValid = 0;
                        return result;
                    }
                    printf("Debug: Term result: %f\n", current.value.floatValue);
                }
                prime = prime->children[2]; // Move to next TermPrime
                if (prime->childCount == 0) break; // epsilon
            }
            result = current;
        } else {
            result = left;
        }
        return result;
    }

    // Handle RelExpr (RelExpr -> ArithExpr RelOpTail)
    if (strcmp(exprNode->symbol, "RelExpr") == 0 && exprNode->childCount >= 1) {
        printf("Debug: Evaluating RelExpr\n");
        ExprValue left = evaluateExpression(exprNode->children[0], table); // ArithExpr
        if (!left.isValid) {
            printf("Debug: RelExpr: Invalid left operand\n");
            return result;
        }
        if (exprNode->childCount == 2 && exprNode->children[1]->childCount > 0) {
            node* relOp = exprNode->children[1]->children[0]; // RelOp
            ExprValue right = evaluateExpression(exprNode->children[1]->children[1], table); // ArithExpr
            if (!right.isValid) {
                printf("Debug: RelExpr: Invalid right operand\n");
                return result;
            }
            result.isValid = 1;
            result.type = TYPE_INT;
            float leftVal = (left.type == TYPE_INT) ? left.value.intValue : left.value.floatValue;
            float rightVal = (right.type == TYPE_INT) ? right.value.intValue : right.value.floatValue;
            printf("Debug: RelExpr operation: %s, left: %f, right: %f\n", relOp->symbol, leftVal, rightVal);
            if (strcmp(relOp->symbol, "==") == 0) {
                result.value.intValue = (leftVal == rightVal);
            } else if (strcmp(relOp->symbol, "!=") == 0) {
                result.value.intValue = (leftVal != rightVal);
            } else if (strcmp(relOp->symbol, "<") == 0) {
                result.value.intValue = (leftVal < rightVal);
            } else if (strcmp(relOp->symbol, "<=") == 0) {
                result.value.intValue = (leftVal <= rightVal);
            } else if (strcmp(relOp->symbol, ">") == 0) {
                result.value.intValue = (leftVal > rightVal);
            } else if (strcmp(relOp->symbol, ">=") == 0) {
                result.value.intValue = (leftVal >= rightVal);
            }
            printf("Debug: RelExpr result: %d\n", result.value.intValue);
        } else {
            result = left;
        }
        return result;
    }

    // Handle Expression (Expression -> RelExpr ExpressionPrime)
    if (strcmp(exprNode->symbol, "Expression") == 0 && exprNode->childCount >= 1) {
        printf("Debug: Evaluating Expression\n");
        return evaluateExpression(exprNode->children[0], table); // RelExpr
    }

    printf("Debug: evaluateExpression: Unhandled node type %s\n", exprNode->symbol);
    return result;
}

void processAssignments(node* root, SymbolTable* table) {
    if (!root) return;
    printf("Debug: Processing node: %s (children: %d)\n", root->symbol, root->childCount);

    // Handle assignment statements (Statement -> id = Expression ;)
    if (strcmp(root->symbol, "Statement") == 0 && root->childCount >= 3 &&
        strcmp(root->children[1]->symbol, "=") == 0 && root->children[0]->isTokenPresent) {
        char* id = root->children[0]->token.data.identifier.lexeme;
        node* exprNode = root->children[2]; // Expression node
        ExprValue exprVal = evaluateExpression(exprNode, table);
        if (exprVal.isValid) {
            int index = findSymbol(table, id);
            if (index != -1) {
                SymbolEntry* entry = &table->entries[index];
                entry->hasValue = 1;
                if (entry->type == TYPE_INT) {
                    entry->value.intValue = exprVal.value.intValue;
                } else {
                    entry->value.floatValue = exprVal.value.floatValue;
                }
                char valueStr[32];
                snprintf(valueStr, sizeof(valueStr), entry->type == TYPE_INT ? "%d" : "%f",
                         entry->type == TYPE_INT ? entry->value.intValue : entry->value.floatValue);
                printf("Debug: Assigned value to %s: %s\n", id, valueStr);
            } else {
                printf("Semantic Error at line %d: Undeclared variable '%s'\n",
                       root->children[0]->token.lineNumber, id);
            }
        } else {
            printf("Semantic Error at line %d: Invalid expression for assignment to '%s'\n",
                   root->children[0]->token.lineNumber, id);
        }
    }

    // Recursively process children
    for (int i = 0; i < root->childCount; i++) {
        processAssignments(root->children[i], table);
    }
}

int parsingTableChecking(tokenNode* head, node** p, SymbolTable* table) {
    stack s;
    initStack(&s);
    node* root = create("Program");
    *p = root;
    push(&s, "$", NULL);
    push(&s, "Program", root);
    tokenNode* currentTokenPtr = head;
    Token currentToken = {0};
    VarType currentType = TYPE_INT;
    char* lastAssignedId = NULL;
    node* currentStatement = NULL; // Track current statement for loop handling

    if (currentTokenPtr) {
        currentToken = getNextToken(&currentTokenPtr);
        printf("Debug: First token: %s (line %d)\n", getTokenString(currentToken), currentToken.lineNumber);
    } else {
        printf("Syntax Error: Empty input\n");
        return 0;
    }

    while (!isEmpty(&s)) {
        printStack(&s);
        StackItem item = pop(&s);
        char* topSymbol = item.symbol;
        node* currentNode = item.treeNode;
        printf("Debug: Popped symbol: %s, isTerminal=%d\n", topSymbol, item.isTerminal);

        // Track statement nodes for loop handling
        if (strcmp(topSymbol, "Statement") == 0) {
            currentStatement = currentNode;
            printf("Debug: Statement node with %d children:", currentNode->childCount);
            for (int i = 0; i < currentNode->childCount; i++) {
                printf(" %s", currentNode->children[i]->symbol);
                if (currentNode->children[i]->isTokenPresent) {
                    printf(" [token: %s]", getTokenString(currentNode->children[i]->token));
                }
            }
            printf("\n");
        }

        int isTerminal = 0;
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(topSymbol, terminals[i]) == 0) {
                isTerminal = 1;
                break;
            }
        }
        if (strcmp(topSymbol, "string") == 0) isTerminal = 1;

        if (isTerminal) {
            if (terminalMatches(topSymbol, currentToken)) {
                if (strcmp(topSymbol, "$") != 0) {
                    currentNode->isTokenPresent = 1;
                    currentNode->token = currentToken;
                    if (strcmp(topSymbol, "id") == 0) {
                        int isDeclaration = 0;
                        node* parent = currentNode->parent;
                        while (parent != NULL) {
                            if (strcmp(parent->symbol, "Statement") == 0) {
                                if (parent->childCount > 0 && strcmp(parent->children[0]->symbol, "Type") == 0) {
                                    isDeclaration = 1;
                                    break;
                                }
                            } else if (strcmp(parent->symbol, "DeclTail") == 0) {
                                if (parent->childCount > 0 && strcmp(parent->children[0]->symbol, ",") == 0) {
                                    isDeclaration = 1;
                                    break;
                                }
                            }
                            parent = parent->parent;
                        }
                        if (isDeclaration) {
                            if (!addSymbol(table, currentToken.data.identifier.lexeme, currentType, currentToken.lineNumber)) {
                                free(topSymbol);
                                return 0;
                            }
                        } else {
                            if (findSymbol(table, currentToken.data.identifier.lexeme) == -1) {
                                printf("Semantic Error at line %d: Undeclared variable '%s'\n",
                                       currentToken.lineNumber, currentToken.data.identifier.lexeme);
                                free(topSymbol);
                                return 0;
                            }
                        }
                    } else if (strcmp(topSymbol, "int") == 0) {
                        currentType = TYPE_INT;
                    } else if (strcmp(topSymbol, "float") == 0) {
                        currentType = TYPE_FLOAT;
                    }
                }
                currentToken = getNextToken(&currentTokenPtr);
                printf("Debug: Advanced to next token: %s\n", getTokenString(currentToken));
                free(topSymbol);
                continue;
            } else {
                printf("Syntax Error at line %d: Expected terminal %s, got %s\n",
                       currentToken.lineNumber, topSymbol, getTokenString(currentToken));
                free(topSymbol);
                return 0;
            }
        }

        if (isNonTerminal(topSymbol)) {
            NonTerminal ntIndex = getNonTerminalEnum(topSymbol);
            Terminal tIndex = getTerminalEnum(NULL, currentToken);

            if (tIndex == -1) {
                printf("Syntax Error at line %d: Invalid token %s\n",
                       currentToken.lineNumber, getTokenString(currentToken));
                free(topSymbol);
                return 0;
            }

            int production = parsingTable[ntIndex][tIndex];
            printf("Debug: Looking up production for non-terminal %s, token %s, got production %d\n",
                   topSymbol, getTokenString(currentToken), production);
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
                            printf("Syntax Error: Production %d does not match non-terminal %s (grammar[%d].lhs = %s)\n",
                                   production, topSymbol, i, grammar[i].lhs);
                            free(topSymbol);
                            return 0;
                        }
                        found = 1;
                        printf("Debug: Validated production %d for %s\n", production, topSymbol);
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
                Token nextTokenCheck = peekNextToken(&currentTokenPtr);
                printf("Debug: Applied epsilon production for %s, next token is %s\n",
                       topSymbol, getTokenString(nextTokenCheck));
                free(topSymbol);
                continue;
            }

            // Handle while loop
            if (strcmp(topSymbol, "Statement") == 0 && currentStatement &&
                currentStatement->childCount > 0 &&
                strcmp(currentStatement->children[0]->symbol, "while") == 0) {
                node* condition = currentStatement->children[2]; // Expression
                node* block = currentStatement->children[4];     // Block
                ExprValue condVal = evaluateExpression(condition, table);

                while (condVal.isValid && condVal.type == TYPE_INT && condVal.value.intValue) {
                    printf("Debug: While loop condition true, executing block\n");
                    // Process all statements in the block
                    processAssignments(block, table);
                    // Re-evaluate the condition
                    condVal = evaluateExpression(condition, table);
                }
                printf("Debug: While loop condition false, exiting loop\n");
                free(topSymbol);
                continue;
            }

            // Handle if statement
            if (strcmp(topSymbol, "Statement") == 0 && currentStatement &&
                currentStatement->childCount > 0 &&
                strcmp(currentStatement->children[0]->symbol, "if") == 0) {
                node* condition = currentStatement->children[2]; // Expression
                node* thenStmt = currentStatement->children[4];  // Statement
                node* elsePart = currentStatement->children[5];  // ElsePart
                ExprValue condVal = evaluateExpression(condition, table);

                if (condVal.isValid && condVal.type == TYPE_INT && condVal.value.intValue) {
                    printf("Debug: If condition true, executing then branch\n");
                    processAssignments(thenStmt, table);
                } else if (elsePart->childCount > 0 && strcmp(elsePart->children[0]->symbol, "else") == 0) {
                    printf("Debug: If condition false, executing else branch\n");
                    processAssignments(elsePart->children[1], table);
                }
                printf("Debug: If statement processed\n");
                free(topSymbol);
                continue;
            }

            int symbolCount = 0;
            while (rhs[symbolCount][0] != '\0') symbolCount++;
            node* tempChildren[10];
            int tempChildCount = 0;
            for (int i = 0; i < symbolCount; i++) {
                node* child = create(rhs[i]);
                child->parent = currentNode;
                tempChildren[tempChildCount++] = child;
                printf("Debug: Created child node for %s\n", rhs[i]);
            }
            for (int i = 0; i < tempChildCount; i++) {
                currentNode->children[i] = tempChildren[i];
                currentNode->childCount++;
                printf("Debug: Added child %s to node %s\n", tempChildren[i]->symbol, currentNode->symbol);
            }
            for (int i = symbolCount - 1; i >= 0; i--) {
                push(&s, rhs[i], tempChildren[i]);
                printf("Debug: Pushed %s to stack\n", rhs[i]);
            }
            printf("Debug: After pushing production symbols, stack is: ");
            printStack(&s);
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

    // Process top-level assignments after parsing
    printf("Debug: Processing top-level assignments\n");
    processAssignments(root, table);

    if (lastAssignedId) free(lastAssignedId);
    printf("\nParsing Successful!\n");
    return 1;
}

char (*getFlatIndex(productionRule* grammar, int prodNum))[maxSymbolLength] {
    for (int i = 0; i < productionCount; i++) {
        for (int j = 0; j < grammar[i].productionCount; j++) {
            if (grammar[i].productionIndex[j] == prodNum) {
                printf("Debug: Found production %d for %s -> ", prodNum, grammar[i].lhs);
                for (int k = 0; grammar[i].rhs[j][k][0] != '\0'; k++) {
                    printf("%s ", grammar[i].rhs[j][k]);
                }
                printf("\n");
                return grammar[i].rhs[j];
            }
        }
    }
    printf("Debug: Production %d not found in grammar\n", prodNum);
    return NULL;
}

const char* getTokenString(Token t) {
    static char buffer[128];
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
        case TOKEN_STRING:
            strcpy(buffer, t.data.string.value);
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
    if (token.type == TOKEN_IDENTIFIER) {
        return T_ID;
    }
    if (token.type == TOKEN_STRING) {
        return T_STRING;
    }
    if (token.type == TOKEN_LITERAL) {
        return T_LITERAL;
    }
    if (token.type == TOKEN_KEYWORD) {
        char* cleaned = cleanTokenString(token.data.keyword.lexeme);
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(cleaned, terminals[i]) == 0) {
                return (Terminal)i;
            }
        }
    }
    if (token.type == TOKEN_OPERATOR) {
        char* cleanedOp = cleanTokenString(token.data.operator.op);
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(cleanedOp, terminals[i]) == 0) {
                return (Terminal)i;
            }
        }
    }
    if (token.type == TOKEN_SEPARATOR) {
        char temp[2] = {token.data.separator.symbol, '\0'};
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(temp, terminals[i]) == 0) {
                return (Terminal)i;
            }
        }
    }
    printf("Debug: Failed to map token %s to terminal\n", getTokenString(token));
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
        printf(" [token: %s, line: %d]", getTokenString(root->token), root->token.lineNumber);
    }
    printf(" (children: %d)", root->childCount);
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

    printf("Debug: Reading input file:\n");
    while (fgets(content, sizeof(content), fptr)) {
        printf("Debug: Line %d: %s", lineNumber, content);
        strcat(fullContent, content);
        if (strchr(content, '\n')) lineNumber++;
    }
    fclose(fptr);
    printf("Debug: Full input content:\n%s\n", fullContent);

    ParsingTableInitialized();

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
            case TOKEN_STRING:
                printf("String: %s (line %d)\n", t.data.string.value, t.lineNumber);
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

    SymbolTable table;
    initSymbolTable(&table);
    node* root = NULL;
    int result = parsingTableChecking(head, &root, &table);

    if (result) {
        printf("\nParsing Successful!\n\n");
        printf("--------------- Parse Tree ---------------\n");
        printParseTree(root, 0);
        printf("------------------------------------------\n");
        printSymbolTable(&table);
    } else {
        printf("\nParsing Failed.\n");
    }

    freeTokenList(headCopy);
    freeNode(root);
    return 0;
}
