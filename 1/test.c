#include <stdio.h>
#include <stdlib.h>

// void f(int *a) {
//     *a = malloc(sizeof(int) * 5);
//     a[0] = 1;
//     a[1] = 2;
//     a[6] = 3;
// }

int
main(int argc, char *argv[])
{
    printf("TEST: change3\n");
  int i, length = 3;
  int* test = calloc(length, sizeof(int));
  test[0] = 1;
  test[1] = 2;
  test[2] = 3;
  int* test2 = calloc(length, sizeof(int));
  test2[0] = 7;
  printf("%p, %p, %d\n", test, &test , *(test + 1));
  printf("%p, %p, %d\n", test2, &test2, *test2);
  printf("%lu\n", sizeof(test));
  printf("Address of variable 'int *test' on the stack (get by &test): ");
  printf("%p\n", &test);
  printf("Value that stores in that variable (get by test): ");
  printf("%p\n", test);
  printf("Value that we can obtain by this address in heap (get by *test): ");
  printf("%d\n", *test);
  printf("&(*test): %p\n", &(*(test + 1)));
  printf("&(*(test + 1)): %d\n", *&(*(test + 1)));
  free(test);
//   printf("Before:");
//   print(test, length);
//   printf("before change, test address: %p\n", test);
//   change3(&test, length);
//   printf("After:");
//   print(test, length);
//   printf("after change, test address: %p\n", test);

    return 0;
}