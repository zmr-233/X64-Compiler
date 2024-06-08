#!/bin/bash
assert(){
    target="$1"
    argc="$2"
    
    # ./make all

    ./main "$argc" > tmp.s || exit

    cc -o tmp tmp.s
    ./tmp
    result="$?"

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

echo END