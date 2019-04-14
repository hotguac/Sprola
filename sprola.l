/* Signal PROcessing LAnguage */
%option nodefault noyywrap yylineno

%{
#include "sprola.tab.h"
%}

%%

[ \t\n]     { return WHITESPACE; }
"main"    { return KEYWORD_MAIN; }
[0-9]+    { yylval = atoi(yytext); return INTEGER; }
[_a-zA-Z][a-zA-Z0-9]* { return IDENTIFIER; }

.         { /* do nothing */ }

%%
