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

Location get_location() {
	Location loc;
	loc.line = lineno;
	loc.col = colno;
	return loc;
}

%}

letters [a-zA-Z]
alphanumeric [a-zA-Z0-9_]

key ({letters}|"_"){alphanumeric}*

%%

"true" {
	ArgumentValue arg_value;
	arg_value.type = BOOL;
	arg_value.v.b = true;

	yylval.arg_value_val = arg_value;
	colno+=strlen(yytext);
	return BOOLV;
}

"false" {
	ArgumentValue arg_value;
	arg_value.type = BOOL;
	arg_value.v.b = false;

	yylval.arg_value_val = arg_value;
	colno+=strlen(yytext);
	return BOOLV;
}

"null" {
	ArgumentValue arg_value;
	arg_value.type = NULLT;
	arg_value.v.n = NULL;

	yylval.arg_value_val = arg_value;
	colno+=strlen(yytext);
	return NULLV;
}

"!" {
	colno++;
	return NOT;
}

"~" {
	colno++;
	return TILDE;
}

"^" {
	colno++;
	return CARET;
}

"<" {
	colno++;
	return LT;
}

">" {
	colno++;
	return GT;
}

"=" {
	colno++;
	return EQ;
}

{key} {
	KeyInfo key_info;
	key_info.name = new string(yytext);
	key_info.at = get_location();


	colno+=strlen(yytext);

	yylval.key_info_val = key_info;
	return KEY;
}

"{" { 
	yylval.loc_val = get_location();
	colno++;
	return LBRACKET;
}

"}" { 
	yylval.loc_val = get_location();	
	colno++;
	return RBRACKET;
}

"(" { 
	yylval.loc_val = get_location();	
	colno++;
	return LPAREN;
}

")" { 
	yylval.loc_val = get_location();	
	colno++;
	return RPAREN;
}

":" { 
	colno++;
	return COLON;
}

"," { 
	colno++;
	return COMMA;
}


"\""[^"]*"\"" {
	string* text_string = new string(yytext);
	string* text = new string(text_string->substr(1,text_string->length()-2));
	ArgumentValue arg_value;
	arg_value.type = STRING;
	arg_value.v.str = text;

	yylval.arg_value_val = arg_value;
	colno+=strlen(yytext);
	return STRINGV;
}


[0-9]+ {
	ArgumentValue arg_value;
	arg_value.type = INT;
	arg_value.v.i = atoi(yytext);

	yylval.arg_value_val = arg_value;

	colno+=strlen(yytext);
	return INTV;
}


[0-9]+\.[0-9]+ {
	ArgumentValue arg_value;
	arg_value.type = FLOAT;
	arg_value.v.f = atof(yytext);

	yylval.arg_value_val = arg_value;
	colno+=strlen(yytext);
	return FLOATV;
}

#.* {
	// Ignore comments in lexer
	colno+=strlen(yytext);
}

\n  {colno=1;lineno++;}
[\t ] {colno++;}
. {
	cerr << RED("Lexical error") << " at position " << RED(to_string(lineno)
	+ ":" + to_string(colno)) <<": invalid character '" << yytext << "'" << endl;
	exit(1);
}

%%
