all: gq

gq.tab.c gq.tab.h: gq.y
	bison -o gq.tab.c gq.y -H

lex.yy.c: gq.l gq.tab.h
	flex gq.l

lexer.o: lex.yy.c
	g++ -c -o lexer.o lex.yy.c

parser.o: gq.tab.c
	g++ -c -o parser.o gq.tab.c

types.o: types.cpp types.hpp
	g++ -c -o types.o types.cpp

errors.o: errors.cpp errors.hpp
	g++ -c -o errors.o errors.cpp

filter.o: filter.cpp filter.hpp
	g++ -c -o filter.o filter.cpp

gq: main.cpp filter.o parser.o lexer.o types.o errors.o
	g++ -o gq main.cpp filter.o parser.o lexer.o types.o errors.o -lcurl -lfl -ly

run: gq
	./gq

clean:
	rm gq lex.yy.c gq.tab.c gq.tab.h parser.o lexer.o types.o errors.o filter.o

