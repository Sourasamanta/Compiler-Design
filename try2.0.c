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
    struct tree* parent;
    Token token;
    int isTokenPresent;
    VarType symbolType;
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
char* getTokenString(Token t, char* buffer, size_t bufferSize);
char* cleanTokenString(char* token);
char (*getFlatIndex(productionRule* grammar, int prodNum))[maxSymbolLength];
int parsingTableChecking(tokenNode* head, node** p, SymbolTable* table);
Token getNextToken(tokenNode** current);
void ungetToken(Token token);
int isNonTerminal(const char* symbol);
void ParsingTableInitialized(void);
void initSymbolTable(SymbolTable* table);
int addSymbol(SymbolTable* table, const char* name, VarType type, int lineNumber);
int findSymbol(SymbolTable* table, const char* name);
void printValue(SymbolEntry* entry);
double evaluateExpression(node* n, SymbolTable* table);
double evaluateRelExpr(node* n, SymbolTable* table);
double evaluateArithExpr(node* n, SymbolTable* table);
double evaluateTerm(node* n, SymbolTable* table);
double evaluateFactor(node* n, SymbolTable* table);
int executeBlock(node* n, SymbolTable* table);
void executeAssignment(node* n, SymbolTable* table);
void executeReturn(node* n, SymbolTable* table);
void executeIfStatement(node* n, SymbolTable* table);
void executeWhileStatement(node* n, SymbolTable* table);
void executePrintf(node* n, SymbolTable* table);
void updateSymbol(SymbolTable* table, int index, double value);
double executeReturnStmt(node* returnStmt, SymbolTable* table);
double executeStatementList(node* statementList, SymbolTable* table, int* hasReturned);
double executeStatement(node* statement, SymbolTable* table, int* hasReturned);
double executeProgram(node* program, SymbolTable* table);

// Global variable to store return value and signal termination
static double returnValue = 0.0;
static int hasReturned = 0;

// Function to reset return state (call before starting execution)
void resetReturnState() {
    returnValue = 0.0;
    hasReturned = 0;
}

// Function to check if a return has occurred
int hasProgramReturned() {
    return hasReturned;
}

// Function to get the return value
double getReturnValue() {
    return returnValue;
}

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
    if (!chars) {
        printf("Error: Attempting to push NULL symbol\n");
        exit(EXIT_FAILURE);
    }
    if (strlen(chars) >= maxSymbolLength) {
        printf("Error: Symbol '%s' exceeds max length of %d\n", chars, maxSymbolLength - 1);
        exit(EXIT_FAILURE);
    }
    p->items[++(p->top)].symbol = strdup(chars);
    if (!p->items[p->top].symbol) {
        printf("Error: Memory allocation failed for symbol '%s'\n", chars);
        exit(EXIT_FAILURE);
    }
    p->items[p->top].treeNode = treeNode;
    p->items[p->top].isTerminal = isNonTerminal(chars) ? 0 : 1;
    printf("Debug: Pushed symbol '%s' to stack\n", chars);
}

void freeNode(node* n) {
    if (n == NULL) return;
    for (int i = 0; i < n->childCount; i++) {
        freeNode(n->children[i]);
    }
    free(n);
}

StackItem pop(stack *s) {
    StackItem item = {NULL, 0, NULL};
    if (!isEmpty(s)) {
        item = s->items[s->top--];
        printf("Debug: Popped symbol '%s'\n", item.symbol ? item.symbol : "NULL");
    } else {
        printf("Error: Stack underflow\n");
    }
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
    tokenNode* current = head;
    while (current != NULL) {
        tokenNode* temp = current;
        current = current->next;
        switch (temp->token.type) {
            case TOKEN_KEYWORD:
                free(temp->token.data.keyword.lexeme);
                break;
            case TOKEN_IDENTIFIER:
                free(temp->token.data.identifier.lexeme);
                break;
            case TOKEN_LITERAL:
                free(temp->token.data.literal.value);
                break;
            case TOKEN_STRING:
                free(temp->token.data.string.value);
                break;
            case TOKEN_OPERATOR:
                free(temp->token.data.operator.op);
                break;
            default:
                break;
        }
        free(temp);
    }
}

tokenNode* check(const char *content, int lineNumber) {
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
            printf("Debug: Found single-line comment start at position %d, line %d\n", i, lineNumber);
            while (content[i] != '\n' && content[i] != '\0') i++;
            if (content[i] == '\n') lineNumber++;
            i++;
            printf("Debug: Skipped single-line comment, now at position %d, line %d\n", i, lineNumber);
            continue;
        }

        if (content[i] == '/' && content[i + 1] == '*') {
            printf("Debug: Found multi-line comment start at position %d, line %d\n", i, lineNumber);
            i += 2;
            while (content[i] != '\0' && !(content[i] == '*' && content[i + 1] == '/')) {
                if (content[i] == '\n') lineNumber++;
                i++;
            }
            if (content[i] == '*' && content[i + 1] == '/') {
                i += 2;
                printf("Debug: Skipped multi-line comment, now at position %d, line %d\n", i, lineNumber);
                continue;
            } else {
                printf("Syntax Error at line %d: Unclosed multi-line comment\n", lineNumber);
                freeTokenList(head);
                return NULL;
            }
        }

        Token t = {0}; // Initialize token to prevent undefined behavior
        t.lineNumber = lineNumber;
        printf("Debug: Creating token at position %d, line %d\n", i, lineNumber);

        if (isalpha(content[i]) || content[i] == '_') {
            int start = i;
            while (isalnum(content[i]) || content[i] == '_') i++;
            int len = i - start;
            if (len >= sizeof(t.data.keyword.lexeme)) len = sizeof(t.data.keyword.lexeme) - 1;
            strncpy(t.data.keyword.lexeme, &content[start], len);
            t.data.keyword.lexeme[len] = '\0';
            printf("Debug: Extracted lexeme '%s' (length %d)\n", t.data.keyword.lexeme, len);

            if (strcmp(t.data.keyword.lexeme, "int") == 0 || strcmp(t.data.keyword.lexeme, "float") == 0 ||
                strcmp(t.data.keyword.lexeme, "return") == 0 || strcmp(t.data.keyword.lexeme, "if") == 0 ||
                strcmp(t.data.keyword.lexeme, "else") == 0 || strcmp(t.data.keyword.lexeme, "while") == 0 ||
                strcmp(t.data.keyword.lexeme, "printf") == 0) {
                t.type = TOKEN_KEYWORD;
                printf("Debug: Token is KEYWORD: %s\n", t.data.keyword.lexeme);
            } else {
                t.type = TOKEN_IDENTIFIER;
                strncpy(t.data.identifier.lexeme, t.data.keyword.lexeme, sizeof(t.data.identifier.lexeme));
                t.data.identifier.lexeme[sizeof(t.data.identifier.lexeme) - 1] = '\0';
                printf("Debug: Token is IDENTIFIER: %s\n", t.data.identifier.lexeme);
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
            if (len >= sizeof(t.data.literal.value)) len = sizeof(t.data.literal.value) - 1;
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
            if (len >= sizeof(t.data.string.value)) len = sizeof(t.data.string.value) - 1;
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
    // Append end-of-input token
    Token endToken = { .type = TOKEN_SEPARATOR, .data.separator.symbol = '$', .lineNumber = lineNumber };
    head = append(endToken, head);
    printf("Debug: Lexer finished, returning token list\n");
    return head;
}

// Initialize the parsing table and grammar productions
void ParsingTableInitialized(void) {
    flatIndexCounter = 1;
    productionCount = 0;

    // Initialize parsing table with -1 (no production)
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

    // 53: Program -> epsilon (new production for empty input)
    strcpy(grammar[productionCount].lhs, "Program");
    grammar[productionCount].productionCount = 1;
    strcpy(grammar[productionCount].rhs[0][0], "epsilon");
    grammar[productionCount].rhs[0][1][0] = '\0';
    grammar[productionCount].productionIndex[0] = flatIndexCounter++;
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
    parsingTable[NT_PROGRAM][T_DOLLAR] = 53; // New mapping for empty program

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

char* cleanTokenString(char* token) {
    if (!token) return NULL;
    int len = strlen(token);
    char* result = malloc(len + 1);
    if (!result) {
        printf("Error: Memory allocation failed in cleanTokenString\n");
        exit(1);
    }
    strcpy(result, token);
    while (len > 0 && (result[len - 1] == ' ' || result[len - 1] == '\n' || result[len - 1] == '\t')) {
        result[--len] = '\0';
    }
    return result;
}

int terminalMatches(const char* terminal, Token token) {
    if (strcmp(terminal, "$") == 0) {
        return token.lineNumber == -1 || token.data.separator.symbol == '$';
    }
    if (strcmp(terminal, "id") == 0) {
        return token.type == TOKEN_IDENTIFIER;
    }
    if (strcmp(terminal, "string") == 0) {
        return token.type == TOKEN_STRING;
    }
    if (strcmp(terminal, "literal") == 0) {
        return token.type == TOKEN_LITERAL;
    }
    char* cleaned = NULL;
    int result = 0;
    if (token.type == TOKEN_KEYWORD) {
        cleaned = cleanTokenString(token.data.keyword.lexeme);
        if (strcmp(terminal, "int") == 0) result = strcmp(cleaned, "int") == 0;
        else if (strcmp(terminal, "float") == 0) result = strcmp(cleaned, "float") == 0;
        else if (strcmp(terminal, "return") == 0) result = strcmp(cleaned, "return") == 0;
        else if (strcmp(terminal, "if") == 0) result = strcmp(cleaned, "if") == 0;
        else if (strcmp(terminal, "else") == 0) result = strcmp(cleaned, "else") == 0;
        else if (strcmp(terminal, "while") == 0) result = strcmp(cleaned, "while") == 0;
        else if (strcmp(terminal, "printf") == 0) result = strcmp(cleaned, "printf") == 0;
    } else if (token.type == TOKEN_OPERATOR) {
        cleaned = cleanTokenString(token.data.operator.op);
        if (strcmp(terminal, "=") == 0) result = strcmp(cleaned, "=") == 0;
        else if (strcmp(terminal, "+") == 0) result = strcmp(cleaned, "+") == 0;
        else if (strcmp(terminal, "-") == 0) result = strcmp(cleaned, "-") == 0;
        else if (strcmp(terminal, "*") == 0) result = strcmp(cleaned, "*") == 0;
        else if (strcmp(terminal, "/") == 0) result = strcmp(cleaned, "/") == 0;
        else if (strcmp(terminal, "!") == 0) result = strcmp(cleaned, "!") == 0;
        else if (strcmp(terminal, "<") == 0) result = strcmp(cleaned, "<") == 0;
        else if (strcmp(terminal, "<=") == 0) result = strcmp(cleaned, "<=") == 0;
        else if (strcmp(terminal, ">") == 0) result = strcmp(cleaned, ">") == 0;
        else if (strcmp(terminal, ">=") == 0) result = strcmp(cleaned, ">=") == 0;
        else if (strcmp(terminal, "==") == 0) result = strcmp(cleaned, "==") == 0;
        else if (strcmp(terminal, "!=") == 0) result = strcmp(cleaned, "!=") == 0;
        else if (strcmp(terminal, "&&") == 0) result = strcmp(cleaned, "&&") == 0;
        else if (strcmp(terminal, "||") == 0) result = strcmp(cleaned, "||") == 0;
    } else if (token.type == TOKEN_SEPARATOR) {
        char temp[2] = {token.data.separator.symbol, '\0'};
        if (strcmp(terminal, ";") == 0) result = token.data.separator.symbol == ';';
        else if (strcmp(terminal, "(") == 0) result = token.data.separator.symbol == '(';
        else if (strcmp(terminal, ")") == 0) result = token.data.separator.symbol == ')';
        else if (strcmp(terminal, ",") == 0) result = token.data.separator.symbol == ',';
        else if (strcmp(terminal, "{") == 0) result = token.data.separator.symbol == '{';
        else if (strcmp(terminal, "}") == 0) result = token.data.separator.symbol == '}';
        else if (strcmp(terminal, "@") == 0) result = token.data.separator.symbol == '@';
        else if (strcmp(terminal, "~") == 0) result = token.data.separator.symbol == '~';
    }
    free(cleaned); // Free the cleaned string
    return result;
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


// Modified evaluateExpression function
double evaluateExpression(node* n, SymbolTable* table) {
    printf("Debug: Evaluating expression node: %s (children: %d, line: %d)\n",
           n->symbol, n->childCount, n->token.lineNumber);
    if (strcmp(n->symbol, "Expression") != 0 || n->childCount < 1) {
        printf("Error: Invalid Expression node at line %d\n", n->token.lineNumber);
        return 0.0;
    }
    if (!n->children[0] || strcmp(n->children[0]->symbol, "RelExpr") != 0) {
        printf("Error: Expression missing RelExpr at line %d\n", n->token.lineNumber);
        return 0.0;
    }
    double value = evaluateRelExpr(n->children[0], table);
    if (n->childCount > 1 && strcmp(n->children[1]->symbol, "ExpressionPrime") == 0) {
        node* exprPrime = n->children[1];
        if (exprPrime->childCount >= 3) {
            if (!exprPrime->children[0] || !exprPrime->children[0]->isTokenPresent) {
                printf("Error: Missing operator in ExpressionPrime at line %d\n",
                       exprPrime->token.lineNumber);
                return 0.0;
            }
            char* op = exprPrime->children[0]->token.data.operator.op;
            if (!exprPrime->children[1] || strcmp(exprPrime->children[1]->symbol, "RelExpr") != 0) {
                printf("Error: ExpressionPrime missing RelExpr at line %d\n",
                       exprPrime->token.lineNumber);
                return 0.0;
            }
            double right = evaluateRelExpr(exprPrime->children[1], table);
            if (strcmp(op, "||") == 0) return (value != 0.0 || right != 0.0) ? 1.0 : 0.0;
            if (strcmp(op, "&&") == 0) return (value != 0.0 && right != 0.0) ? 1.0 : 0.0;
        }
    }
    return value;
}

double evaluateRelExpr(node* n, SymbolTable* table) {
    printf("Debug: Evaluating RelExpr\n");
    if (strcmp(n->symbol, "RelExpr") != 0 || n->childCount < 1) {
        printf("Error: Invalid RelExpr node at line %d\n", n->token.lineNumber);
        return 0.0;
    }
    if (!n->children[0] || strcmp(n->children[0]->symbol, "ArithExpr") != 0) {
        printf("Error: RelExpr missing ArithExpr at line %d\n", n->token.lineNumber);
        return 0.0;
    }
    double value = evaluateArithExpr(n->children[0], table);
    if (n->childCount > 1 && strcmp(n->children[1]->symbol, "RelOpTail") == 0) {
        node* relOpTail = n->children[1];
        if (relOpTail->childCount >= 3) {
            if (!relOpTail->children[0] || strcmp(relOpTail->children[0]->symbol, "RelOp") != 0 ||
                !relOpTail->children[0]->children[0] || !relOpTail->children[0]->children[0]->isTokenPresent) {
                printf("Error: Invalid RelOp in RelOpTail at line %d\n",
                       relOpTail->token.lineNumber);
                return 0.0;
            }
            char* op = relOpTail->children[0]->children[0]->token.data.operator.op;
            if (!relOpTail->children[1] || strcmp(relOpTail->children[1]->symbol, "ArithExpr") != 0) {
                printf("Error: RelOpTail missing ArithExpr at line %d\n",
                       relOpTail->token.lineNumber);
                return 0.0;
            }
            double right = evaluateArithExpr(relOpTail->children[1], table);
            if (strcmp(op, ">") == 0) return (value > right) ? 1.0 : 0.0;
            if (strcmp(op, "<") == 0) return (value < right) ? 1.0 : 0.0;
            if (strcmp(op, ">=") == 0) return (value >= right) ? 1.0 : 0.0;
            if (strcmp(op, "<=") == 0) return (value <= right) ? 1.0 : 0.0;
            if (strcmp(op, "==") == 0) return (value == right) ? 1.0 : 0.0;
            if (strcmp(op, "!=") == 0) return (value != right) ? 1.0 : 0.0;
        }
    }
    return value;
}

double evaluateArithExpr(node* n, SymbolTable* table) {
    if (!n || strcmp(n->symbol, "ArithExpr") != 0) {
        printf("Error: Invalid ArithExpr node\n");
        return 0.0;
    }
    double left = evaluateTerm(n->children[0], table);
    if (n->childCount == 1) { // Changed from n->childCount == 2 to handle single Term
        n->symbolType = n->children[0]->symbolType;
        return left;
    }
    node* arithExprPrime = n->children[1];
    if (strcmp(arithExprPrime->symbol, "ArithExprPrime") != 0) {
        printf("Error: Invalid ArithExprPrime in ArithExpr at line %d\n", n->token.lineNumber);
        return left;
    }
    if (arithExprPrime->childCount == 0 || strcmp(arithExprPrime->children[0]->symbol, "epsilon") == 0) {
        n->symbolType = n->children[0]->symbolType;
        return left;
    }
    node* opNode = arithExprPrime->children[0];
    double right = evaluateTerm(arithExprPrime->children[1], table);
    n->symbolType = (n->children[0]->symbolType == TYPE_INT && arithExprPrime->children[1]->symbolType == TYPE_INT) ? TYPE_INT : TYPE_FLOAT;
    switch (opNode->token.data.operator.op[0]) {
        case '+':
            return left + right;
        case '-':
            return left - right;
        default:
            printf("Error: Unknown operator %s at line %d\n",
                   opNode->token.data.operator.op, opNode->token.lineNumber);
            return 0.0;
    }
}

double evaluateTerm(node* n, SymbolTable* table) {
    printf("Debug: Evaluating Term\n");
    if (strcmp(n->symbol, "Term") != 0 || n->childCount < 1) {
        printf("Error: Invalid Term node at line %d\n", n->token.lineNumber);
        return 0.0;
    }
    if (!n->children[0] || strcmp(n->children[0]->symbol, "Factor") != 0) {
        printf("Error: Term missing Factor at line %d\n", n->token.lineNumber);
        return 0.0;
    }
    double value = evaluateFactor(n->children[0], table);
    n->symbolType = n->children[0]->symbolType; // Propagate Factor type
    if (n->childCount > 1 && strcmp(n->children[1]->symbol, "TermPrime") == 0) {
        node* termPrime = n->children[1];
        if (termPrime->childCount >= 3) {
            if (!termPrime->children[0] || !termPrime->children[0]->isTokenPresent) {
                printf("Error: Missing operator in TermPrime at line %d\n",
                       termPrime->token.lineNumber);
                return 0.0;
            }
            char* op = termPrime->children[0]->token.data.operator.op;
            if (!termPrime->children[1] || strcmp(termPrime->children[1]->symbol, "Factor") != 0) {
                printf("Error: TermPrime missing Factor at line %d\n",
                       termPrime->token.lineNumber);
                return 0.0;
            }
            double right = evaluateFactor(termPrime->children[1], table);
            // Set type: int if both operands are int, else float
            n->symbolType = (n->children[0]->symbolType == TYPE_INT && termPrime->children[1]->symbolType == TYPE_INT) ? TYPE_INT : TYPE_FLOAT;
            if (strcmp(op, "*") == 0) return value * right;
            if (strcmp(op, "/") == 0) {
                if (right == 0.0) {
                    printf("Error: Division by zero at line %d\n", termPrime->token.lineNumber);
                    return 0.0;
                }
                if (n->symbolType == TYPE_INT) {
                    return (double)((int)value / (int)right);
                }
                return value / right;
            }
        }
    }
    return value;
}

double evaluateFactor(node* n, SymbolTable* table) {
    printf("Debug: Evaluating Factor (children: %d, line: %d)\n",
           n->childCount, n->token.lineNumber);
    if (strcmp(n->symbol, "Factor") != 0) {
        printf("Error: Invalid Factor node at line %d\n", n->token.lineNumber);
        return 0.0;
    }

    if (n->childCount == 0) {
        printf("Error: Factor node has no children at line %d\n", n->token.lineNumber);
        return 0.0;
    }

    node* child = n->children[0];

    // Case 1: Factor -> id
    if (strcmp(child->symbol, "id") == 0 && child->isTokenPresent) {
        int index = findSymbol(table, child->token.data.identifier.lexeme);
        if (index == -1) {
            printf("Semantic Error at line %d: Undeclared variable '%s'\n",
                   child->token.lineNumber, child->token.data.identifier.lexeme);
            return 0.0;
        }
        if (!table->entries[index].hasValue) {
            printf("Semantic Warning at line %d: Variable '%s' used before initialization\n",
                   child->token.lineNumber, child->token.data.identifier.lexeme);
            return 0.0;
        }
        double value = table->entries[index].type == TYPE_INT ?
                       (double)table->entries[index].value.intValue :
                       (double)table->entries[index].value.floatValue;
        n->symbolType = table->entries[index].type; // Set parent type
        printf("Debug: ID %s, found at index %d, value: %f\n",
               child->token.data.identifier.lexeme, index, value);
        return value;
    }
    // Case 2: Factor -> literal
    else if (strcmp(child->symbol, "literal") == 0 && child->isTokenPresent) {
        double value = atof(child->token.data.literal.value);
        n->symbolType = child->symbolType; // Propagate literal type
        printf("Debug: Literal %s evaluated to: %f\n",
               child->token.data.literal.value, value);
        return value;
    }
    // Case 3: Factor -> ( Expression )
    else if (strcmp(child->symbol, "(") == 0 && n->childCount >= 3) {
        if (strcmp(n->children[1]->symbol, "Expression") != 0 ||
            strcmp(n->children[2]->symbol, ")") != 0) {
            printf("Error: Invalid parenthesized expression at line %d\n",
                   n->token.lineNumber);
            return 0.0;
        }
        double value = evaluateExpression(n->children[1], table);
        n->symbolType = n->children[1]->symbolType; // Propagate Expression type
        printf("Debug: Evaluated parenthesized expression to %f\n", value);
        return value;
    }
    // Case 4: Factor -> UnaryOp Factor
    else if (strcmp(child->symbol, "UnaryOp") == 0 && n->childCount >= 2) {
        node* unaryOp = n->children[0];
        node* factor = n->children[1];
        if (unaryOp->childCount < 1 || !unaryOp->children[0] || !unaryOp->children[0]->isTokenPresent) {
            printf("Error: Invalid UnaryOp structure at line %d\n",
                   n->token.lineNumber);
            return 0.0;
        }
        node* opNode = unaryOp->children[0];
        char* op = opNode->token.data.operator.op;
        double value = evaluateFactor(factor, table);
        n->symbolType = factor->symbolType; // Propagate Factor type
        printf("Debug: Applying unary operator %s to value %f\n", op, value);
        if (strcmp(op, "-") == 0) {
            return -value;
        }
        else if (strcmp(op, "!") == 0) {
            n->symbolType = TYPE_INT; // NOT produces boolean (int)
            return (value == 0.0) ? 1.0 : 0.0;
        }
        else if (strcmp(op, "+") == 0) {
            return value;
        }
        else {
            printf("Error: Unknown unary operator %s at line %d\n",
                   op, opNode->token.lineNumber);
            return 0.0;
        }
    }

    printf("Error: Invalid Factor node structure at line %d\n", n->token.lineNumber);
    return 0.0;
}

void printValue(SymbolEntry* entry) {
    if (!entry->hasValue) {
        printf("(uninitialized)");
        return;
    }
    if (entry->type == TYPE_INT) {
        printf("%d", entry->value.intValue);
    } else {
        printf("%f", entry->value.floatValue);
    }
}

double executeStatement(node* statement, SymbolTable* table, int* hasReturned) {
    printf("Debug: Executing statement: Statement (children: %d, line: %d)\n",
           statement->childCount, statement->token.lineNumber);
    if (!statement || strcmp(statement->symbol, "Statement") != 0) {
        printf("Error: Invalid Statement node at line %d\n", statement->token.lineNumber);
        return 0.0;
    }
    node* child = statement->children[0];
    if (strcmp(child->symbol, "Block") == 0) {
        printf("Debug: Executing block (children: %d) at line %d\n",
               child->childCount, child->token.lineNumber);
        return executeStatementList(child->children[1], table, hasReturned);
    } else if (strcmp(child->symbol, "Type") == 0) {
        printf("Debug: Skipping declaration statement\n");
        return 0.0; // Declarations don't return values
    } else if (strcmp(child->symbol, "id") == 0 && statement->childCount >= 3 &&
               strcmp(statement->children[1]->symbol, "=") == 0) {
        executeAssignment(statement, table);
        return 0.0; // Assignments don't return values
    } else if (strcmp(child->symbol, "ReturnStmt") == 0) {
        printf("Debug: Executing ReturnStmt\n");
        double value = executeReturnStmt(child, table);
        *hasReturned = 1;
        printf("Debug: Returning value %f at line %d\n",
               value, child->token.lineNumber);
        return value;
    } else if (strcmp(child->symbol, "if") == 0) {
        executeIfStatement(statement, table);
        return 0.0; // If statements don't return values
    } else if (strcmp(child->symbol, "while") == 0) {
        executeWhileStatement(statement, table);
        return 0.0; // While statements don't return values
    } else if (strcmp(child->symbol, "PrintfStmt") == 0) {
        printf("Debug: Executing PrintfStmt\n");
        executePrintf(child, table); // Pass child directly
        return 0.0;
    } else {
        printf("Error: Unhandled Statement type %s at line %d\n",
               child->symbol, child->token.lineNumber);
        return 0.0;
    }
}

double executeReturnStmt(node* returnStmt, SymbolTable* table) {
    if (!returnStmt || strcmp(returnStmt->symbol, "ReturnStmt") != 0) {
        printf("Error: Invalid ReturnStmt node\n");
        return 0.0;
    }
    if (returnStmt->childCount < 2) {
        printf("Error: ReturnStmt missing expression\n");
        return 0.0;
    }
    return evaluateExpression(returnStmt->children[1], table);
}


int executeBlock(node* n, SymbolTable* table) {
    if (!n || strcmp(n->symbol, "Block") != 0) {
        printf("Error: Invalid block node\n");
        return 0; // Continue execution
    }
    printf("Debug: Executing block (children: %d) at line %d\n",
           n->childCount, n->children[0]->token.lineNumber);

    if (n->childCount < 3) {
        printf("Error: Incomplete block node (children: %d)\n", n->childCount);
        return 0; // Continue execution
    }

    // Validate block structure: { StatementList }
    if (strcmp(n->children[0]->symbol, "{") != 0 ||
        strcmp(n->children[1]->symbol, "StatementList") != 0 ||
        strcmp(n->children[2]->symbol, "}") != 0) {
        printf("Error: Malformed block structure\n");
        return 0; // Continue execution
    }

    int hasReturned = 0;
    executeStatementList(n->children[1], table, &hasReturned);
    return hasReturned; // Propagate return status
}

void executeAssignment(node* n, SymbolTable* table) {
    if (n->childCount < 4) {
        printf("Error: Invalid assignment node (children: %d) at line %d\n",
               n->childCount, n->token.lineNumber);
        return;
    }
    node* idNode = n->children[0];
    node* exprNode = n->children[2];
    if (strcmp(idNode->symbol, "id") != 0 || !idNode->isTokenPresent) {
        printf("Error: Invalid id node in assignment at line %d\n",
               idNode->token.lineNumber);
        return;
    }
    int index = findSymbol(table, idNode->token.data.identifier.lexeme);
    if (index == -1) {
        printf("Semantic Error at line %d: Undeclared variable '%s'\n",
               idNode->token.lineNumber, idNode->token.data.identifier.lexeme);
        return;
    }
    double value = evaluateExpression(exprNode, table);
    if (table->entries[index].type == TYPE_INT && value != (int)value) {
        printf("Warning: Assigning non-integer value %f to integer variable '%s' at line %d\n",
               value, idNode->token.data.identifier.lexeme, idNode->token.lineNumber);
    }
    printf("Debug: Assigning %f to %s\n", value, idNode->token.data.identifier.lexeme);
    updateSymbol(table, index, value);
}

void updateSymbol(SymbolTable* table, int index, double value) {
    if (index < 0 || index >= table->count) {
        printf("Error: Invalid symbol table index %d\n", index);
        return;
    }
    SymbolEntry* entry = &table->entries[index];
    if (entry->type == TYPE_INT) {
        entry->value.intValue = (int)value;
        printf("Debug: Updated symbol %s to value %d\n", entry->name, entry->value.intValue);
    } else {
        entry->value.floatValue = (float)value;
        printf("Debug: Updated symbol %s to value %f\n", entry->name, entry->value.floatValue);
    }
    entry->hasValue = 1;
}

double executeProgram(node* program, SymbolTable* table) {
    printf("Debug: Starting program execution\n");
    if (!program || strcmp(program->symbol, "Program") != 0) {
        printf("Error: Invalid program node\n");
        return 0.0;
    }
    if (program->childCount == 0 || (program->childCount == 1 && strcmp(program->children[0]->symbol, "epsilon") == 0)) {
        printf("Debug: Empty program, returning 0.0\n");
        return 0.0;
    }
    if (program->childCount < 1 || strcmp(program->children[0]->symbol, "StatementList") != 0) {
        printf("Error: Program does not contain StatementList\n");
        return 0.0;
    }
    int hasReturned = 0;
    double result = executeStatementList(program->children[0], table, &hasReturned);
    printf("Debug: Program execution completed with return value %f\n", result);
    return result;
}

double executeStatementList(node* statementList, SymbolTable* table, int* hasReturned) {
    printf("Debug: Executing statement: StatementList (children: %d)\n", statementList->childCount);
    if (!statementList || strcmp(statementList->symbol, "StatementList") != 0) {
        printf("Error: Invalid StatementList node\n");
        return 0.0;
    }
    if (statementList->childCount == 0 || (statementList->childCount == 1 && strcmp(statementList->children[0]->symbol, "epsilon") == 0)) {
        printf("Debug: Empty StatementList\n");
        return 0.0; // Empty statement list
    }
    double result = 0.0;
    for (int i = 0; i < statementList->childCount && !*hasReturned; i++) {
        node* child = statementList->children[i];
        if (strcmp(child->symbol, "Statement") == 0) {
            result = executeStatement(child, table, hasReturned);
        } else if (strcmp(child->symbol, "StatementList") == 0) {
            result = executeStatementList(child, table, hasReturned);
        } else {
            printf("Error: Unexpected child node %s in StatementList at line %d\n",
                   child->symbol, child->token.lineNumber);
        }
        if (*hasReturned) {
            printf("Debug: Return statement encountered, stopping execution\n");
            break;
        }
    }
    return result;
}

void executeReturn(node* n, SymbolTable* table) {
    if (!n || strcmp(n->symbol, "ReturnStmt") != 0) {
        printf("Error: Invalid return statement node\n");
        return;
    }
    if (n->childCount < 3) {
        printf("Error: Invalid return node (children: %d, expected at least 3)\n", n->childCount);
        return;
    }
    node* exprNode = n->children[1];
    if (strcmp(exprNode->symbol, "Expression") != 0) {
        printf("Error: Invalid expression in return statement at line %d\n",
               n->children[0]->token.lineNumber);
        return;
    }
    double value = evaluateExpression(exprNode, table);
    printf("Debug: Returning value %f at line %d\n", value, n->children[0]->token.lineNumber);
    returnValue = value;
    hasReturned = 1;
}

void executeIfStatement(node* n, SymbolTable* table) {
    printf("Debug: Executing IfStatement (children: %d) at line %d\n",
           n->childCount, n->token.lineNumber);
    if (!n || strcmp(n->symbol, "Statement") != 0 || n->childCount < 6 ||
        strcmp(n->children[0]->symbol, "if") != 0 ||
        strcmp(n->children[1]->symbol, "(") != 0 ||
        strcmp(n->children[2]->symbol, "Expression") != 0 ||
        strcmp(n->children[3]->symbol, ")") != 0 ||
        strcmp(n->children[4]->symbol, "Statement") != 0 ||
        strcmp(n->children[5]->symbol, "ElsePart") != 0) {
        printf("Error: Invalid IfStatement node structure at line %d\n",
               n->token.lineNumber);
        return;
    }

    // Evaluate the condition
    double conditionValue = evaluateExpression(n->children[2], table);
    printf("Debug: Condition evaluated to %f\n", conditionValue);

    int hasReturned = 0;
    if (conditionValue != 0.0) {
        // Execute then-branch
        printf("Debug: Executing then-branch at line %d\n", n->children[4]->token.lineNumber);
        executeStatement(n->children[4], table, &hasReturned);
    } else {
        // Execute else-branch (if present)
        node* elsePart = n->children[5];
        if (elsePart->childCount >= 2 && strcmp(elsePart->children[0]->symbol, "else") == 0 &&
            strcmp(elsePart->children[1]->symbol, "Statement") == 0) {
            printf("Debug: Executing else-branch at line %d\n",
                   elsePart->children[1]->token.lineNumber);
            executeStatement(elsePart->children[1], table, &hasReturned);
        } else {
            printf("Debug: No else-branch, skipping\n");
        }
    }
}

void executeWhileStatement(node* n, SymbolTable* table) {
    printf("Debug: Executing WhileStatement (children: %d) at line %d\n",
           n->childCount, n->token.lineNumber);
    if (!n || strcmp(n->symbol, "Statement") != 0 || n->childCount < 5 ||
        strcmp(n->children[0]->symbol, "while") != 0 ||
        strcmp(n->children[1]->symbol, "(") != 0 ||
        strcmp(n->children[2]->symbol, "Expression") != 0 ||
        strcmp(n->children[3]->symbol, ")") != 0 ||
        strcmp(n->children[4]->symbol, "Block") != 0) {
        printf("Error: Invalid WhileStatement node structure at line %d\n",
               n->token.lineNumber);
        return;
    }

    node* conditionNode = n->children[2];
    node* blockNode = n->children[4];
    int hasReturned = 0;

    while (!hasReturned && evaluateExpression(conditionNode, table) != 0.0) {
        printf("Debug: While condition is true, executing block at line %d\n",
               blockNode->token.lineNumber);
        hasReturned = executeBlock(blockNode, table);
    }
    printf("Debug: Exited while loop at line %d\n", n->token.lineNumber);
}

void executePrintf(node* n, SymbolTable* table) {
    printf("Debug: Executing PrintfStmt at line %d (children: %d)\n",
           n->token.lineNumber, n->childCount);
    if (!n || strcmp(n->symbol, "PrintfStmt") != 0 || n->childCount != 6) {
        printf("Error: Invalid PrintfStmt node at line %d (children: %d)\n",
               n->token.lineNumber, n->childCount);
        return;
    }
    // Validate PrintfStmt structure: printf @ string PrintTail @ ;
    if (strcmp(n->children[0]->symbol, "printf") != 0 ||
        strcmp(n->children[1]->symbol, "@") != 0 ||
        strcmp(n->children[2]->symbol, "string") != 0 ||
        strcmp(n->children[3]->symbol, "PrintTail") != 0 ||
        strcmp(n->children[4]->symbol, "@") != 0 ||
        strcmp(n->children[5]->symbol, ";") != 0) {
        printf("Error: Invalid PrintfStmt structure at line %d\n",
               n->token.lineNumber);
        return;
    }
    node* stringNode = n->children[2];
    if (!stringNode->isTokenPresent) {
        printf("Error: Missing string token in PrintfStmt at line %d\n",
               stringNode->token.lineNumber);
        return;
    }
    char* format = stringNode->token.data.string.value;
    char* cursor = format;
    node* printTail = n->children[3];
    while (*cursor != '\0') {
        if (*cursor == '~' && *(cursor + 1) == 'a' && *(cursor + 2) == '\0') {
            // Special case: handle "~a" with empty PrintTail
            int index = findSymbol(table, "a");
            if (index == -1) {
                printf("Semantic Error: Undeclared variable 'a' at line %d\n",
                       n->token.lineNumber);
                return;
            }
            SymbolEntry* entry = &table->entries[index];
            if (entry->hasValue) {
                if (entry->type == TYPE_INT) {
                    printf("%d", entry->value.intValue);
                } else {
                    printf("%f", entry->value.floatValue);
                }
            } else {
                printf("Error: Uninitialized variable 'a' at line %d\n",
                       n->token.lineNumber);
                return;
            }
            cursor += 2; // Skip ~a
        } else if (*cursor == '~') {
            if (!printTail || printTail->childCount < 3 || strcmp(printTail->symbol, "PrintTail") != 0) {
                printf("Error: Missing identifier for format specifier ~ at line %d\n",
                       n->token.lineNumber);
                return;
            }
            node* idNode = printTail->children[1];
            if (!idNode || strcmp(idNode->symbol, "id") != 0 || !idNode->isTokenPresent) {
                printf("Error: Invalid identifier in PrintTail at line %d\n",
                       printTail->token.lineNumber);
                return;
            }
            int index = findSymbol(table, idNode->token.data.identifier.lexeme);
            if (index == -1) {
                printf("Semantic Error: Undeclared variable '%s' at line %d\n",
                       idNode->token.data.identifier.lexeme,
                       n->token.lineNumber);
                return;
            }
            SymbolEntry* entry = &table->entries[index];
            if (entry->hasValue) {
                if (entry->type == TYPE_INT) {
                    printf("%d", entry->value.intValue);
                } else {
                    printf("%f", entry->value.floatValue);
                }
            } else {
                printf("Error: Uninitialized variable '%s' at line %d\n",
                       idNode->token.data.identifier.lexeme,
                       n->token.lineNumber);
                return;
            }
            printTail = printTail->children[2]; // Move to next PrintTail
            cursor++; // Skip ~
        } else {
            putchar(*cursor);
            cursor++;
        }
    }
    if (printTail && printTail->childCount > 0) {
        printf("Error: Extra PrintTail arguments at line %d\n",
               printTail->token.lineNumber);
    }
    printf("\n"); // Add newline after printing the formatted string
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
    char buffer[256]; // Buffer for getTokenString

    if (currentTokenPtr) {
        currentToken = getNextToken(&currentTokenPtr);
        printf("Debug: First token: %s (type %d, line %d)\n",
               getTokenString(currentToken, buffer, sizeof(buffer)),
               currentToken.type, currentToken.lineNumber);
    } else {
        printf("Syntax Error: Empty input\n");
        freeNode(root);
        return 0;
    }

    while (!isEmpty(&s)) {
        printStack(&s);
        StackItem item = pop(&s);
        char* topSymbol = item.symbol;
        node* currentNode = item.treeNode;

        if (!topSymbol) {
            printf("Error: Popped NULL symbol from stack\n");
            freeNode(root);
            freeTokenList(head);
            return 0;
        }

        // Validate topSymbol
        int isValidSymbol = 0;
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(topSymbol, terminals[i]) == 0) {
                isValidSymbol = 1;
                break;
            }
        }
        for (int i = 0; !isValidSymbol && nonterminals[i] != NULL; i++) {
            if (strcmp(topSymbol, nonterminals[i]) == 0) {
                isValidSymbol = 1;
                break;
            }
        }
        if (!isValidSymbol) {
            printf("Error: Invalid symbol '%s' popped from stack\n", topSymbol);
            free(topSymbol);
            freeNode(root);
            freeTokenList(head);
            return 0;
        }

        printf("Debug: Processing symbol: %s (isTerminal: %d)\n",
               topSymbol, isNonTerminal(topSymbol) ? 0 : 1);

        if (terminalMatches(topSymbol, currentToken)) {
            if (strcmp(topSymbol, "$") == 0 && (currentToken.lineNumber == -1 || currentToken.data.separator.symbol == '$')) {
                printf("Debug: Reached end of input, parsing successful\n");
                free(topSymbol);
                return 1;
            }
            if (currentNode) {
                currentNode->isTokenPresent = 1;
                currentNode->token = currentToken;
                if (strcmp(topSymbol, "id") == 0) {
                    int index = findSymbol(table, currentToken.data.identifier.lexeme);
                    if (index != -1) {
                        currentNode->symbolType = table->entries[index].type; // Set type from symbol table
                    }
                    int isDeclaration = 0;
                    node* parent = currentNode->parent;
                    if (parent && strcmp(parent->symbol, "Statement") == 0 &&
                        parent->childCount > 0 && strcmp(parent->children[0]->symbol, "Type") == 0) {
                        isDeclaration = 1;
                    } else if (parent && strcmp(parent->symbol, "DeclTail") == 0 &&
                               parent->childCount > 0 && strcmp(parent->children[0]->symbol, ",") == 0) {
                        isDeclaration = 1;
                    }
                    if (isDeclaration) {
                        if (findSymbol(table, currentToken.data.identifier.lexeme) == -1) {
                            if (!addSymbol(table, currentToken.data.identifier.lexeme, currentType, currentToken.lineNumber)) {
                                free(topSymbol);
                                freeNode(root);
                                freeTokenList(head);
                                return 0;
                            }
                        } else {
                            printf("Semantic Error at line %d: Variable '%s' already declared\n",
                                   currentToken.lineNumber, currentToken.data.identifier.lexeme);
                            free(topSymbol);
                            freeNode(root);
                            freeTokenList(head);
                            return 0;
                        }
                    } else if (strcmp(currentNode->parent->symbol, "PrintfStmt") != 0 &&
                               findSymbol(table, currentToken.data.identifier.lexeme) == -1) {
                        printf("Semantic Error at line %d: Undeclared variable '%s'\n",
                               currentToken.lineNumber, currentToken.data.identifier.lexeme);
                        free(topSymbol);
                        freeNode(root);
                        freeTokenList(head);
                        return 0;
                    }
                } else if (strcmp(topSymbol, "literal") == 0) {
                    // Check if literal is integer or float
                    char* endptr;
                    strtod(currentToken.data.literal.value, &endptr);
                    if (*endptr == '\0') {
                        currentNode->symbolType = TYPE_INT; // Integer literal
                    } else if (*endptr == '.') {
                        currentNode->symbolType = TYPE_FLOAT; // Float literal
                    }
                } else if (strcmp(topSymbol, "int") == 0) {
                    currentType = TYPE_INT;
                } else if (strcmp(topSymbol, "float") == 0) {
                    currentType = TYPE_FLOAT;
                }
            }
            currentToken = getNextToken(&currentTokenPtr);
            printf("Debug: Advanced to next token: %s (type %d, line %d)\n",
                   getTokenString(currentToken, buffer, sizeof(buffer)),
                   currentToken.type, currentToken.lineNumber);
            free(topSymbol);
            continue;
        }

        if (isNonTerminal(topSymbol)) {
            NonTerminal ntIndex = getNonTerminalEnum(topSymbol);
            if (ntIndex == -1) {
                printf("Error: Invalid non-terminal %s\n", topSymbol);
                free(topSymbol);
                freeNode(root);
                freeTokenList(head);
                return 0;
            }
            Terminal tIndex = getTerminalEnum(NULL, currentToken);
            if (tIndex == -1) {
                printf("Syntax Error at line %d: Invalid token %s\n",
                       currentToken.lineNumber, getTokenString(currentToken, buffer, sizeof(buffer)));
                free(topSymbol);
                freeNode(root);
                freeTokenList(head);
                return 0;
            }

            int production = parsingTable[ntIndex][tIndex];
            printf("Debug: Looking up production for non-terminal %s, token %s, got production %d\n",
                   topSymbol, getTokenString(currentToken, buffer, sizeof(buffer)), production);
            if (production == -1) {
                printf("Syntax Error at line %d: No production for %s on token %s. Expected one of: ",
                       currentToken.lineNumber, topSymbol, getTokenString(currentToken, buffer, sizeof(buffer)));
                for (int i = 0; i < num_terminal; i++) {
                    if (parsingTable[ntIndex][i] != -1) {
                        printf("%s ", terminals[i]);
                    }
                }
                printf("\n");
                free(topSymbol);
                freeNode(root);
                freeTokenList(head);
                return 0;
            }

            char (*rhs)[maxSymbolLength] = getFlatIndex(grammar, production);
            if (!rhs) {
                printf("Syntax Error: Invalid production index %d for non-terminal %s\n", production, topSymbol);
                free(topSymbol);
                freeNode(root);
                freeTokenList(head);
                return 0;
            }

            if (strcmp(rhs[0], "epsilon") == 0) {
                printf("Debug: Applied epsilon production for %s\n", topSymbol);
                free(topSymbol);
                continue;
            }

            int symbolCount = 0;
            while (rhs[symbolCount][0] != '\0' && symbolCount < maxSymbolPerProduction) {
                symbolCount++;
            }
            node* tempChildren[10];
            int tempChildCount = 0;
            for (int i = 0; i < symbolCount; i++) {
                node* child = create(rhs[i]);
                child->parent = currentNode;
                child->token.lineNumber = currentToken.lineNumber; // Propagate line number
                tempChildren[tempChildCount++] = child;
            }
            for (int i = 0; i < tempChildCount; i++) {
                currentNode->children[i] = tempChildren[i];
                currentNode->childCount++;
            }
            // Debug output for PrintfStmt node creation
            if (strcmp(topSymbol, "PrintfStmt") == 0) {
                printf("Debug: Creating PrintfStmt node with %d children\n", currentNode->childCount);
                for (int i = 0; i < currentNode->childCount; i++) {
                    printf("  Child %d: %s\n", i, currentNode->children[i]->symbol);
                }
            }
            for (int i = symbolCount - 1; i >= 0; i--) {
                push(&s, rhs[i], tempChildren[i]);
            }
            free(topSymbol);
        } else {
            printf("Syntax Error at line %d: Expected terminal %s, got %s\n",
                   currentToken.lineNumber, topSymbol, getTokenString(currentToken, buffer, sizeof(buffer)));
            free(topSymbol);
            freeNode(root);
            freeTokenList(head);
            return 0;
        }
    }

    if (currentTokenPtr != NULL || currentToken.lineNumber != -1) {
        printf("Syntax Error at line %d: Unexpected tokens remaining\n",
               currentToken.lineNumber);
        freeNode(root);
        freeTokenList(head);
        return 0;
    }

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

char* getTokenString(Token t, char* buffer, size_t bufferSize) {
    if (t.lineNumber == -1) {
        snprintf(buffer, bufferSize, "EOF");
        return buffer;
    }
    switch (t.type) {
        case TOKEN_KEYWORD:
            snprintf(buffer, bufferSize, "%s", t.data.keyword.lexeme);
            return buffer;
        case TOKEN_OPERATOR:
            snprintf(buffer, bufferSize, "%s", t.data.operator.op);
            return buffer;
        case TOKEN_IDENTIFIER:
            snprintf(buffer, bufferSize, "%s", t.data.identifier.lexeme);
            return buffer;
        case TOKEN_LITERAL:
            snprintf(buffer, bufferSize, "%s", t.data.literal.value);
            return buffer;
        case TOKEN_STRING:
            snprintf(buffer, bufferSize, "%s", t.data.string.value);
            return buffer;
        case TOKEN_SEPARATOR:
            snprintf(buffer, bufferSize, "%c", t.data.separator.symbol);
            return buffer;
        default:
            snprintf(buffer, bufferSize, "UNKNOWN");
            return buffer;
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
    char* cleaned = NULL;
    Terminal result = -1;
    if (token.type == TOKEN_KEYWORD) {
        cleaned = cleanTokenString(token.data.keyword.lexeme);
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(cleaned, terminals[i]) == 0) {
                result = (Terminal)i;
                break;
            }
        }
    } else if (token.type == TOKEN_OPERATOR) {
        cleaned = cleanTokenString(token.data.operator.op);
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(cleaned, terminals[i]) == 0) {
                result = (Terminal)i;
                break;
            }
        }
    } else if (token.type == TOKEN_SEPARATOR) {
        char temp[2] = {token.data.separator.symbol, '\0'};
        for (int i = 0; terminals[i] != NULL; i++) {
            if (strcmp(temp, terminals[i]) == 0) {
                result = (Terminal)i;
                break;
            }
        }
    }
    free(cleaned); // Free the cleaned string
    char buffer[256];
    if (result == -1) {
        printf("Debug: Failed to map token %s to terminal\n", getTokenString(token, buffer, sizeof(buffer)));
    }
    return result;
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
      char buffer[256];
      if (root->isTokenPresent) {
        printf(" [token: %s, line: %d]", getTokenString(root->token, buffer, sizeof(buffer)), root->token.lineNumber);
      }
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
    if (!head) {
        printf("Lexing failed.\n");
        return 1;
    }

    printf("\n----- Full Token List -----\n");
    tokenNode* temp = head;
    int tokenCount = 0;
    char buffer[256];
    while (temp != NULL) {
        Token t = temp->token;
        printf("Token %d: %s (type %d, line %d)\n", tokenCount++, getTokenString(t, buffer, sizeof(buffer)), t.type, t.lineNumber);
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
        printf("\n----- Executing Program -----\n");
        double returnValue = executeProgram(root, &table);
        printf("\n----- Execution Complete -----\n");
        printf("Return value: %f\n", returnValue);
        printSymbolTable(&table);
    } else {
        printf("\nParsing Failed.\n");
    }

    for (int i = 0; i < table.count; i++) {
        free(table.entries[i].name);
    }
    freeTokenList(head);
    freeNode(root);
    return 0;
}
