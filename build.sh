#/bin/bash

includes="include"
sources="source/main.c source/lexer.c source/parser.c"

mkdir -p bin
clang $sources -I$includes -o bin/ra-engine
