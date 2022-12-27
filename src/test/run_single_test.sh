#!/bin/bash

if [ ! -f $1/input.json ]; then
	echo "{}" | ./gq $1/query.graphql
	exit 1
else
	./gq $1/query.graphql -j $1/input.json
fi
