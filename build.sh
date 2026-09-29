#!/bin/bash

compiler="clang"

includes="-Iinclude"
flags="-std=c17 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wformat=2 -Wundef -Wstrict-prototypes -g -O0"
defs="-DDEBUG"

sources="source/lexer.c source/parser.c source/executor.c"
unity_src="test/Unity/src/unity.c"
bin="bin/ra"

mkdir -p bin

if [ "$1" = "test" ]; then
	includes="-Iinclude -Itest/Unity/src"
	sources="${sources} ${unity_src}"

	$compiler $flags $includes $sources test/test_spec_lexer.c -o bin/test_spec_lexer || exit 1
	$compiler $flags $includes $sources test/test_spec_parser.c -o bin/test_spec_parser || exit 1

	bin/test_spec_lexer || exit 1
	bin/test_spec_parser || exit 1

	exit 0
elif [ "$1" = "alltests" ]; then
	includes="-Iinclude -Itest/Unity/src"
	sources="${sources} ${unity_src}"

	$compiler $flags $includes $sources test/test_lexer.c -o bin/test_lexer || exit 1
	$compiler $flags $includes $sources test/test_parser.c -o bin/test_parser || exit 1
	$compiler $flags $includes $sources test/test_spec_lexer.c -o bin/test_spec_lexer || exit 1
	$compiler $flags $includes $sources test/test_spec_parser.c -o bin/test_spec_parser || exit 1

	bin/test_lexer || exit 1
	bin/test_parser || exit 1
	bin/test_spec_lexer || exit 1
	bin/test_spec_parser || exit 1

	exit 0
elif [ -n "$1" ]; then
	echo "Usage: $0 [test|alltests]"
	echo "  test       - Build and run assignment specification tests"
	echo "  alltests   - Build and run all tests"
	echo "  <empty>    - Build the main executable"
	exit 1
fi

sources="$sources source/main.c"
$compiler $flags $includes $defs $sources -o $bin
if [ $? -ne 0 ]; then
	echo "Build failed"
	exit 1
fi
