/* Signal PROcessing LAnguage  */

%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// use GNU version
#define _GNU_SOURCE
#include <libgen.h>

#include <string.h>

#include "ast.h"
#include "sprola.h"
#include "utils.h"

char current_filename[MAX_FILENAME_SIZE];   // read source from here


int verbose_flag = 0;
int ll_flag = 0;

void printrefs();
void emit_code(struct ast *a);
struct symbol *lookup(char* sym);

/* Forward declaration for routines defined below in code section */
void yyerror(char const*);

%}

/* These are the possible value types returned by the lexer for tokens */
%union {
  int ival;
  float fval;
  char* sval;
  struct ast *a;
  struct symbol *sym;
}

/* --------  declare tokens  ----------------------------------*/
%token KW_For "for"
%token KW_To "to"

%token T_Option
%token T_Id
%token T_Type
%token T_Audio_In
%token T_Audio_Out
%token T_Control_In
%token T_URI

%token T_OpenP "("
%token T_CloseP ")"
%token T_OpenCB "{"
%token T_CloseCB "}"
%token T_Equal "="
%token T_OpenBr "["
%token T_CloseBR "]"

%token T_Plus "+"
%token T_Minus "-"

%token T_LessThan "<"
%token T_GreaterThan ">"

%token T_Semicolon ";"

%token <ival> T_Integer
%token <fval> T_Float

%token T_String

%type <sval> T_Type

%type <a> statements statement assignment options option
%type <a> factor term expr expra function code_block
%type <a> functions variable_declaration variable_declarations
%type <a> array_reference condition for_statement program

%type <sym> T_Id T_String

%debug
%start program

%%
/*----------------------------------------------------------------------------*/
/*  Start of rules section                                                    */
/*----------------------------------------------------------------------------*/

program
  : options variable_declarations functions
    {
      $$ = newprogram($1, $2, $3);
      if (verbose_flag) {
        printrefs();
        printf(">>>\nDump AST\n");
        dumpast($$, 1);
        printf(">>>\n");
      }
      emit_code($$);
    }
  | options functions
    {
      $$ = newprogram($1, NULL, $2);
      if (verbose_flag) {
        printrefs();
        printf(">>>\nDump AST\n");
        dumpast($$, 1);
        printf(">>>\n");
      }
      emit_code($$);
    }
  | variable_declarations functions
    {
      $$ = newprogram(NULL, $1, $2);
      if (verbose_flag) {
        printrefs();
        printf(">>>\nDump AST\n");
        dumpast($$, 1);
        printf(">>>\n");
      }
      emit_code($$);
    }
  | functions
    {
      $$ = newprogram(NULL, NULL, $1);
      if (verbose_flag) {
        printrefs();
        printf(">>>\nDump AST\n");
        dumpast($$, 1);
        printf(">>>\n");
      }
      emit_code($<a>1);
    }
  ;

options
  : options option
    {
      $$ = newast(N_options, $1, $2);
    }
  | option
    {
      $$ = newast(N_options, NULL, $1);
    }
  ;

option
  : T_Option T_Audio_In T_Id
    {
      $$ = newoption(OPT_audio_input, newsymref($3));
    }
  | T_Option T_Audio_Out T_Id
    {
      $$ = newoption(OPT_audio_output, newsymref($3));
    }
  | T_Option T_Control_In T_Id
    {
      $$ = newoption(OPT_control_in, newsymref($3));
    }
  | T_Option T_URI T_String
    {
      $$ = newoption(OPT_uri, newsymref($3));
    }
  ;

variable_declarations
  : variable_declarations variable_declaration
    {
      $$ = newast(N_var_declarations, $1, $2);
    }
  | variable_declaration
    {
      $$ = $1;
    }
  ;

variable_declaration
  : T_Type T_Id T_Semicolon
    {
      $$ = newvardecl($1, newsymref($2));
    }
  ;

functions
  : functions function
    {
      $$ = newast(N_functions, $1, $2);
    }
  | function
    {
      $$ = $1;
    }
  ;

function
  : T_Type T_Id T_OpenP T_CloseP code_block
    {
      $$ = newfunction(TY_void, newsymref($2), $5);
    }
  ;

code_block
  : T_OpenCB variable_declarations statements T_CloseCB
    {
      $$ = newast(N_scope, $2, $3);
    }
  | T_OpenCB statements T_CloseCB
    {
      $$ = $2;
    }
  | T_OpenCB T_CloseCB { $$ = NULL; }
  ;

statements
  : statements statement
    {
      $$ = newast(N_statement_list, $1, $2);
    }
  | statement
    {
      $$ = $1;
    }
  ;

statement
  : assignment T_Semicolon
    {
      $$ = $1;
      }
  | for_statement
    {
    $$ = $1;
    }
  ;

assignment
  : T_Id T_Equal expr
    {
      $$ = newasgn(newsymref($1), $3);
    }
  | array_reference T_Equal expr
    {
      $$ = newasgn($1, $3);
    }
  ;

array_reference
  : T_Id T_OpenBr expr T_CloseBR
    {
      $$ = newarrayref($1, $3);
    }
  ;

condition
  : expr T_LessThan expr
    {
      $$ = newlessthan($1, $3);
    }
  ;

for_statement
  : KW_For T_OpenP assignment T_Semicolon
    condition T_Semicolon
    assignment T_CloseP code_block
    {
      $$ = newforloop($3, $5, $7, $9);
    }
  ;

expr
  : expra
    {
      $$ = $1;
    }
  ;

expra
  : expra T_Plus term
    {
      $$ = newast(N_add, $1, $3);
    }
  | expra T_Minus term
    {
      $$ = newast(N_subtract, $1, $3);
    }
  | term
    {
      $$ = $1;
    }
  ;

term
  : term '*' factor
    {
      $$ = newast(N_multiply, $1, $3);
    }
  | term '/' factor
    {
      $$ = newast(N_divide, $1, $3);
    }
  | factor
    {
      $$ = $1;
    }
  ;

factor
  : T_OpenP expr T_CloseP
    {
      $$ = $2;
    }
  | T_Minus factor
    {
      $$ = newast(N_negate, $2, NULL);
    }
  | T_Integer
    {
      $$ = newint($1);
    }
  | T_Float
    {
      $$ = newfloat($1);
    }
  | T_Id
    {
      $$ = newsymref($1);
    }
  | array_reference
    {
      $$ = (struct ast *)$1;
    }
  ;

%%
/*----------------------------------------------------------------------------*/
/*  Start of code section                                                    */
/*----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  int index;
  int c;

  verbose_flag = 0;

  while ((c = getopt (argc, argv, "vhl")) != -1) {
    switch (c) {
      case 'v':
        verbose_flag = 1;
        break;
      case 'h':
        printf("usage: sprola [-v] [-h] source\n");
        printf("-v turn on verbose output messages\n");
        printf("-h print this message\n");
        printf("-l generate human readable llvm code for inspection in <source>.ll\n");
        break;
      case 'l':
        ll_flag = 1;
        break;
      case '?':
        if (isprint(optopt)) {
          fprintf(stderr, "Unknown option -%c\n", optopt);
          fprintf(stderr, "use sprola -h for help\n");
        } else {
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
          fprintf(stderr, "use sprola -h for help\n");
        }
        return 1;
      default:
        abort();
    }
  }

  memset(current_filename, 0, MAX_FILENAME_SIZE);

  for (index = optind; index < argc; index++) {
    if (current_filename[0] == 0) {
      strncpy(current_filename, argv[index], MAX_FILENAME_SIZE);
    } else {
      fprintf(stderr, "Too many arguements\n");
      fprintf(stderr, "use sprola -h for help\n");
    }
  }

  if (current_filename[0] == 0) {
    strncpy(current_filename, "stdin", MAX_FILENAME_SIZE);
  } else {
    if (!(yyin = fopen(current_filename, "r"))) {
      perror(current_filename);
      return (1);
    }
  }

  build_names(current_filename, &names);

  yyparse();
}

void yyerror(char const* s)
{
  fprintf(stderr, "error: %s on line %d token %s\n", s, yylineno, yytext);
}
