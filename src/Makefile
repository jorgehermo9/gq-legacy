all: compile

compile: gq.l gq.y
	flex gq.l
	bison -o gq.tab.c gq.y -yd
	g++ -o gq lex.yy.c gq.tab.c -lfl -ly

run: compile
	./gq

clean:
	rm gq lex.yy.c gq.tab.c gq.tab.h

