#include"x64_comp.h"


// 输出错误信息
// static文件内可以访问的函数
// Fmt为传入的字符串， ... 为可变参数，表示Fmt后面所有的参数
void error(char *Fmt, ...) {
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
void verrorAt(char *Loc, char *Fmt, va_list VA) {
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
void errorAt(char *Loc, char *Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Loc, Fmt, VA);
    exit(1);
}

// Tok解析出错，并退出程序
void errorTok(Token *Tok, char *Fmt, ...) {
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Tok->Loc, Fmt, VA);
    exit(1);
}


//------------------------判断Token与str关系-------------

// 判断Tok的值是否等于指定值，没有用char，是为了后续拓展
bool equal(Token *Tok, char *Str) {
    // 比较字符串LHS（左部），RHS（右部）的前N位，S2的长度应大于等于N.
    // 比较按照字典序，LHS<RHS回负值，LHS=RHS返回0，LHS>RHS返回正值
    // 同时确保，此处的Op位数=N
    return memcmp(Tok->Loc, Str, Tok->Len) == 0 && Str[Tok->Len] == '\0';
}

// 跳过指定的Str
Token *skip(Token *Tok, char *Str) {
    if (!equal(Tok, Str))
        errorTok(Tok, "expect '%s'", Str);
    return Tok->Next;
}
