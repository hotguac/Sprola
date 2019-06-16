cflags = `/usr/local/bin/llvm-config --cflags ` -Wall -g -O0 -c
ldflags = `/usr/local/bin/llvm-config --ldflags ` -Wall -g -O0 -lm

default_output.bc: sprola amp.spl
	./sprola -v -l amp.spl

sprola: sprola.tab.o lex.yy.o symbols.o ast.o \
		codegen.o codegen_std.o codegen_ast.o utils.o
	clang $(ldflags) -o $@  $^ \
		 /usr/local/lib/libLLVM.so \
		 /usr/local/lib/libc++.so

sprola.tab.o: sprola.tab.c
	clang -g -O0 -c -o sprola.tab.o sprola.tab.c

sprola.tab.c: sprola.y
	bison -d --report=all sprola.y
#	bison -d -v --report=all sprola.y

lex.yy.o: lex.yy.c
	clang -g -O0 -c -o lex.yy.o lex.yy.c

lex.yy.c: sprola.l sprola.y sprola.h
#	flex -d -T sprola.l or flex -T sprola.l
	flex sprola.l

symbols.o: sprola.h symbols.c symbols.h
	clang -Wall -g -O0 -c -o symbols.o symbols.c

ast.o: ast.c ast.h
	clang -Wall -g -O0 -c -o ast.o ast.c

codegen.o: codegen.c
	clang $(cflags) -o codegen.o codegen.c

codegen_std.o: codegen_std.c
	clang $(cflags) -o codegen_std.o codegen_std.c

codegen_ast.o: codegen_ast.c
	clang $(cflags) -o codegen_ast.o codegen_ast.c

utils.o: utils.c utils.h
	clang $(cflags) -o utils.o utils.c

clean:
	rm -f sprola 
	rm -f *.o
	rm -f *.so
	rm -f *.bc
	rm -f lex.yy.c
	rm -f sprola.tab.*
	rm -f sprola.output
	rm -f default_output.*
	rm -f SprolaTemp*.bc
	rm -f SprolaTemp*.ll
