test: clean test.o userfs.o
	gcc test.o userfs.o -Wall -O0 -o test.out && ./test.out

test.o: test.c
	gcc -c test.c -Wall -O0 -o test.o -I ../../utils/

userfs.o: userfs.c
	gcc -c userfs.c -Wall -O0 -o userfs.o

clean:
	rm -f *.o *.out