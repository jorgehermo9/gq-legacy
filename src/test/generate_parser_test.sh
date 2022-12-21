#!/bin/bash

# Exmple:
# From src folder, execute
# ./test/generate_parser_test.sh input.graphql {test_name}

test_dir="test/parser/$2"

mkdir $test_dir
cp $1 $test_dir/query.graphql
echo "{}" | ./gq $1 &>$test_dir/output.txt
