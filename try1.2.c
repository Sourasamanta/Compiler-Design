#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include <stdlib.h>

// Token Types
typedef enum {
  TOKEN_KEYWORD,
  TOKEN_LITERAL,
  TOKEN_OPERATOR,
  TOKEN_SEPARATOR
} TokenType;

// Token Structures
typedef struct {
  char lexeme[32];
} TokenKeyword;

typedef struct {
  int value;
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
  union {
    TokenKeyword keyword;
    TokenLiteral literal;
    TokenOperator operator;
    TokenSeparator separator;
  } data;
} Token;

/////////////////////////////////////////////////////////////////////////////
typedef struct tokenNode{
  Token token;
  struct tokenNode* next;
}tokenNode;


/////////////////////////////////////////////////////////////////////////////

tokenNode* append(Token T,tokenNode* head){
  tokenNode* tail;
  tail=head;
  tokenNode* newNode=(tokenNode*)malloc(sizeof(tokenNode));
  newNode->token=T;
  newNode->next=NULL;
  if(head==NULL){
    return newNode;
  }
  while (tail->next!=NULL) {
    tail=tail->next;
  }
  if(tail!=NULL){
    tail->next=newNode;
  }
  return head;
}

////////////////////////////////////////////////////////////////////////////
tokenNode* check(char *content){
  int i=0,j=0;  //Initialize i and j
  int val;
  int n;
  Token key;



  tokenNode* head=NULL; //head Initialize



//Loop till content ends
  while(content[i]!='\0'){

//check alphabet
    if(isalpha(content[i])){
      j=i; n=0;
      while (isalpha(content[i])) i++;
      //printf("Keyword Found: ");
      key.type=TOKEN_KEYWORD;
      for(int m=j;m<i;m++){
        //printf("%c",content[m]);
        key.data.keyword.lexeme[n++]=content[m];  //put it in another string
      }
      key.data.keyword.lexeme[n]='\0';
      head=append(key,head);
      //printf("\n");
    }

//check digit
    else if(isdigit(content[i])){
      j=i;
      key.data.literal.value=0;
      while (isdigit(content[i])) i++;
      //printf("Literal Found: ");
      key.type=TOKEN_LITERAL;
      for(int m=j;m<i;m++){
        val=content[m]-'0';
        key.data.literal.value = 10 * key.data.literal.value + val;
      }
      //printf("  %d\n", key.data.literal.value);
      head=append(key,head);
    }

//check space
    else if(isspace(content[i])){
      i++;
    }

//check Operator
    else if(content[i]=='+'){
      key.type=TOKEN_OPERATOR;
      if(content[i+1]=='+'){
        strcpy(key.data.operator.op, "++");
        i+=2;
      } else if(content[i+1]=='+'){
        strcpy(key.data.operator.op, "+=");
        i+=2;
      } else {
        strcpy(key.data.operator.op, "+");
        i++;
      }
      //printf("Operator Found: %s\n", key.data.operator.op);
      head=append(key,head);
}

    else if(content[i]=='-'){
      key.type=TOKEN_OPERATOR;
      if(content[i+1]=='-'){
        strcpy(key.data.operator.op, "--");
        i+=2;
      } else if(content[i+1]=='='){
        strcpy(key.data.operator.op, "-=");
        i+=2;
      } else {
        strcpy(key.data.operator.op, "-");
        i++;
      }
      //printf("Operator Found: %s\n", key.data.operator.op);
      head=append(key,head);
    }

    else if(content[i]=='='){
      key.type=TOKEN_OPERATOR;
      if(content[i+1]=='='){
        strcpy(key.data.operator.op, "==");
        i+=2;
      } else {
        strcpy(key.data.operator.op, "=");
        i++;
      }
      //printf("Operator Found: %s\n", key.data.operator.op);
      head=append(key,head);
    }

    else if(content[i]=='*'){
      key.type=TOKEN_OPERATOR;
      if(content[i+1]=='*'){
        strcpy(key.data.operator.op, "**");
        i+=2;
      } else if(content[i+1]=='='){
        strcpy(key.data.operator.op, "*=");
        i+=2;
      } else {
        strcpy(key.data.operator.op, "*");
        i++;
      }
      //printf("Operator Found: %s\n", key.data.operator.op);
      head=append(key,head);
    }

    else if(content[i]=='/'){
      key.type=TOKEN_OPERATOR;
      if(content[i+1]=='='){
        strcpy(key.data.operator.op, "/=");
        i+=2;
      } else {
        strcpy(key.data.operator.op, "/");
        i++;
      }
      //printf("Operator Found: %s\n", key.data.operator.op);
      head=append(key,head);
    }
    else if(content[i]=='&'){
      i++;
      j=i; n=1;
      key.data.keyword.lexeme[0]='&';
      while (isalpha(content[i])) i++;
      //printf("Keyword Found: ");
      key.type=TOKEN_KEYWORD;
      for(int m=j;m<i;m++){
        //printf("%c",content[m]);
        key.data.keyword.lexeme[n++]=content[m];  //put it in another string
      }
      key.data.keyword.lexeme[n]='\0';
      head=append(key,head);
      //printf("\n");
    }

    else if(content[i] == '"') {
  key.type = TOKEN_KEYWORD;
  int m = 0;
  i++; // skip the opening quote
  while(content[i] != '"' && content[i] != '\0') {
    key.data.keyword.lexeme[m++] = content[i++];
  }
  key.data.keyword.lexeme[m] = '\0';
  if(content[i] == '"') i++; // skip the closing quote
  //printf("String Literal Found: \"%s\"\n", key.data.keyword.lexeme);
  head = append(key, head);
}


//check Seperator
    else {
      switch(content[i]){
        case '(':
        case ')':
        case ';':
          key.type=TOKEN_SEPARATOR;
          key.data.separator.symbol=content[i];
          //printf("Separator Found: %c\n", key.data.separator.symbol);
          head=append(key,head);
          i++;
          break;

        default:
          //printf("Illegal character: %c\n", content[i]);
          i++;
          break;
      }
    }
  }
    return head;
}


//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

void main(){
  tokenNode* head;
  FILE* fptr;
  fptr=fopen("exit1.txt","r");
  char content[100];
  while(fgets(content,100,fptr)){
  head=check(content);

  //Link list check
  printf("\n----- Token List -----\n");
  while (head != NULL) {
    Token t = head->token;
    switch (t.type) {
      case TOKEN_KEYWORD:
        printf("Keyword: %s\n", t.data.keyword.lexeme);
        break;
      case TOKEN_LITERAL:
        printf("Literal: %d\n", t.data.literal.value);
        break;
      case TOKEN_OPERATOR:
        printf("Operator: %s\n", t.data.operator.op);
        break;
      case TOKEN_SEPARATOR:
        printf("Separator: %c\n", t.data.separator.symbol);
        break;
    }
    head = head->next;
  }

}
  fclose(fptr);

}
