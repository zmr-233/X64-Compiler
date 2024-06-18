#include"x64_comp.h"
//main中定义
char *CurrentInput;

int main(int Argc, char **Argv) {
    if (Argc != 2) {
        error("%s: invalid number of arguments", Argv[0]);
    }
    CurrentInput = Argv[1];

    // 词法分析-解析Argv[1]，生成终结符流
    Token *Tok = tokenize();

    // 语法分析-解析终结符流
    Node *node = parse(Tok);
    
    //代码生成
    codegen(node);

    return 0;
}
