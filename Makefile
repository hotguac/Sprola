sprola: sprola.tab.c lex.yy.c symbols.o ast.o codegen.o
	clang \
	`llvm-config --ldflags \
	--libs core executionengine analysis native bitwriter --system-libs` \
	-g -O0 -lm -o $@ sprola.tab.c lex.yy.c symbols.o ast.o codegen.o

sprola.tab.c: sprola.y
	bison -d -v --report=all sprola.y
#	bison -d -v --report=all sprola.y

lex.yy.c: sprola.l sprola.y sprola.h
#	flex -d -T sprola.l or flex -T sprola.l
	flex -T sprola.l

symbols.o: sprola.h symbols.c symbols.h
	clang -g -O0 -c -o symbols.o symbols.c

ast.o: ast.c ast.h
	clang -g -O0 -c -o ast.o ast.c

codegen.o: codegen.c
	clang `llvm-config --cflags` \
	-I/usr/include/llvm-c-4.0/ \
	-I/usr/include/llvm-4.0/ \
	-g -O0 -c -o codegen.o codegen.c

clean:
	rm -f *.o
	rm -f lex.yy.c
	rm -f sprola.tab.*
	rm -f sprola.output
	rm -f default_output.*
