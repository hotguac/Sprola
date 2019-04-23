sprola: sprola.tab.c lex.yy.c symbols.o ast.o
	clang -lm -o $@ sprola.tab.c lex.yy.c symbols.o ast.o

sprola.tab.c: sprola.y
	bison -d -v --report=all sprola.y

lex.yy.c: sprola.l sprola.y sprola.h
	flex -d -T sprola.l

symbols.o: sprola.h symbols.c
	clang -c -o symbols.o symbols.c

ast.o: ast.c ast.h
	clang -c -o ast.o ast.c

clean:
	rm *.o
	rm lex.yy.c
	rm sprola.tab.*
	rm sprola.output
