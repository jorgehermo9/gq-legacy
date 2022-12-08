%{
#include "gq.tab.h"
#include <iostream>
#include <string>
using namespace std;


// This definition is in order
extern "C" int yylex();
extern void yyerror (char const *);

extern char* RED;
extern char* WHITE;
%}

letters [a-zA-Z]
alphanumeric [a-zA-Z0-9_]

%%

({letters}|"_")({alphanumeric})* { 
	yylval.key_name = new string(yytext);
	return KEY;
}

"{" { return LBRACKET;}
"}" { return RBRACKET;}

\n  {yylineno++;}
[\t ] {}
. {

	cerr << "lexical error at line " << yylineno << ": invalid character '" << yytext << "'" << endl;
	exit(1);
}

%%
