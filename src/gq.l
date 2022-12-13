%{
#include "gq.tab.h"
#include <iostream>
#include <string>
#include "types.hpp"
using namespace std;

extern "C" int yylex();
extern void yyerror (char const *);

int lineno = 1;
int colno = 1;
%}

letters [a-zA-Z]
alphanumeric [a-zA-Z0-9_]

%%

({letters}|"_")({alphanumeric})* {
	KeyInfo key_info;
	key_info.name = new string(yytext);
	key_info.declared_line = lineno;
	key_info.declared_col = colno;

	colno+=strlen(yytext);

	yylval.key_info_val = key_info;
	return KEY;
}

"{" { colno++;return LBRACKET;}
"}" { colno++;return RBRACKET;}

\n  {colno=1;lineno++;}
[\t ] {colno++;}
. {
	cerr << RED <<"Lexical error" <<RESET<< " at position " <<RED<< lineno
	<<":"<<colno<< RESET<<": invalid character '" << yytext << "'" << endl;
	exit(1);
}

%%
