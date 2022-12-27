#!/bin/bash

# Colors
R="\e[1;31m"
G="\e[1;32m"
B="\e[1;34m"
W="\e[0m"

total=0
passed=0
echo -e "Running all tests...\n"

for f in $(ls test/lexer); do
	echo "{}" | ./gq "test/lexer/$f/query.graphql" &>/tmp/test.txt
	if diff -q /tmp/test.txt "test/lexer/$f/output.txt" &>/dev/null; then
		echo -e "${G}[OK]${W}   $f"
		passed=$((passed + 1))
	else
		echo -e "${R}[FAIL]${W} $f"
	fi
	total=$((total + 1))
done

for f in $(ls test/parser); do
	echo "{}" | ./gq "test/parser/$f/query.graphql" &>/tmp/test.txt
	if diff -q /tmp/test.txt "test/parser/$f/output.txt" &>/dev/null; then
		echo -e "${G}[OK]${W}   $f"
		passed=$((passed + 1))
	else
		echo -e "${R}[FAIL]${W} $f"
	fi
	total=$((total + 1))
done

for f in $(ls test/main); do
	./gq "test/main/$f/query.graphql" -j "test/main/$f/input.json" &>/tmp/test.txt
	if diff -q /tmp/test.txt "test/main/$f/output.txt" &>/dev/null; then
		echo -e "${G}[OK]${W}   $f"
		passed=$((passed + 1))
	else
		echo -e "${R}[FAIL]${W} $f"
	fi
	total=$((total + 1))
done

echo -e "\nPassed: ${B}$passed/$total${W}"
if [ $passed -eq $total ]; then
	echo -e "${G}All tests passed!${W}"
else
	echo -e "${R}Failed: $((total - passed))${W}"
fi
