sprola: sprola.tab.c lex.yy.c
	g++ -o $@ sprola.tab.c lex.yy.c

sprola.tab.c: sprola.y
	bison -d --report=all sprola.y

lex.yy.c: sprola.l sprola.y sprola.h
	flex -d -T sprola.l
