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

static void
read_file(const char *fname, int **ret_array, size_t *ret_size)
{
    FILE* file;
    
    if ((file = fopen(fname, "r")) == NULL) {
        printf("Can't open file.");
        exit(-1);
    }

    int *numbers = malloc(sizeof(int) * 2);
    size_t free_cells = 2;
    if (numbers == NULL) {
        printf("Can't allocate memory.");
        exit(-1);
    }

    for (int i = 0; fscanf(file, "%d", &numbers[i]) != EOF; i++) {
        // printf("idx %d: %d\n", i, numbers[i]);
        if (free_cells / 2 <= i) {
            if (realloc(numbers, sizeof(free_cells * 2)) == NULL) {
                printf("Can't reallocate memory.");
                free(numbers);
                exit(-1);
            }
            free_cells *= 2;
        }
        (*ret_size)++;
    }

    *ret_array = numbers;

    // for (int i = 0; i < count; ++i) {
    //     printf("%d\n", numbers[i]);
    // }
    printf("Length of array: %zu, Size of array: %zu\n", *ret_size, free_cells);
    
    fclose(file);
}

int
main(int argc, char **argv)
{
    int *nums = NULL;
    size_t count = 0;
    read_file("test1.txt", &nums, &count);

    me_mergesort(nums, 0, count - 1);
    
    for (int i = 0; i < count; ++i) {
        printf("%d\n", nums[i]);
    }

    free(nums);

    return 0;
}