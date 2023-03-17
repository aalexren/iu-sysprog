test: clean all
	./sol.o

all: clean test.o userfs.o
	gcc test.o userfs.o -o sol.o

test.o: test.c
	gcc -c test.c -o test.o -I ../../utils/

userfs.o: userfs.c
	gcc -c userfs.c -o userfs.o

clean:
	rm -f *.o *.out