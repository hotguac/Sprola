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
extern struct symbol *lookup(char* sym);

/* Forward declaration for routines defined below in code section */
void yyerror(char const*);

%}

/* These are the possible value types returned by the lexer for tokens */
%union {
  int ival;
  char* sval;
  struct ast *a;
  struct symbol *s;
}

/* --------  declare tokens  ----------------------------------*/
%token OPTION
%token IDENTIFIER
%token FUNC
%token TYPE

%token OP "("
%token CP ")"
%token OCB "{"
%token CCB "}"
%token COMMA ","
%token COLON ":"
%token DOT "."
%token EQUAL "="

%token <ival> INTEGER

%type <a> statements statement assignment expression

%type <s> IDENTIFIER

%debug
%start program

%%
/*----------------------------------------------------------------------------*/
/*  Start of rules section                                                    */
/*----------------------------------------------------------------------------*/

program: statements  {  printrefs();
                        printf(">>>\nDump AST\n");
                        dumpast($1,1);
                        printf(">>>\n");
                        emit_code($<a>1); }
  ;

statements: %empty { $$ = NULL; }
  | statements statement { $$ = newast(N_statement_list, $1, $2); }
  ;

statement: assignment { $$ = $1; }
  ;

assignment: IDENTIFIER "=" expression { $$ = newasgn($1, $3); }
  ;

expression: INTEGER { $$ = newint($1); }
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
