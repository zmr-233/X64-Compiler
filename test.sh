#!/bin/bash
main="$1"
tmp="$2"
PLATFORM="$3"

assert(){
    target="$1"
    argc="$2"

    "$main" "$argc" > "$tmp".s || exit

    if [ "$PLATFORM" = "x64" ]; then
        cc "$tmp".s -o "$tmp"
        "$tmp"
        result="$?"
    else
        if [ ! $RISCV ] || [ -z $RISCV ]; then
            echo "ENV \$RISCV not exist, \n Add ~/.zshrc: export RISCV=~/riscv export PATH:~/riscv/bin"
            exit 1
        fi
        $RISCV/bin/riscv64-unknown-linux-gnu-gcc -static "$tmp".s -o "$tmp";
        $RISCV/bin/qemu-riscv64 -L $RISCV/sysroot "$tmp";
        result="$?"        
    fi

    if [ "$target" = "$result" ]; then
        echo "$argc => $result"
    else
        echo "$argc => except $target but get $result"
    fi
}

# assert 期待值 输入值
# [1] 返回指定数值
assert 0 0
assert 42 42

# [2] 支持+ -运算符
assert 34 '12-34+56'

# [3] 支持空格
assert 41 ' 12 + 34 - 5 '

# [5] 支持* / ()运算符
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 17 '1-8/(2*2)+3*6'

# [6] 支持一元运算的+ -
assert 10 '-10+20'
assert 10 '- -10'
assert 10 '- - +10'
assert 48 '------12*+++++----++++++++++4'

# [7] 支持条件运算符
assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'
assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'
assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'
assert 1 '5==2+3'
assert 0 '6==4+3'
assert 1 '0*9+5*2==4+4*(6/3)-2'
echo END