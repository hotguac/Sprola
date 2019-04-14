sprola: sprola.tab.c lex.yy.c
	g++ -o $@ sprola.tab.c lex.yy.c

sprola.tab.c: sprola.y
	bison -d sprola.y

lex.yy.c: sprola.l sprola.y
	flex sprola.l
