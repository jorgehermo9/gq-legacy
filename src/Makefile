all: main

grammar: gq.l gq.y
	flex gq.l
	bison -o gq.tab.c gq.y -H

main: grammar main.cpp
	g++ -o gq main.cpp lex.yy.c gq.tab.c -lfl -ly -lcurl

run: main
	./gq

clean:
	rm gq lex.yy.c gq.tab.c gq.tab.h

