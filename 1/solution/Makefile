run: build
	./sol.o -c 3 test1.txt test2.txt test3.txt test4.txt test5.txt test6.txt

build: solution.c
	gcc solution.c libcoro.c -O0 -g -Wall -o sol.o