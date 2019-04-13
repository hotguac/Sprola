/* Signal PROcessing LAnguage */
%{
#include "sprola.tab.h"
%}

%%

[ \t]     { return WHITESPACE; }
"main"    { return KEYWORD_MAIN; }
[0-9]+    { yylval = atoi(yytext); return INTEGER; }
[_a-zA-Z][a-zA-Z0-9]* { return IDENTIFIER; }
.         { /* do nothing */ }

%%
