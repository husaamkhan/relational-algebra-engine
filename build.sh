#!/bin/bash

compiler="clang"
includes="-Iinclude"
sources=" source/lexer.c source/parser.c"
unity_src="test/Unity/src/unity.c"
flags=""
defs=""
bin="bin/ra-engine"
run=0

mkdir -p bin

if [ "$1" = "debug" ]; then
    flags="-g -O0"
    defs="-DDEBUG"
    sources="$sources source/main.c"
elif [ "$1" = "test" ]; then
    includes="-Iinclude -Itest/Unity/src"
    flags="-g -O0"
    bin="bin/test_lexer"
    sources="$sources test/Unity/src/unity.c test/test_lexer.c"
    run=1
elif [ -z "$1" ]; then
    sources="$sources source/main.c"
else
    echo "Usage: $0 [debug|test|<empty>]"
    echo "  debug   - Build with debug symbols and no optimization"
    echo "  test    - Build and run tests"
    echo "  <empty> - Default build"
    exit 1
fi

$compiler $flags $includes $defs $sources -o $bin
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

if [ "$run" -eq 1 ]; then
    $bin
fi
