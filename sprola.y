/* Signal PROcessing LAnguage  */
%{
#include <stdio.h>

int yylex();

extern FILE * yyin;

void yyerror(char const*);
%}

/*
%union {
  float f;
  double d;
  int i;
  char* str;
}
*/

/* declare tokens */
%token IDENTIFIER
%token KEYWORD_MAIN
%token INTEGER
%token WHITESPACE

%%

start: /* nothing : to match beginning of input */
 | start KEYWORD_MAIN { printf("found main\n"); }
 ;

%%
int main(int argc, char **argv)
{

  if (argc > 1) {
    if (!(yyin = fopen(argv[1], "r"))) {
      perror(argv[1]);
      return (1);
    }
  }

  yyparse();
}

void yyerror(char const* s)
{
  fprintf(stderr, "error: %s\n", s);
}
