/* Signal PROcessing LAnguage  */

%{
#include <stdio.h>
#include <string.h>
#include "ast.h"

/* These external routines are defined in or generated from sprola.l */
extern int yylex();
extern int yyparse();
extern FILE * yyin;
extern int yylineno;
extern char *yytext;

char *current_filename;   // read source from here
char *output_filename;    // write .ll output here

extern void printrefs();
extern void emit_code(struct ast *a);

/* Forward declaration for routines defined below in code section */
void yyerror(char const*);

%}

/* These are the possible value types returned by the lexer for tokens */
%union {
  float fval;
  double dval;
  int ival;
  char* sval;
  struct ast *a;
}

/* --------  declare tokens  ----------------------------------*/
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

%type <a> statements statement
%%
/*----------------------------------------------------------------------------*/
/*  Start of rules section                                                    */
/*----------------------------------------------------------------------------*/

program: statements  { printrefs(); printf(">>>\n"); emit_code($<a>1); }
 ;

statements: /* */
 | statements statement { $$ = newnum(2); }
 ;

statement: assignment
   | reference '=' INTEGER { /* */ }
   | function { /* */ }
   | option { /* */ }
   ;

assignment: IDENTIFIER '=' INTEGER { /* */ }

option: OPTION IDENTIFIER { /* */ }
 ;

function: FUNC IDENTIFIER params code_block { /* */ }
 ;

params: OP CP { /* */ }
    | OP param_list CP { /* */ }
    ;

param_list: param_list COMMA parameter { /* */ }
  | parameter { /* */ }
  ;

type: TYPE { /* */ }
    ;

parameter: IDENTIFIER COLON type { /* */ }
    ;

code_block: OCB CCB { /* */ }
  | OCB statements CCB { /* */ }
  ;

reference: IDENTIFIER DOT IDENTIFIER { /* */ }
  ;

%%
/*----------------------------------------------------------------------------*/
/*  Start of code section                                                    */
/*----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  static const char *default_input = "stdin";
  static const char *default_output = "default_output.ll";

  if (argc > 1) {
    current_filename = argv[1];

    if (!(yyin = fopen(argv[1], "r"))) {
      perror(argv[1]);
      return (1);
    }
  } else {
    current_filename = strdup(default_input);
  }

  if (argc > 2) {
    output_filename = argv[2];
  } else {
    output_filename = strdup(default_output);
  }

  yyparse();
}

void yyerror(char const* s)
{
  fprintf(stderr, "error: %s on line %d token %s\n", s, yylineno, yytext);
}
