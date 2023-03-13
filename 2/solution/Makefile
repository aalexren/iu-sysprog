run: build
	./sol.o

build: clean solution.c
	gcc solution.c parser.c command.c stack.c pair.c \
	heap_help/heap_help.c -I$(PWD)/heap_help \
	-O0 -g -Wall -o sol.o
# -fsanitize=address -static-libsan

clean:
	rm -rf *.o