#!/bin/bash

# Exmple:
# From src folder, execute
# ./test/main/generate_grammar_test.sh input.graphql {test_name}

test_dir="test/grammar/$2"

mkdir $test_dir
cp $1 $test_dir/query.graphql
echo "{}" | ./gq $1 &>$test_dir/output.txt
