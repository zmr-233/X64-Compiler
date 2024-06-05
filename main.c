#include<stdbool.h>
#include<stdio.h>
#include<stdarg.h>
#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>

typedef enum {
    TK_PUNCT,
    TK_NUM,
    TK_EOF
}TokenKind;

typedef struct Token Token;
struct Token{
    TokenKind kind;
    Token* next;
    long val;
    char* start;
    int len;
};

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
        if(*p=='+' || *p=='-'){
            cur->next = getNewToken(TK_PUNCT,*p,p,p+1);
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

int main(int argc,char* argv[]){
    if(argc!=2){
        error("Error args %s \n",argv[1]);
    }
    CurrentInput = argv[1];
    Token* tok = tokenHandle(CurrentInput);
    ;
    
    printf(".globl main\n");
    printf("main:\n");
    printf("    movq $%ld, %%rax\n",tokenGetNum(tok));
    tok=tok->next;

    while(tok->kind != TK_EOF){
        if(equal(tok,"+")){
            tok = tok->next;
            printf("    addq $%ld, %%rax\n",tokenGetNum(tok));
            tok=tok->next;;
        }else{
            tok = skip(tok,"-");
            printf("    subq $%ld, %%rax\n",tokenGetNum(tok));
            tok=tok->next;
        }
    }
    printf("    ret\n");
    return 0;
}