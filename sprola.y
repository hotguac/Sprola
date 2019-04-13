/* Signal PROcessing LAnguage */
%{
#include <stdio.h>

int yylex();
void yyerror(char const*);
%}

/* declare tokens */
%token IDENTIFIER
%token KEYWORD_MAIN
%token INTEGER
%token EOL WHITESPACE

%%

start: /* nothing : to match beginning of input */
 | start KEYWORD_MAIN EOL { printf("= %d\n", $2); }
 ;

%%
int main(int argc, char **argv)
{
  yyparse();
}

void yyerror(char const* s)
{
  fprintf(stderr, "error: %s\n", s);
}
