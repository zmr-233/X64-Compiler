#include"x64_comp.h"

//-------------------------------------------------------------------

// 返回TK_NUM的值
// static int getNumber(Token *Tok) {
//     if (Tok->Kind != TK_NUM)
//         errorTok(Tok, "expect a number");
//     return Tok->Val;
// }

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

// 终结符解析--词法分析
Token *tokenize() {
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
