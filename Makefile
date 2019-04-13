sprola: sprola.y sprola.l
	bison -d sprola.y
	flex sprola.l
	g++ -o $@ sprola.tab.c lex.yy.c -lfl
