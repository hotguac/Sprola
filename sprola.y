/* Signal PROcessing LAnguage  */

%{
#include <stdio.h>

extern int yylex();
extern int yyparse();
extern FILE * yyin;
extern int yylineno;
extern char *yytext;

extern char *current_filename;
extern void printrefs();

void yyerror(char const*);
%}

%union {
  float fval;
  double dval;
  int ival;
  char* sval;
}

/* declare tokens */
%token OPTION
%token IDENTIFIER
%token FUNC
%token EOL
%token OP CP OCB CCB OBK CBK
%token TYPE
%token COMMA ","
%token COLON ":"
%token DOT "."

%token <ival> INTEGER

%%

program: /* */
 | functions         { printf("no options, just functions\n"); printrefs(); }
 | options functions  { printf("options and functions\n"); printrefs(); }
 ;

options: /* nothing to match beginning of input */
 | options OPTION IDENTIFIER { printf("Looks like we're building an LV2 Plugin!\n"); }
 ;

functions: /* nothing : to match beginning of input */
 | functions function params code_block { printf("found main %d\n", yylineno); }
 ;

function: FUNC IDENTIFIER { printf("found FUNC on line %d\n", yylineno); }
  ;

params: OP CP { printf("found () %d\n", yylineno); }
    | OP param_list CP { printf("found () %d\n", yylineno); }
    ;

param_list: param_list COMMA parameter { /* */ }
  | parameter { /* */ }
  ;

type: TYPE { /* */ }
    ;

parameter: IDENTIFIER COLON type { printf("declared parameter\n"); }
    ;

code_block: OCB CCB { printf("found code_block\n"); }
  | OCB statements CCB { printf("found code_block\n"); }
  ;

reference: IDENTIFIER DOT IDENTIFIER
  ;

statements: statement { printf("found a statement\n"); }
  | statements statement { printf("found a group statements\n"); }
  ;

statement: IDENTIFIER '=' INTEGER { printf("found a statement\n"); }
  | reference '=' INTEGER { printf("found a statement\n"); }
  ;

%%

int main(int argc, char **argv)
{

  if (argc > 1) {
    current_filename = argv[1];

    if (!(yyin = fopen(argv[1], "r"))) {
      perror(argv[1]);
      return (1);
    }
  }

  yyparse();
}

void yyerror(char const* s)
{
  fprintf(stderr, "error: %s on line %d token %s\n", s, yylineno, yytext);
}
