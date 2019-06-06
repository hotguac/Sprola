sprola: sprola.tab.o lex.yy.o symbols.o ast.o codegen.o
	clang `/usr/local/bin/llvm-config --ldflags ` \
		-g -O0 -lm -o $@ \
		sprola.tab.o lex.yy.o symbols.o ast.o codegen.o \
		 /usr/local/lib/libLLVM.so \
		 /usr/local/lib/libc++.so

sprola.tab.o: sprola.tab.c
	clang -g -O0 -c -o sprola.tab.o sprola.tab.c

sprola.tab.c: sprola.y
	bison -d -v --report=all sprola.y
#	bison -d -v --report=all sprola.y

lex.yy.o: lex.yy.c
	clang -g -O0 -c -o lex.yy.o lex.yy.c

lex.yy.c: sprola.l sprola.y sprola.h
#	flex -d -T sprola.l or flex -T sprola.l
	flex -T sprola.l

symbols.o: sprola.h symbols.c symbols.h
	clang -g -O0 -c -o symbols.o symbols.c

ast.o: ast.c ast.h
	clang -g -O0 -c -o ast.o ast.c

codegen.o: codegen.c
	clang `/usr/local/bin/llvm-config --cflags` \
 -g -O0 -c -o codegen.o codegen.c

clean:
	rm -f *.o
	rm -f lex.yy.c
	rm -f sprola.tab.*
	rm -f sprola.output
	rm -f default_output.*
