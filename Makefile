cflags = `/usr/local/bin/llvm-config --cflags ` -Wall -g -O0 -c
ldflags = `/usr/local/bin/llvm-config --ldflags ` -Wall -g -O0 -lm -lbsd

always.run: sprola amp.spl
	./sprola -v -l amp.spl

sprola: sprola.tab.o lex.yy.o symbols.o ast.o \
		codegen.o codegen_std.o codegen_ast.o codegen_ttl.o utils.o
	clang $(ldflags) -o $@  $^ \
		 /usr/local/lib/libLLVM.so \
		 /usr/local/lib/libc++.so

sprola.tab.o: sprola.tab.c  *.h
	clang -g -O0 -c -o sprola.tab.o sprola.tab.c

sprola.tab.c: sprola.y
	bison -d --report=all sprola.y
#	bison -d -v --report=all sprola.y

lex.yy.o: lex.yy.c *.h
	clang -g -O0 -c -o lex.yy.o lex.yy.c

lex.yy.c: sprola.l sprola.y *.h
#	flex -d -T sprola.l or flex -T sprola.l
	flex sprola.l

symbols.o: symbols.c *.h
	clang -Wall -g -O0 -c -o symbols.o symbols.c

ast.o: ast.c  *.h
	clang -Wall -g -O0 -c -o ast.o ast.c

codegen.o: codegen.c *.h
	clang $(cflags) -o codegen.o codegen.c

codegen_std.o: codegen_std.c *.h
	clang $(cflags) -o codegen_std.o codegen_std.c

codegen_ast.o: codegen_ast.c *.h
	clang $(cflags) -o codegen_ast.o codegen_ast.c

codegen_ttl.o: codegen_ttl.c *.h
	clang $(cflags) -o codegen_ttl.o codegen_ttl.c

utils.o: utils.c *.h
	clang $(cflags) -o utils.o utils.c

clean:
	rm -f sprola
	rm -f *.o
	rm -f *.so
	rm -f *.bc
	rm -f lex.yy.c
	rm -f sprola.tab.*
	rm -f sprola.output
	rm -f verbose_dump.ll
	rm -f test_out.wav

tidy:
	clang-tidy  \
		-checks=clang-*,bug*,cert-*,mod*,llvm*,-llvm-header-guard,goog*,fuch*,perf*,port*,read* \
		-header-filter=.* *.c *.h

test:
	cp -r amp.lv2 ~/.lv2/
	lv2file -i exclude/test.wav -o exclude/test_out.wav --connect=1:inputL --connect=1:inputR http://example.org/sprola
