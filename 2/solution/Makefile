run: build
	./sol.o

build: solution.c
	gcc solution.c parser.c command.c stack.c pair.c -O0 -g -Wall -o sol.o \
	-I../../utils/heap_help/ -L. -lheap -fsanitize=address -static-libsan
