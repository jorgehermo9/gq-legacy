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

string* source_error_message=NULL;

void yyerror (Query* result_query, char const *error);
void error_at_position(string error, int line,int col);
void error_between_positions(string error, Location start, Location end);
void error_in_query(KeyInfo key_info, string error);

%}

%parse-param {Query* result_query}
// Include this so union recognizes types
%code requires {
	#include "types.hpp"
}
%union {
	KeyInfo key_info_val;
	QueryKey query_key_val;
	vector<Query>* queries_val;
	Query query_val;
	Location loc_val;
	ArgumentValue arg_value_val;
	Argument arg_val;
	vector<Argument>* arg_list_val;
}
%define parse.error verbose

%token YYEOF COLON COMMA
%token <key_info_val> KEY


%token <loc_val> LBRACKET RBRACKET LPAREN RPAREN

%token<arg_value_val> STRINGV INTV FLOATV

%type <queries_val> query_content
%type <query_val> query
%type <query_key_val> query_key
%type <arg_val> argument
%type <arg_list_val> argument_list arguments

%start S
%%

S: argument_list LBRACKET query_content RBRACKET YYEOF {
	KeyInfo key_info;
	
	key_info.name = NULL;
	key_info.at = $2;

	QueryKey query_key;
	query_key.info = key_info;
	query_key.args = NULL;

	result_query->query_key = query_key;
	result_query->children = $3;
	} 
	|LBRACKET RBRACKET YYEOF{
		KeyInfo key_info;
		key_info.name = NULL;
		key_info.at = $1;

		QueryKey query_key;
		query_key.info = key_info;
		query_key.args = NULL;

		// Allow empty root query to return all fields
		result_query->query_key = query_key;
		result_query->children = NULL;
	}
	| LBRACKET query_content YYEOF{
		string error_message = "missing closing bracket '}' for root query";
		error_at_position(error_message,lineno,colno);
	}
	| query_content RBRACKET YYEOF{
		string error_message = "missing opening bracket '{' for root query";
		error_at_position(error_message,1,1);
	}
	| LBRACKET error RBRACKET YYEOF{
		string error_message = "error in root query content";
		error_between_positions(error_message,$1,$3);
	}
	| LBRACKET error YYEOF{
		string error_message = "error in root query content";
		error_at_position(error_message,$1.line,$1.col);
	}
	| LBRACKET query_content RBRACKET error{
		string error_message = "unexpected token after root query";
		// error line and col where the lexer stopped
		error_at_position(error_message,lineno,colno);
	}
	| YYEOF{
		string error_message = "root query is not defined";
		error_at_position(error_message,lineno,colno);
	}
	|error{
		string error_message = "unexpected token";
		error_at_position(error_message,lineno,colno);
	}


argument : 
	KEY COLON STRINGV {

		Argument arg;
		arg.info = $1;
		arg.value = $3;
		$$ = arg;
	}
	| KEY COLON INTV {
		Argument arg;
		arg.info = $1;
		arg.value = $3;
		$$ = arg;
	}
	| KEY COLON FLOATV {
		Argument arg;
		arg.info = $1;
		arg.value = $3;
		$$ = arg;
	}
	| KEY COLON error {
		// Store error here and do not exit, in order to print information of the query
		source_error_message = new string("invalid argument value in field '" +CYAN + *$1.name + RESET+ 
		"' declared at "+RED+to_string($1.at.line)+":"+to_string($1.at.col)+RESET);
		// Throw error token, since if only the tokens KEY and COLON are present (empty value),
		// throw error would no throw by default (since error token could match here, and it is not
		// necessary to be in a state error)
		YYERROR;
	}

	/* TODO: Fix shift/reduce */
	/* Commenting this removes the error but idk */
	/* | KEY error {
		source_error_message = new string("expected ':' after argument field '" +CYAN + *$1.name + RESET+ 
		"' declared at "+ RED +to_string($1.at.line)+":"+to_string($1.at.col)+RESET);

		YYERROR;
	} */

argument_list : 
	// Do not handle error for arguments, since it is handled in argument rules
	argument {
		$$ = new vector<Argument>;
		$$->push_back($1);
	}
	| argument_list COMMA argument {
		$1->push_back($3);
		$$ = $1;
	}
	| argument_list error {
		Argument last_argument = $1->back();
		source_error_message = new string("unexpected token after argument field '"
		+CYAN + *last_argument.info.name + RESET+
		"' declared at "+RED+to_string(last_argument.info.at.line) + ":" + to_string(last_argument.info.at.col) + RESET
		+ "\n" + GREEN + "Hint:" + RESET + " arguments must be separated by ','");

		// Throw error token
		YYERROR;
	}
	| argument_list COMMA error {
		Argument last_argument = $1->back();
		source_error_message = source_error_message == NULL ?
			new string("unexpected token after argument field '"
			+ CYAN + *last_argument.info.name + RESET+
			"' declared at "+RED+to_string(last_argument.info.at.line) + ":" + to_string(last_argument.info.at.col) + RESET) :
			source_error_message;

		// Throw error token
		YYERROR;
	}

arguments:
	LPAREN argument_list RPAREN {
		$$ = $2;
	}
	| LPAREN error RPAREN {
		source_error_message = source_error_message==NULL?
			new string("unexpected token in query arguments"):
			source_error_message;

		YYERROR;
	}

query_key : 
	KEY {
		QueryKey query_key;
		query_key.info = $1;
		query_key.args = NULL;
		$$ = query_key;
	}
	| KEY arguments {
		QueryKey query_key;
		query_key.info = $1;
		query_key.args = $2;
		$$ = query_key;
	}
	| KEY error {
		string error_message = source_error_message==NULL?
			"unexpected token after query key":
			*source_error_message;
		error_in_query($1,error_message);
	}

query_content: query_content query_key {
		Query new_query;
		new_query.query_key = $2;
		new_query.children = NULL;

		$1->push_back(new_query);
		$$ = $1;
	}
	| query_content query {
		$1->push_back($2);
		$$ = $1;
	}
	| query_key {
		Query new_query;
		new_query.query_key = $1;
		new_query.children = NULL;

		$$ = new vector<Query>;
		$$->push_back(new_query);
	}
	| query {
		$$ = new vector<Query>;
		$$->push_back($1);
}

query: query_key LBRACKET query_content RBRACKET {
	Query new_query;
	new_query.query_key = $1;
	new_query.children = $3;
	$$ = new_query;
	}
	| query_key LBRACKET error RBRACKET{
		error_in_query($1.info,"error in query content");
	}
	| query_key LBRACKET RBRACKET{
		error_in_query($1.info,"query fields cannot be empty\n"
		+GREEN+"Hint:"+RESET+" in order to get all fields, remove brackets");
	}
	| query_key LBRACKET YYEOF {
		error_in_query($1.info,"missing closing bracket '}'");
	}
	| query_key LBRACKET query_content YYEOF {
		error_in_query($1.info,"missing closing bracket '}'");
	}
	

%%

void yyerror (Query* result_query, char const *error) {
	/* error_at_position(string(error), lineno, colno); */
}

void error_at_position(string error, int line,int col) {
    /* Some errors need to use line number on the last line scanned (TEXT XML_ELEMENT tokens),
    and others just need the starting line of the expression (starting_lineno)*/
	cerr << RED <<"Syntax error "<<RESET<<"at " << RED
       << line << ":" << col
       << RESET << ": " << error << endl;
    exit(1);
}

void error_between_positions(string error, Location start, Location end) {
	cerr << RED <<"Syntax error "<<RESET<<"between " << RED
	   << start.line << ":" << start.col
	   << RESET << " and " << RED
	   << end.line << ":" << end.col
	   << RESET << ": " << error << endl;
	exit(1);
}

void error_no_position(string error) {
	cerr << RED <<"Syntax error"<<RESET<<": " << error << endl;
	exit(1);
}

void error_in_query(KeyInfo key_info, string error) {
	cerr << RED << "Syntax error "<< RESET << "in query " <<
	CYAN << "'" << *key_info.name << "'" << RESET <<"' at " << RED
	<< key_info.at.line << ":" << key_info.at.col
	<< RESET << ": " << error << endl;
	exit(1);
}
