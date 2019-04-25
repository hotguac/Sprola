sprola: sprola.tab.c lex.yy.c symbols.o ast.o
	clang -g -O0 -lm -o $@ sprola.tab.c lex.yy.c symbols.o ast.o

sprola.tab.c: sprola.y
	bison -d --report=all sprola.y
#	bison -d -v --report=all sprola.y

lex.yy.c: sprola.l sprola.y sprola.h
#	flex -d -T sprola.l
	flex -T sprola.l

symbols.o: sprola.h symbols.c symbols.h
	clang -g -O0 -c -o symbols.o symbols.c

ast.o: ast.c ast.h
	clang -g -O0 -c -o ast.o ast.c

clean:
	rm *.o
	rm lex.yy.c
	rm sprola.tab.*
	rm sprola.output
