/* Signal PROcessing LAnguage  */
%{
#include <stdio.h>

extern int yylex();
extern int yyparse();
extern FILE * yyin;
extern int yylineno;

void yyerror(char const*);
%}


%union {
  float fval;
  double dval;
  int ival;
  char* sval;
}


/* declare tokens */
%token IDENTIFIER
%token FUNC MAIN
%token EOL
%token OP CP OCB CCB OBK CBK

%token <ival> INTEGER

%%

start: /* nothing : to match beginning of input */
 | start func_main empty_params code_block { printf("found main %d\n", yylineno); }
 ;

func_main: FUNC MAIN { printf("found FUNC MAIN %d\n", yylineno); }
  ;

empty_params: OP CP { printf("found () %d\n", yylineno); }
  ;

code_block: OCB CCB { printf("found code_block\n"); }
  | OCB statements CCB { printf("found code_block\n"); }
  ;

statements: statement { printf("found a statement\n"); }
  | statements statement { printf("found a group statements\n"); }
  ;

statement: IDENTIFIER { printf("found a statement\n"); }
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
