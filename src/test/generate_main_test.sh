#!/bin/bash

# Exmple:
# From src folder, execute
# ./test/main/generate_main_test.sh input.graphql example.json {test_name}

test_dir="test/main/$3"

mkdir $test_dir
cp $1 $test_dir/query.graphql
cp $2 $test_dir/input.json
./gq $1 -j $2 &>$test_dir/output.txt
