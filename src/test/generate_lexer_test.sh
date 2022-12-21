#!/bin/bash

# Exmple:
# From src folder, execute
# ./test/generate_lexer_test.sh input.graphql {test_name}

test_dir="test/lexer/$2"

mkdir $test_dir
cp $1 $test_dir/query.graphql
echo "{}" | ./gq $1 &>$test_dir/output.txt
