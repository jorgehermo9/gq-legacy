%{
#include <string>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
// Include types here so yyerror recognizes struct
#include "types.hpp"

extern "C" int yylex();
extern int lineno;
extern int colno;

void yyerror (Query* result_query, char const *error);
void error_at_position(string error, int line,int col);
void error_in_query(KeyInfo key_info, string error);

%}

%parse-param {Query* result_query}
// Include this so union recognizes types
%code requires {
	#include "types.hpp"
}
%union {
	KeyInfo key_info_val;
	vector<Query>* queries_val;
	Query query_val;
}
%define parse.error verbose

%token LBRACKET RBRACKET YYEOF
%token <key_info_val> KEY

%type <queries_val> query_content
%type <query_val> query
%start S
%%

S: LBRACKET query_content RBRACKET YYEOF {
	KeyInfo key_info;
	
	key_info.name = NULL;
	key_info.declared_line=0;
	key_info.declared_col=0;
	
	result_query->key_info = key_info;
	result_query->children = $2;
	} 
	|LBRACKET RBRACKET YYEOF{
		// We allow empty queries on root and it will return an empty json object
		KeyInfo key_info;

		key_info.name = NULL;
		key_info.declared_line=0;
		key_info.declared_col=0;

		result_query->key_info = key_info;
		result_query->children = new vector<Query>;
	}
	| LBRACKET query_content YYEOF{
		string error_message = "missing closing bracket '}' for root query";
		error_at_position(error_message,0,0);
	}| query_content RBRACKET YYEOF{
		string error_message = "missing opening bracket '{' for root query";
		error_at_position(error_message,0,0);
	}

query_content: query_content KEY {
		Query new_query;
		new_query.key_info = $2;
		new_query.children = NULL;

		$1->push_back(new_query);
		$$ = $1;
	}
	| query_content query {
		$1->push_back($2);
		$$ = $1;
	}
	| KEY {
		Query new_query;
		new_query.key_info = $1;
		new_query.children = NULL;

		$$ = new vector<Query>;
		$$->push_back(new_query);
	}
	| query {
		$$ = new vector<Query>;
		$$->push_back($1);
	}

query: KEY LBRACKET query_content RBRACKET {
	Query new_query;
	new_query.key_info = $1;
	new_query.children = $3;
	$$ = new_query;
	}
	| KEY LBRACKET RBRACKET{
		error_in_query($1,"missing query content");
	}
	| KEY LBRACKET YYEOF {
		error_in_query($1,"missing closing bracket '}'");
	}
	| KEY LBRACKET query_content YYEOF {
		error_in_query($1,"missing closing bracket '}'");
	}

%%


void yyerror (Query* result_query, char const *error) {
	error_at_position(string(error), lineno, colno);
}



void error_at_position(string error, int line,int col) {
    /* Some errors need to use line number on the last line scanned (TEXT XML_ELEMENT tokens),
    and others just need the starting line of the expression (starting_lineno)*/
	cerr << RED <<"Syntax error "<<RESET<<"at " << RED
       << line << ":" << col
       << RESET << ": " << error << endl;
    exit(1);
}
void error_no_position(string error) {
	cerr << RED <<"Syntax error"<<RESET<<": " << error << endl;
	exit(1);
}

void error_in_query(KeyInfo key_info, string error) {
	cerr << RED << "Syntax error "<< RESET << "in query '" <<
	CYAN << *key_info.name << RESET <<"' at " << RED
	<< key_info.declared_line << ":" << key_info.declared_col
	<< RESET << ": " << error << endl;
	exit(1);
}