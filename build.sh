#!/bin/bash

compiler="clang"
includes="-Iinclude"
sources="source/lexer.c source/parser.c source/main.c"
unity_src="test/Unity/src/unity.c"
flags="-g -O0"
defs="-DDEBUG"
bin="bin/ra"

mkdir -p bin

if [ "$1" = "test" ]; then
	includes="-Iinclude -Itest/Unity/src"
	common="source/lexer.c source/parser.c test/Unity/src/unity.c"

	$compiler $flags $includes $common test/test_lexer.c -o bin/test_lexer || exit 1
	$compiler $flags $includes $common test/test_parser.c -o bin/test_parser || exit 1

	bin/test_lexer
	bin/test_parser
	exit 0
elif [ -n "$1" ]; then
	echo "Usage: $0 [test]"
	echo "  test    - Build and run tests"
	echo "  <empty> - Build the main executable"
	exit 1
fi

$compiler $flags $includes $defs $sources -o $bin
if [ $? -ne 0 ]; then
	echo "Build failed"
	exit 1
fi
