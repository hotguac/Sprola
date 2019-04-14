/* Signal PROcessing LAnguage */
%option nodefault noyywrap yylineno

%{
#include "sprola.tab.h"

extern void yyerror(char const*);

%}

%%

[ \t]   ;
\n        { /* ++yylineno; */ }
"func"    { yylval.sval = strdup(yytext); return FUNC; }
"main"    { yylval.sval = strdup(yytext); return MAIN; }
"("       { return OP; }
")"       { return CP; }
"{"       { return OCB; }
"}"       { return CCB; }
"["       { return OBK; }
"]"       { return CBK; }
[0-9]+    { yylval.ival = atoi(yytext); return INTEGER; }
[_a-zA-Z][a-zA-Z0-9]* { return IDENTIFIER; }
"//".*    { /* ignore comments for now */ }

.         { /* do nothing */ }

%%
