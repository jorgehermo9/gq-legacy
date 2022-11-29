%{
#include <stdlib.h>
#include <time.h>
#include "gq.tab.h"

// This definition is in order
extern "C" int yylex();
extern void yyerror (char const *);

extern char* RED;
extern char* WHITE;

int count_linebreaks(char *s) {
	int count = 0;
	while (*s) {
		if (*s == '\n') count++;
		s++;
	}
	return count;
}
%}

tag_label [_a-zA-Z][_a-zA-Z0-9]*

text [^<>'"\t\n ][^<>'"]*

comment "<!--"([^-]|"-"[^-])*"-->"

%%

"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" {
	yylval.linenumber = yylineno;
	return XML_DECLARATION;
	}


{comment} {yylineno+=count_linebreaks(yytext);}


"<"{tag_label}">" {
	// copy substring (1, length - 1) from yytext to new buffer
	char *label = (char*)malloc(sizeof(char) * (yyleng - 2 + 1));
	strncpy(label, yytext + 1, yyleng - 2);
	label[yyleng - 2] = '\0';

	struct GenericInfo* generic_info = (GenericInfo*) malloc(sizeof(struct GenericInfo));
	generic_info->string = label;
	generic_info->starting_line = yylineno;

	yylval.generic_info = generic_info;
	return OPENING_TAG; }

"</"{tag_label}">" {
	char *label = (char*)malloc(sizeof(char) * (yyleng - 3 + 1));
	strncpy(label, yytext + 2, yyleng - 3);
	label[yyleng - 3] = '\0';

	struct GenericInfo* generic_info = (GenericInfo*) malloc(sizeof(struct GenericInfo));
	generic_info->string = label;
	generic_info->starting_line = yylineno;

	yylval.generic_info = generic_info;
	return CLOSING_TAG; }

"<"{tag_label}"/>" {
	char *label = (char*) malloc(sizeof(char) * (yyleng - 3 + 1));
	strncpy(label, yytext + 1, yyleng - 3);
	label[yyleng - 3] = '\0';

	struct GenericInfo* generic_info =(GenericInfo*)  malloc(sizeof(struct GenericInfo));
	generic_info->string = label;
	generic_info->starting_line = yylineno;

	yylval.generic_info = generic_info;
	return SELF_CLOSING_TAG; }

{text} {
	//Increment lineno by the number of \n in the text
	int starting_lineno = yylineno;

	yylineno+=count_linebreaks(yytext);


	// copy substring (0, length) from yytext to new buffer
	char *text = (char*)malloc(sizeof(char) * yyleng);
	strncpy(text, yytext, yyleng);

	struct GenericInfo* generic_info =(GenericInfo*)  malloc(sizeof(struct GenericInfo));
	generic_info->string = text;
	generic_info->starting_line = starting_lineno;

	yylval.generic_info = generic_info;

	return TEXT_FRAGMENT; }

\n  {yylineno++;}
[\t ] {}


. {

	fprintf(stderr, "%slexical error at line %d:%s invalid character '%s'\n", RED, yylineno, WHITE, yytext);	
	exit(1);
}
%%
