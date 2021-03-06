/* Signal PROcessing LAnguage */
/*   Flex definition          */

/* we'll create our own main function so we */
/*   don't need the default routine         */
%option nodefault

/* we only need a single input file         */
%option noyywrap

/* automatic line numbers for debugging     */
%option yylineno

%{
#include "sprola.tab.h" /* Generated by Bison */
#include "sprola.h"     /* Manually maintained */

void yyerror(char const*);

%}

digit [0-9]

%%

[ \t\n]   ;

^"%option" { return T_Option; }
"Audio_In"  { return T_Audio_In; }
"Audio_Out" { return T_Audio_Out; }
"Control_In" { return T_Control_In; }
"URI" { return T_URI; }

"void" { yylval.sval = strdup(yytext); return T_Type; }
"int" { yylval.sval = strdup(yytext); return T_Type; }
"float" { yylval.sval = strdup(yytext); return T_Type; }

"for" { return KW_For; }
"if" { return KW_If; }
"else" { return KW_Else; }

"="       { return T_Equal; }
"-"       { return T_Minus; }
"+"       { return T_Plus; }
"("       { return T_OpenP; }
")"       { return T_CloseP; }
"{"       { return T_OpenCB; }
"}"       { return T_CloseCB; }
"["       { return T_OpenBr; }
"]"       { return T_CloseBR; }
";"       { return T_Semicolon; }
"<"       { return T_LessThan; }
">"       { return T_GreaterThan; }

\"[^"\\]*\" { addref(yylineno, current_filename, yytext, 0);
              yylval.sym = lookup(yytext); return T_String; }


{digit}+    { yylval.ival = atoi(yytext); return T_Integer; }
([+-]*{digit}+[.]{digit}*)|({digit}*[.]{digit}+) { yylval.fval = atof(yytext); return T_Float; }


"//".*    { /* ignore comments for now */ }

[_a-zA-Z][a-zA-Z0-9_]* {  addref(yylineno, current_filename, yytext, 0);
                          yylval.sym = lookup(yytext);
                          return T_Id; }


.         { return yytext[0]; }

%%
