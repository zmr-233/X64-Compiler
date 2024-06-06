#include<stdbool.h>
#include<stdio.h>
#include<stdarg.h>
#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>

typedef enum {
    TK_PUNCT,
    // TK_MUL,TK_DIV,TK_ADD,TK_SUB,
    TK_PAREN,
    // TK_LPAREN,TK_RPAREN,// ( )
    // TK_LBRACKET,TK_RBRACKET,//[]
    // TK_LBRACE,TK_RBRACE,//{}
    TK_NUM,
    TK_EOF
}TokenKind;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
}NodeKind;

typedef struct Token Token;
struct Token{
    NodeKind kind;
    Token* next;
    long val;
    char* start;
    int len;
};

typedef struct Node Node;
struct Node{
    NodeKind kind;
    Node* ls;
    Node* rs;
    long val;
};

Node* newNode(NodeKind kind,Node* ls,Node* rs){
    Node* tmp = calloc(1,sizeof(Node));
    tmp->kind = kind;
    tmp->ls = ls, tmp->rs=rs;
    return tmp;
}   

Node* newNumNode(long val){
    Node* tmp = calloc(1,sizeof(Node));
    tmp->kind = ND_NUM;
    tmp->val = val;
    return tmp;
}

void static error(char* fmt,...){
    va_list args;    
    va_start(args,fmt);
    vfprintf(stderr,fmt,args);
    fprintf(stderr,"\n");
    va_end(args);
    exit(1);
}

//Global argv
static char *CurrentInput;

void verrorAt(char* loc,char* FMT,va_list VA){
    fprintf(stderr,"%s\n",CurrentInput);
    int pos = loc-CurrentInput;
    fprintf(stderr,"%*s",pos,"");
    fprintf(stderr,"^ ");
    fprintf(stderr,FMT,VA);
    fprintf(stderr,"\n");
}

void errorAt(char* loc,char* FMT,...){
    va_list VA;
    va_start(VA,FMT);
    verrorAt(loc,FMT,VA);
    va_end(VA);
    exit(1);
}

void errorTok(Token* tok,char* FMT,...){
    va_list VA;
    va_start(VA,FMT);
    verrorAt(tok->start,FMT,VA);
    va_end(VA);
    exit(1);
}

static bool equal(Token* tok,char* str){
    return memcmp(tok->start,str,tok->len)==0 && str[tok->len]=='\0';
}

static Token* skip(Token* tok,char* str){
    if (!equal(tok, str))
        // error("expect '%s'", str);
        errorTok(tok,"Cannot skip Token");
    return tok->next;
}

static Token* getNewToken(int kind,long val,char* start,char* end){
    Token* tok = calloc(1,sizeof(Token));
    tok->kind = kind;
    tok->val = val;
    tok->start = start;
    tok->len = end-start;
    return tok;
}

static Token* tokenHandle(char* p){
    Token head={};
    Token* cur = &head;
    while(*p){
        if(isspace(*p)){
            ++p;
            continue;
        }
        if(isdigit(*p)){
            cur->next = getNewToken(TK_NUM,0,p,p);
            cur = cur->next;
            const char* old_p = p;
            cur->val = strtol(p,&p,10);
            cur->len = p-old_p;
            continue;
        }
        if(*p=='+' || *p=='-' || *p=='*' || *p=='/'){
            cur->next = getNewToken(TK_PUNCT,*p,p,p+1);
            cur = cur->next;
            ++p;
            continue;
        }
        if(*p == '(' || *p == ')'){
            // TK_LPAREN,TK_RPAREN
            cur->next = getNewToken(TK_PAREN,*p,p,p+1);
            cur = cur->next;
            ++p;
            continue;
        }
        // error("Unkown sign %c \n",*p);
        errorAt(p,"Unknown sign");
    }
    cur->next=getNewToken(TK_EOF,0,p,p);
    return head.next;
}

long tokenGetNum(Token* tok){
    // fprintf(stderr,"---\n");
    if(tok->kind != TK_NUM){
        // error("Except a number");
        errorTok(tok,"Except number");
    }
    return tok->val;
}

//------------------------------------------------------

static Token* gtok;

bool consume(char ch){
    if(*(gtok->start)==ch){ 
        gtok = gtok->next;
        return 1;
    }
    return 0;
}

long consumeNum(){
    if(gtok->kind==TK_NUM){
        long tmp = gtok->val;
        gtok = gtok->next;
        return tmp;
    }
    errorTok(gtok,"Except Number");
}

void except(char ch){
    if(*(gtok->start)==ch){
        gtok = gtok->next;
        return;
    }
    errorTok(gtok,"Except '%c' not found",ch);
}

Node* expr();

Node* primary(){
    if(consume('(')){
        Node* node = expr();
        except(')');
        return node;
    }
    return newNumNode(consumeNum());
}

Node* mul(){
    Node* node = primary();

    for(;;){
        if(consume('*')){
            node = newNode(ND_MUL,node,primary());
        }else if(consume('/')){
            node = newNode(ND_DIV,node,primary());
        }else{
            return node;
        }
    }
}

Node* expr(){
    Node* node = mul();

    for(;;){
        if(consume('+')){
            node = newNode(ND_ADD,node,mul());
        }else if(consume('-')){
            node = newNode(ND_SUB,node,mul());
        }else{
            return node;
        }
    }
}

void generate(Node* node){
    if(node->kind==ND_NUM){
        printf("    pushq $%ld\n",node->val);
        return;
    }
    generate(node->ls);
    generate(node->rs);
    printf("    popq %%rdi\n");
    printf("    popq %%rax\n");
    switch(node->kind){
    case ND_ADD:
        printf("    addq %%rdi, %%rax\n");
        break;
    case ND_SUB:
        printf("    subq %%rdi, %%rax\n");
        break;
    case ND_MUL:
        printf("    imulq %%rdi, %%rax\n");
        break;
    case ND_DIV:
        printf("    cqto\n");
        printf("    idivq %%rdi\n");
        break;
    }
    printf("    pushq %%rax\n");
}

int main(int argc,char* argv[]){
    if(argc!=2){
        error("Error args %s \n",argv[1]);
    }
    CurrentInput = argv[1];
    gtok = tokenHandle(CurrentInput);
    Node* root = expr();
    
    printf(".globl main\n");
    printf("main:\n");
    generate(root);
    printf("    popq %%rax\n");
    printf("    ret\n");
    return 0;
}