#!/bin/bash

includes="include"
sources="source/main.c source/lexer.c source/parser.c"
unity_src="test/Unity/src/unity.c"

mkdir -p bin

case "$1" in
    debug)
        clang $sources -I$includes -DDEBUG -o bin/ra-engine
        ;;
    test)
        clang test/test_lexer.c source/lexer.c $unity_src -I$includes -Itest/Unity/src -o bin/test_lexer
        ./bin/test_lexer
        ;;
    *)
        clang $sources -I$includes -o bin/ra-engine
        ;;
esac
