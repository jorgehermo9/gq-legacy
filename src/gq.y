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
void error_at_position(string error, Location at);
void error_between_positions(string error, Location start, Location end);
void error_in_query(KeyInfo key_info, string error);
void error_in_root(Location at, string error);
void update_source_error(string new_message);
string get_loc_string(Location loc);

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
	ArgumentOperation arg_op_val;
	Argument arg_val;
	vector<Argument>* arg_list_val;
	string* string_val;
	OperationModifier modifier_val;
	Operation op_val;
	stack<KeyInfo>* key_stack_val;
}
%define parse.error verbose

%token YYEOF COLON COMMA NOT TILDE CARET LT GT EQ STAR DOLLAR DOT
%token <key_info_val> KEY

%token <loc_val> LBRACKET RBRACKET LPAREN RPAREN
%token<arg_value_val> STRINGV INTV FLOATV BOOLV NULLV

%type <queries_val> query_content
%type <query_val> query
%type <query_key_val> query_key
%type <arg_val> argument
%type <arg_list_val> argument_list arguments
%type <string_val> alias
%type <op_val> operation
%type <modifier_val> modifier
%type <arg_op_val> arg_operation
%type <arg_value_val> arg_value
%type <key_stack_val> arg_key_path

%start S
%%

S: arguments LBRACKET query_content RBRACKET YYEOF {
	KeyInfo key_info;
	
	key_info.name = NULL;
	key_info.at = $2;

	QueryKey query_key;
	query_key.info = key_info;
	query_key.args = $1;

	result_query->query_key = query_key;
	result_query->children = $3;
	} 
	| arguments LBRACKET RBRACKET YYEOF{
		KeyInfo key_info;
		key_info.name = NULL;
		key_info.at = $2;

		QueryKey query_key;
		query_key.info = key_info;
		query_key.args = $1;

		// Allow empty root query to return all fields
		result_query->query_key = query_key;
		result_query->children = NULL;
	}
	| arguments LBRACKET query_content YYEOF {
		Location at = { .line = lineno, .col = colno };
		string error_message = "missing closing bracket '}' for root query";
		error_at_position(error_message, at);
	}
	| arguments query_content RBRACKET YYEOF {
		Location at = { .line = 1, .col = 1 };
		string error_message = "missing opening bracket '{' for root query";
		error_at_position(error_message, at);
	}
	| arguments LBRACKET error RBRACKET YYEOF {
		string error_message = "unexpected token in root query content";
		error_between_positions(error_message, $2, $4);
	}
	| arguments LBRACKET error YYEOF {
		string error_message = "unexpected token in root query content";
		error_at_position(error_message, $2);
	}
	| arguments LBRACKET query_content RBRACKET error {
		Location at = { .line = lineno, .col = colno };
		string error_message = "unexpected token after root query";
		error_at_position(error_message, at);
	}
	| arguments YYEOF {
		Location at = { .line = lineno, .col = colno };
		string error_message = "root query is not defined";
		error_at_position(error_message, at);
	}
	/* Arguments could have generated an error that did not exit, so exit here */
	| error {
		string message = "unexpected token";
		update_source_error(message);

		Location at = { .line = lineno, .col = colno };
		error_in_root(at, *source_error_message);
	}

modifier:
	{$$ = NONE; }
	| STAR { $$=CASE_INSENSITIVE; }

operation:
	{ $$ = EQ_OP; }
	| TILDE { $$ = CONTAINS_OP; }
	| NOT TILDE { $$ = NOT_CONTAINS_OP; }
	| CARET { $$ = STARTS_WITH_OP; }
	| NOT CARET { $$ = NOT_STARTS_WITH_OP; }
	| DOLLAR { $$ = ENDS_WITH_OP; }
	| NOT DOLLAR { $$ = NOT_ENDS_WITH_OP; }
	| LT { $$ = LT_OP; }
	| GT { $$ = GT_OP; }
	| LT EQ { $$ = LE_OP; }
	| GT EQ { $$ = GE_OP; }
	| NOT EQ { $$ = NE_OP; }
	| EQ { $$ = EQ_OP; }

arg_operation:
	operation modifier {
	ArgumentOperation arg_op;
	arg_op.op = $1;
	arg_op.modifier = $2;
	$$ = arg_op;
	}

arg_value: 
	STRINGV { $$ = $1; }
	| INTV { $$ = $1; }
	| FLOATV { $$ = $1; }
	| BOOLV { $$ = $1; }
	| NULLV { $$ = $1; }

arg_key_path:
	KEY {
		stack<KeyInfo>* key_stack = new stack<KeyInfo>();
		key_stack->push($1);
		$$ = key_stack;
	}
	| KEY DOT arg_key_path {
		$3->push($1);
		$$ = $3;
	}

argument: 
	arg_key_path COLON arg_operation arg_value {
		Argument arg;
		arg.key_path = $1;
		arg.operation = $3;
		arg.value = $4;
		$$ = arg;
	}
	| COLON arg_operation arg_value {
		Argument arg;
		arg.key_path = new stack<KeyInfo>();
		arg.operation = $2;
		arg.value = $3;
		$$ = arg;
	}
	| arg_key_path COLON error arg_value {
		string key_path = join_key_path(*$1);
		Location first_location = $1->top().at;
		string message = "invalid operation in field '" + CYAN(key_path) + 
			"' declared at " + RED(get_loc_string(first_location));
		update_source_error(message);

		YYERROR; // Throw error sice it's not thrown by default
	}
	| arg_key_path COLON error {
		string key_path = join_key_path(*$1);
		Location first_location = $1->top().at;
		string message = "invalid argument value in field '" + CYAN(key_path) + 
		"' declared at " + RED(get_loc_string(first_location));
		update_source_error(message);

		YYERROR;
	}
	| arg_key_path error {
		string key_path = join_key_path(*$1);
		Location first_location = $1->top().at;
		string message = "expected ':' after argument field '" + CYAN(key_path) + 
			"' declared at "+ RED(get_loc_string(first_location));
		update_source_error(message);

		YYERROR;
	}

argument_list: 
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
		string key_path = join_key_path(*last_argument.key_path);
		Location first_location = last_argument.key_path->top().at;
		string message = "unexpected token after argument field '"
			+ CYAN(key_path)
			+ "' declared at "+ RED(get_loc_string(first_location))
			+ "\n" + GREEN("Hint:") + " arguments must be separated by ','";
		update_source_error(message);

		YYERROR;
	}
	| argument_list COMMA error {
		Argument last_argument = $1->back();
		string key_path = join_key_path(*last_argument.key_path);
		Location first_location = last_argument.key_path->top().at;
		string message = "unexpected token after argument field '"
				+ CYAN(key_path) +
				"' declared at " + RED(get_loc_string(first_location));
		update_source_error(message);

		YYERROR;
	}

arguments:
	{ $$ = NULL; }
	| LPAREN argument_list RPAREN {
		$$ = $2;
	}
	| LPAREN RPAREN {
			source_error_message = source_error_message == NULL ? 
				new string("arguments cannot be empty"):
				source_error_message;
			YYERROR;
	}
	| LPAREN error RPAREN {
		source_error_message = source_error_message == NULL ?
			new string("unexpected token in query arguments") :
			source_error_message;
			YYERROR;
	}
	| LPAREN error {
		update_source_error("unexpected token in query arguments, expected ')'");
		YYERROR;
	}

alias:
	{ $$ = NULL; }
	| COLON KEY {
		$$ = $2.name;
	}
	| COLON error {
		update_source_error("unexpected token in alias after ':'");
		YYERROR;
	}

query_key:
	KEY alias arguments {
		QueryKey query_key;
		query_key.info = $1;
		query_key.args = $3;
		query_key.alias = $2 != NULL ? $2 : new string(*$1.name);
		$$ = query_key;
	}
	| KEY error {
		update_source_error("unexpected token after query key");
		error_in_query($1, *source_error_message);
	}

query_content:
	query_content query_key {
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

query:
	query_key LBRACKET query_content RBRACKET {
		Query new_query;
		new_query.query_key = $1;
		new_query.children = $3;
		$$ = new_query;
	}
	| query_key LBRACKET error RBRACKET {
		error_in_query($1.info, "error in query content");
	}
	| query_key LBRACKET RBRACKET{
		error_in_query($1.info, "query fields cannot be empty\n"
		+ GREEN("Hint:") + " in order to get all fields, remove brackets");
	}
	| query_key LBRACKET YYEOF {
		error_in_query($1.info, "missing closing bracket '}'");
	}
	| query_key LBRACKET query_content YYEOF {
		error_in_query($1.info, "missing closing bracket '}'");
	}
	
%%

void yyerror (Query* result_query, char const *error) {
	/* error_at_position(string(error), lineno, colno); */
}

void update_source_error(string new_message) {
	source_error_message = source_error_message == NULL ?
		new string(new_message) :
		source_error_message;
}

string get_loc_string(Location at) {
	return to_string(at.line) + ":" + to_string(at.col);
}

void error_at_position(string error, Location at) {
	cerr << RED("Syntax error") <<" at " << RED(get_loc_string(at))
		<< ": " << error << endl;
  exit(1);
}

void error_between_positions(string error, Location start, Location end) {
	cerr << RED("Syntax error") <<" between " << RED(get_loc_string(start))
		<< " to " << RED(get_loc_string(end))
		<< ": " << error << endl;
	exit(1);
}

void error_in_query(KeyInfo key_info, string error) {
	cerr << RED("Syntax error") << " in query " <<
	CYAN("'" + *key_info.name + "'") << " at " << RED(get_loc_string(key_info.at))
	<< ": " << error << endl;
	exit(1);
}

void error_in_root(Location at, string error) {
	cerr << RED("Syntax error") << " in " <<
	CYAN("root query") << " at " << RED(get_loc_string(at))
	<< ": " << error << endl;
	exit(1);
}
