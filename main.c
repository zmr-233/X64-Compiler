#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// 终结符分析，词法分析
//

// 为每个终结符都设置种类来表示
typedef enum {
    TK_PUNCT, // 操作符如： + -
    TK_NUM,   // 数字
    TK_EOF,   // 文件终止符，即文件的最后
} TokenKind;

// 终结符结构体
typedef struct Token Token;
struct Token {
    TokenKind Kind; // 种类
    Token *Next;    // 指向下一终结符
    int Val;        // 值
    char *Loc;      // 在解析的字符串内的位置
    int Len;        // 长度
};

// 输入的字符串
static char *CurrentInput;

// 输出错误信息
// static文件内可以访问的函数
// Fmt为传入的字符串， ... 为可变参数，表示Fmt后面所有的参数
static void error(char *Fmt, ...) {
    // 定义一个va_list变量
    va_list VA;
    // VA获取Fmt后面的所有参数
    va_start(VA, Fmt);
    // vfprintf可以输出va_list类型的参数
    vfprintf(stderr, Fmt, VA);
    // 在结尾加上一个换行符
    fprintf(stderr, "\n");
    // 清除VA
    va_end(VA);
    // 终止程序
    exit(1);
}

// 输出错误出现的位置
static void verrorAt(char *Loc, char *Fmt, va_list VA) {
    // 先输出源信息
    fprintf(stderr, "%s\n", CurrentInput);

    // 输出出错信息
    // 计算出错的位置，Loc是出错位置的指针，CurrentInput是当前输入的首地址
    int Pos = Loc - CurrentInput;
    // 将字符串补齐为Pos位，因为是空字符串，所以填充Pos个空格。
    fprintf(stderr, "%*s", Pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, Fmt, VA);
    fprintf(stderr, "\n");
    va_end(VA);
}

// 字符解析出错，并退出程序
static void errorAt(char *Loc, char *Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Loc, Fmt, VA);
    exit(1);
}

// Tok解析出错，并退出程序
static void errorTok(Token *Tok, char *Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Tok->Loc, Fmt, VA);
    exit(1);
}

// 判断Tok的值是否等于指定值，没有用char，是为了后续拓展
static bool equal(Token *Tok, char *Str) {
    // 比较字符串LHS（左部），RHS（右部）的前N位，S2的长度应大于等于N.
    // 比较按照字典序，LHS<RHS回负值，LHS=RHS返回0，LHS>RHS返回正值
    // 同时确保，此处的Op位数=N
    return memcmp(Tok->Loc, Str, Tok->Len) == 0 && Str[Tok->Len] == '\0';
}

// 跳过指定的Str
static Token *skip(Token *Tok, char *Str) {
    if (!equal(Tok, Str))
        errorTok(Tok, "expect '%s'", Str);
    return Tok->Next;
}

// 返回TK_NUM的值
static int getNumber(Token *Tok) {
    if (Tok->Kind != TK_NUM)
        errorTok(Tok, "expect a number");
    return Tok->Val;
}

// 生成新的Token
static Token *newToken(TokenKind Kind, char *Start, char *End) {
    // 分配1个Token的内存空间
    Token *Tok = calloc(1, sizeof(Token));
    Tok->Kind = Kind;
    Tok->Loc = Start;
    Tok->Len = End - Start;
    return Tok;
}


// 判断Str是否以SubStr开头
static bool startsWith(char *Str, char *SubStr) {
  // 比较LHS和RHS的N个字符是否相等
  return strncmp(Str, SubStr, strlen(SubStr)) == 0;
}

// 读取操作符
static int readPunct(char *Ptr) {
  // 判断2字节的操作符
  if (startsWith(Ptr, "==") || startsWith(Ptr, "!=") || startsWith(Ptr, "<=") ||
      startsWith(Ptr, ">="))
    return 2;

  // 判断1字节的操作符
  return ispunct(*Ptr) ? 1 : 0;
}

// 终结符解析
static Token *tokenize() {
    char *P = CurrentInput;
    Token Head = {};
    Token *Cur = &Head;

    while (*P) {
        // 跳过所有空白符如：空格、回车
        if (isspace(*P)) {
            ++P;
            continue;
        }

        // 解析数字
        if (isdigit(*P)) {
            // 初始化，类似于C++的构造函数
            // 我们不使用Head来存储信息，仅用来表示链表入口，这样每次都是存储在Cur->Next
            // 否则下述操作将使第一个Token的地址不在Head中。
            Cur->Next = newToken(TK_NUM, P, P);
            // 指针前进
            Cur = Cur->Next;
            const char *OldPtr = P;
            Cur->Val = strtoul(P, &P, 10);
            Cur->Len = P - OldPtr;
            continue;
        }

        // 解析操作符
        int PunctLen = readPunct(P);
        if (PunctLen) {
            Cur->Next = newToken(TK_PUNCT, P, P + PunctLen);
            Cur = Cur->Next;
            // 指针前进Punct的长度位
            P += PunctLen;
            continue;
        }

        // 处理无法识别的字符
        errorAt(P, "invalid token");
    }

    // 解析结束，增加一个EOF，表示终止符。
    Cur->Next = newToken(TK_EOF, P, P);
    // Head无内容，所以直接返回Next
    return Head.Next;
}

//
// 生成AST（抽象语法树），语法解析
//

// AST的节点种类
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NEG, // 负号-
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_GT,  // >
  ND_GE,  // >=
  ND_NUM, // 整形
} NodeKind;

// AST中二叉树节点
typedef struct Node Node;
struct Node {
    NodeKind Kind; // 节点种类
    Node *LHS;     // 左部，left-hand side
    Node *RHS;     // 右部，right-hand side
    int Val;       // 存储ND_NUM种类的值
};

// 新建一个节点
static Node *newNode(NodeKind Kind) {
    Node *Nd = calloc(1, sizeof(Node));
    Nd->Kind = Kind;
    return Nd;
}

// 新建一个单叉树
static Node *newUnary(NodeKind Kind, Node *Expr) {
    Node *Nd = newNode(Kind);
    Nd->LHS = Expr;
    return Nd;
}

// 新建一个二叉树节点
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS) {
    Node *Nd = newNode(Kind);
    Nd->LHS = LHS;
    Nd->RHS = RHS;
    return Nd;
}

// 新建一个数字节点
static Node *newNum(int Val) {
    Node *Nd = newNode(ND_NUM);
    Nd->Val = Val;
    return Nd;
}


/**
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | "(" expr ")"
 */
static Node *expr(Token **Rest, Token *Tok);
static Node *equality(Token **Rest,Token *Tok);
static Node *realtional(Token **Rest,Token *Tok);
static Node *add(Token **Rest,Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);

static Node *expr(Token **Rest, Token *Tok){
    return equality(Rest,Tok);
}

static Node *equality(Token **Rest,Token *Tok){
    Node* Nd = realtional(&Tok,Tok);

    while(true){
        if(equal(Tok,"==")){
            Nd = newBinary(ND_EQ, Nd, realtional(&Tok,Tok->Next));
            continue;
        }
        if(equal(Tok,"!=")){
            Nd = newBinary(ND_NE, Nd, realtional(&Tok,Tok->Next));
            continue;
        }
        *Rest = Tok;
        return Nd;
    }
}

static Node *realtional(Token **Rest,Token *Tok){
    Node* Nd = add(&Tok,Tok);

    while(true){
        if(equal(Tok,"<")){
            Nd = newBinary(ND_LT, Nd, add(&Tok,Tok->Next));
            continue;
        }
        if(equal(Tok,"<=")){
            Nd = newBinary(ND_LE, Nd, add(&Tok,Tok->Next));
            continue;
        }
        if(equal(Tok,">")){
            Nd = newBinary(ND_GT, Nd, add(&Tok,Tok->Next));
            continue;
        }
        if(equal(Tok,">=")){
            Nd = newBinary(ND_GE, Nd, add(&Tok,Tok->Next));
            continue;
        }
        *Rest = Tok;
        return Nd;
    }
}
// 解析加减
// add = mul ("+" mul | "-" mul)*
static Node *add(Token **Rest, Token *Tok) {
    // mul
    Node *Nd = mul(&Tok, Tok);

    // ("+" mul | "-" mul)*
    while (true) {
        // "+" mul
        if (equal(Tok, "+")) {
            Nd = newBinary(ND_ADD, Nd, mul(&Tok, Tok->Next));
            continue;
        }

        // "-" mul
        if (equal(Tok, "-")) {
            Nd = newBinary(ND_SUB, Nd, mul(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

// 解析乘除
// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **Rest, Token *Tok) {
    // unary
    Node *Nd = unary(&Tok, Tok);

    // ("*" unary | "/" unary)*
    while (true) {
        // "*" unary
        if (equal(Tok, "*")) {
            Nd = newBinary(ND_MUL, Nd, unary(&Tok, Tok->Next));
            continue;
        }

        // "/" unary
        if (equal(Tok, "/")) {
            Nd = newBinary(ND_DIV, Nd, unary(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

// 解析一元运算
// unary = ("+" | "-") unary | primary
static Node *unary(Token **Rest, Token *Tok) {
    // "+" unary
    if (equal(Tok, "+"))
        return unary(Rest, Tok->Next);

    // "-" unary
    if (equal(Tok, "-"))
        return newUnary(ND_NEG, unary(Rest, Tok->Next));

    // primary
    return primary(Rest, Tok);
}

// 解析括号、数字
// primary = "(" expr ")" | num
static Node *primary(Token **Rest, Token *Tok) {
    // "(" expr ")"
    if (equal(Tok, "(")) {
        Node *Nd = expr(&Tok, Tok->Next);
        *Rest = skip(Tok, ")");
        return Nd;
    }

    // num
    if (Tok->Kind == TK_NUM) {
        Node *Nd = newNum(Tok->Val);
        *Rest = Tok->Next;
        return Nd;
    }

    errorTok(Tok, "expected an expression");
    return NULL;
}

//
// 语义分析与代码生成
//

// 记录栈深度
static int Depth;

// 压栈，将结果临时压入栈中备用
// sp为栈指针，栈反向向下增长，64位下，8个字节为一个单位，所以sp-8
// 当前栈指针的地址就是sp，将a0的值压入栈
// 不使用寄存器存储的原因是因为需要存储的值的数量是变化的。
static void push(char *Reg) {
    printf("    pushq %%%s\n", Reg);
    Depth++;
}

// 弹栈，将sp指向的地址的值，弹出到a1
static void pop(char *Reg) {
    printf("    popq %%%s\n", Reg);
    Depth--;
}

// 生成表达式
static void genExpr(Node *Nd) {
    // 生成各个根节点
    switch (Nd->Kind) {
    // 加载数字到rax
    case ND_NUM:
        printf("    movq $%d,%%rax\n", Nd->Val);
        return;
    // 对寄存器取反
    case ND_NEG:
        genExpr(Nd->LHS);
        // neg a0, a0是sub a0, x0, a0的别名, 即a0=0-a0
        printf("    negq %%rax\n");
        return;
    default:
        break;
    }

    // 递归到最右节点
    genExpr(Nd->RHS);
    // 将结果压入栈
    push("rax");
    // 递归到左节点
    genExpr(Nd->LHS);
    // 将结果弹栈到a1 -->这里默认返回在
    pop("rdi");

    // 生成各个二叉树节点
    switch (Nd->Kind) {
    case ND_ADD: // + a0=a0+a1
        printf("    addq %%rdi, %%rax\n");
        return;
    case ND_SUB: // - a0=a0-a1
        printf("    subq %%rdi, %%rax\n");
        return;
    case ND_MUL: // * a0=a0*a1
        printf("    imulq %%rdi, %%rax\n");
        return;
    case ND_DIV: // / a0=a0/a1
        printf("    cqto\n");
        printf("    idivq %%rdi\n");
        return;
    // 关系运算
    case ND_LT:
        printf("    cmpq %%rdi, %%rax\n");
        printf("    setl %%al\n");
        printf("    movzbq %%al, %%rax\n");
        return;
    case ND_LE:
        printf("    cmpq %%rdi, %%rax\n");
        printf("    setle %%al\n");
        printf("    movzbq %%al, %%rax\n");
        return;
    case ND_GT:
        printf("    cmpq %%rdi, %%rax\n");
        printf("    setg %%al\n");
        printf("    movzbq %%al, %%rax\n");
        return;
    case ND_GE:
        printf("    cmpq %%rdi, %%rax\n");
        printf("    setge %%al\n");
        printf("    movzbq %%al, %%rax\n");
        return;
    // == !=
    case ND_EQ:
        printf("    cmpq %%rdi, %%rax\n");
        printf("    sete %%al\n");
        printf("    movzbq %%al, %%rax\n");
        return;
    case ND_NE:
        printf("    cmpq %%rdi, %%rax\n");
        printf("    setne %%al\n");
        printf("    movzbq %%al, %%rax\n");
        return;
    default:
        error("invalid expression");
        break;
    }
}

int main(int Argc, char **Argv) {
    // 判断传入程序的参数是否为2个，Argv[0]为程序名称，Argv[1]为传入的第一个参数
    if (Argc != 2) {
        // 异常处理，提示参数数量不对。
        // fprintf，格式化文件输出，往文件内写入字符串
        // stderr，异常文件（Linux一切皆文件），用于往屏幕显示异常信息
        // %s，字符串
        error("%s: invalid number of arguments", Argv[0]);
    }

    // 解析Argv[1]，生成终结符流
    CurrentInput = Argv[1];
    Token *Tok = tokenize();

    // 解析终结符流
    Node *node = expr(&Tok, Tok);

    if (Tok->Kind != TK_EOF)
        errorTok(Tok, "extra token");

    // 声明一个全局main段，同时也是程序入口段
    printf(".globl main\n");
    // main段标签
    printf("main:\n");

    // 遍历AST树生成汇编
    genExpr(node);

    // ret为jalr x0, x1, 0别名指令，用于返回子程序
    // 返回的为rax的值
    printf("  ret\n");

    // 如果栈未清空则报错
    assert(Depth == 0);

    return 0;
}
