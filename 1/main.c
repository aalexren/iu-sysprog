#include <stdio.h>
#include <stdlib.h>


static void
me_merge(int *array, int lhs, int mid, int rhs) {
    int lsize = mid - lhs + 1;
    int rsize = rhs - mid;

    int larr[lsize];
    int rarr[rsize];

    for (int i = 0; i < lsize; ++i) {
        larr[i] = array[lhs + i];
    }
    for (int i = 0; i < rsize; ++i) {
        rarr[i] = array[mid + i + 1];
    }

    int i = 0, j = 0, k = lhs;
    while (i < lsize && j < rsize) {
        if (larr[i] < rarr[j]) {
            array[k] = larr[i];
            i++;
        }
        else {
            array[k] = rarr[j];
            j++;
        }
        k++;
    }

    while (i < lsize) {
        array[k] = larr[i];
        i++;
        k++;
    }
    
    while (j < rsize) {
        array[k] = rarr[j];
        j++;
        k++;
    }
}

static void
me_mergesort(int *array, int lhs, int rhs)
{
    if (lhs < rhs) {
        int mid = (lhs + rhs) / 2;
        me_mergesort(array, lhs, mid);
        me_mergesort(array, mid + 1, rhs);
        me_merge(array, lhs, mid, rhs);
    }
}

// static void
// read_file(const char *fname, int **ret_array, size_t *ret_size)
// {
//     FILE* file;
    
//     if ((file = fopen(fname, "r")) == NULL) {
//         printf("Can't open file.");
//         exit(-1);
//     }

//     *ret_array = (int *)malloc(sizeof(int) * 2);
//     size_t free_cells = 2;
//     if (*ret_array == NULL) {
//         printf("Can't allocate memory.");
//         exit(-1);
//     }

//     for (int i = 0; fscanf(file, "%d", &(*ret_array)[i]) != EOF; i++) {
//         if (free_cells / 2 <= i) {
//             if (realloc(*ret_array, sizeof(free_cells * 2)) == NULL) {
//                 printf("Can't reallocate memory.");
//                 free(*ret_array);
//                 exit(-1);
//             }
//             free_cells *= 2;
//         }
//         (*ret_size)++;
//     }

//     /* Print to debug */
//     // for (int i = 0; i < *ret_size; ++i) {
//     //     printf("%d\n", (*ret_array)[i]);
//     // }
//     // printf("Length of array: %zu, Size of array: %zu\n", *ret_size, free_cells);
    
//     fclose(file);
// }

// static int*
// read_file(const char *fname, size_t *ret_size)
// {
//     FILE* file;

//     if ((file = fopen(fname, "r")) == NULL) {
//         printf("Can't open file.");
//         exit(-1);
//     }

//     size_t free_cells = 2;
//     int *ret_array = calloc(free_cells, sizeof(int));
//     if (ret_array == NULL) {
//         printf("Can't allocate memory.");
//         exit(-1);
//     }

//     for (int i = 0; fscanf(file, "%d", &ret_array[i]) != EOF; i++) {
//         // if (free_cells / 2 <= i) {
//         //     free_cells *= 2;
//         //     if (realloc(ret_array, sizeof(int) * free_cells) == NULL) {
//         //         printf("Can't reallocate memory.");
//         //         free(ret_array);
//         //         exit(-1);
//         //     }
//         // }
//         (*ret_size)++;
//     }

//     fclose(file);

//     return ret_array;
// }

int
main(int argc, char **argv)
{
    argc = 3;
    int **nums = (int **)calloc(argc - 1, sizeof(int*));
    printf("I'm working....%d\n", argc);
    char *argvv[] = {
        "./main.o",
        "test1.txt",
        "test2.txt"
    };
    nums[0] = *nums;
    nums[1] = *(nums + 1);

    printf("Address of variable on the stack (get by &<name>): ");
    printf("%p\n", &nums);
    printf("Value that stores in that variable (get by <name>): ");
    printf("%p\n", nums);
    printf("Value that we can obtain by this address in heap (get by *<name>): ");
    printf("%p\n", *nums);

    size_t count = 0;
    for (int i = 0; i < argc - 1; ++i) {
        printf("I'm working in the loop %d...\n", i);
        // read_file(argv[i + 1], &(nums[i]), &count);
        nums[i] = read_file(argvv[i + 1], &count);
        printf("Address of variable on the stack (get by &<name>): ");
        printf("%p\n", &nums);
        printf("Value that stores in that variable (get by <name>): ");
        printf("%p\n", nums);
        printf("Value that we can obtain by this address in heap (get by *<name>): ");
        printf("%p\n", *(nums[i]));

        printf("%zu\n", count);
        // me_mergesort(nums[i], 0, count - 1);
        printf("-----------------\n");
        for (int j = 0; j < count; ++j) {
            printf("%d\n", nums[i][j]);
        }
        count = 0;
    }

    // int *nums = NULL;
    // read_file("test1.txt", &nums, &count);

    // me_mergesort(nums, 0, count - 1);
    
    // for (int i = 0; i < count; ++i) {
    //     printf("%d\n", nums[i]);
    // }

    // printf("%zu", *nums);
    // for (int i = 0; i < argc - 1; ++i) {
    //     free(nums[i]);
    // }
    // free(nums);

    return 0;
}