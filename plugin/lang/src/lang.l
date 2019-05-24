%{
#include <string>
#include "lang.tab.hpp"

#include <iostream>

int read_input(char* buffer, int* num_bytes_read, int max_bytes_to_read);

#undef YY_INPUT
#define YY_INPUT(b, r, s) read_input(b, &r, s)

	void yyerror(const char* s);
%}

%option noyywrap

%%

	/* Skip comments */
[ \t]*[;][^\n]+ ;
	/* Match text */
[^\[\]\n\r=\\]+ {
	yylval.sval = new std::string(yytext, yyleng);
	return STRING;
}
[\\] {
	return CONTINUE;
}
"=" {
	return ASSIGNMENT;
}
"[" {
	return SECTION_START;
}
"]" {
	return SECTION_END;
}
[\n\r]+ {
	return NEWLINE;
}
[ \t] ;

%%
