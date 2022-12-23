#!/bin/bash

for f in $(ls test/lexer); do
	echo "{}" | ./gq "test/lexer/$f/query.graphql" &>"test/lexer/$f/output.txt"
done

for f in $(ls test/parser); do
	echo "{}" | ./gq "test/parser/$f/query.graphql" &>"test/parser/$f/output.txt"
done

for f in $(ls test/main); do
	./gq "test/main/$f/query.graphql" -j "test/main/$f/input.json" &>"test/main/$f/output.txt"
done
