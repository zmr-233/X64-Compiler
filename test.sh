#!/bin/bash
assert(){
    argc="$1"
    target="$2"
    
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

assert 0 0
assert 233 233
assert 42 42

assert '2+7-5' 4
assert ' 4 + 9 - 3 + 4 ' 14

# assert '2 + + 7 - 5' 0 #ERROR Except number
# assert '2 + * 7 - 5' 0 # ERROR Unknown sign


echo END