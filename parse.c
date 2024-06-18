#include"x64_comp.h"

//
// 生成AST（抽象语法树），语法解析
//

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

//语法分析对外函数

Node* parse(Token* Tok){
    Node* node = expr(&Tok,Tok);

    //检查是否是文件末尾
    if (Tok->Kind != TK_EOF)
        errorTok(Tok, "extra token");

    return node;
}