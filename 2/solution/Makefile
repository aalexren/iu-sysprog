run: build
	./sol.o

build: solution.c
	gcc solution.c parser.c command.c stack.c pair.c -O0 -g -Wall -o sol.o \
	-I$(PWD)/heap_help -L$(PWD)/heap_help -lheap 
# -fsanitize=address -static-libsan
