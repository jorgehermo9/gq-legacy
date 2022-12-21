#!/bin/bash

for f in $(ls test/grammar); do
	echo "{}" | ./gq "test/grammar/$f/query.graphql" &>"test/grammar/$f/output.txt"
done

for f in $(ls test/main); do
	./gq "test/main/$f/query.graphql" -j "test/main/$f/input.json" &>"test/main/$f/output.txt"
done
