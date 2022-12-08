%{
using namespace std;

#include <string>
#include <iostream>

#include <string>
#include <vector>
// Include types here so yyerror recognizes struct
#include "types.hpp"



extern "C" int yylex();
extern int yylineno;

void yyerror (Query* result_query, char const *error);

string RED = "\033[0;31m";
string ORANGE = "\033[0;33m";
string GREEN = "\033[0;32m";
string WHITE = "\033[0m";
%}

%parse-param {Query* result_query}
%code requires{
	// Include this so union recognizes types
#include "types.hpp"
}
%union{
	string* key_name;
	vector<Query*>* queries;
	Query* queryVal;
}
%define parse.error verbose

%token LBRACKET RBRACKET YYEOF

%token <key_name> KEY

%type <queries> query_content
%type <queryVal> query S
%start S



%%

S: LBRACKET query_content RBRACKET YYEOF{

	Query new_query;
	new_query.key = NULL;
	new_query.children = $2;
	*result_query= new_query;
}


query_content: 
	query_content KEY
	{
		Query* new_query = (Query*) malloc(sizeof(Query));
		new_query->key = $2;
		new_query->children = NULL;
		$1->push_back(new_query);
		$$ = $1;
	}
	| query_content query {
		$1->push_back($2);
		$$ = $1;
	}
	| KEY {
		Query* new_query = (Query*) malloc(sizeof(Query));
		new_query->key = $1;
		new_query->children = NULL;

		$$ = new vector<Query*>;
		$$->push_back(new_query);
	}
	| query {
		$$ = new vector<Query*>;
		$$->push_back($1);
	}

query: KEY LBRACKET query_content RBRACKET{
	Query* new_query = (Query*) malloc(sizeof(Query));
	new_query->key = $1;
	new_query->children = $3;
	$$ = new_query;
}

%%


void yyerror (Query* result_query, char const *error) {

	cout << RED << "Error: " << WHITE << error << endl;
	cout << ORANGE << "Line: " << WHITE << yylineno << endl;
}



