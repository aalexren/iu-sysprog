#include <stdio.h>
#include <stdlib.h>


static void
me_merge(int *array, int lhs, int mid, int rhs) {
    int lsize = mid - lhs + 1;
    int rsize = rhs - mid;

    int *larr = (int *)calloc(lsize, sizeof(int));
    int *rarr = (int *)calloc(rsize, sizeof(int));

    if (larr == NULL || rarr == NULL) {
        printf("Can't allocate memory at me_merge!");
        exit(-1);
    }

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

    free(rarr);
    free(larr);
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


/**
 * @brief Read file and returns number of scanned numbers.
 * 
 * @param fname file name to read from
 * @param array array to save numbers
 * @return int number of scanned numbers
 */
static int
read_file(char *fname, int **array)
{
    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        printf("Can't open file %s at read_file!", fname);
    }

    /* Make temporary array to read in numbers. */
    int *temp = (int *)calloc(1, sizeof(int));
    /* Assuming length of array. */
    int count = 0;
    /* Size of allocated memory. */
    int allocated = 1;

    for (int i = 0; ;++i) {
        /* Read number to array cell. */
        int status = fscanf(file, "%d", &temp[i]);
        if (status == EOF) {
            break;
        }
        count += 1;
        /* Increase memory block twice if needed. */
        if (count == allocated) {
            allocated = allocated * 2;
            int *new_temp = (int *)realloc(temp, sizeof(int) * allocated);
            if (new_temp == NULL) {
                printf("Can't re-allocate memory at scan_file!");
                exit(-1);
            }
            else {
                temp = new_temp;
            }
        }
    }

    int *new_temp = (int *)realloc(temp, sizeof(int) * count);
    if (new_temp == NULL) {
        printf("Can't re-allocate memory at scan_file!");
        exit(-1);
    }
    else {
        temp = new_temp;
    }
    
    /* Save numbers to passed array. */
    *array = temp;

    /* Do not forget to close file. */
    fclose(file);

    return count;
}

/**
 * @brief Write numbers in sorted order into file.
 * 
 * @param fname file name
 * @param array 2D array with numbers
 * @param sizes 1D array with lengths of rows
 * @param size length of 1D array
 */
static void
write_result_to_file(char *fname, int **array, int *sizes, int size)
{
    FILE *file = fopen(fname, "w");
    if (file == NULL) {
        printf("Can't open file %s at read_file!", fname);
    }

    /**
     * We will decrease size every time when one of counter becomes 0.
     * Since it zero means we wrote all numbers.
     * Hence, we don't need count total number of numbers.
     */
    int zeros = size;

    int *iters = (int *)calloc(size, sizeof(int));
    if (iters == NULL) {
        printf("Can't allocate memory at write_result_to_file!");
        exit(-1);
    }

    int min = array[0][0];
    for (; zeros > 0; ) {
        int min_idx = -1;

        /* Take first available to find minimum. */
        for (int i = 0; i < size; ++i) {
            if (iters[i] < sizes[i]) {
                min = array[i][iters[i]];
                min_idx = i;
            }
        }
        if (min_idx == -1) {
            break;
        }
        for (int i = 0; i < size; ++i) {
            /**
             * Check with all available (we didn't pass)
             * first values in every row.
             */
            if (iters[i] < sizes[i] && min > array[i][iters[i]]) {
                min = array[i][iters[i]];
                min_idx = i;
            }
        }
        iters[min_idx] += 1;

        /* If no numbers left for this row decrease number of total available rows. */
        if (iters[min_idx] >= sizes[min_idx]) {
            zeros -= 1;
        }

        /* Write number to file. */
        fprintf(file, "%d ", min);
    }

    free(iters);
    fclose(file);
}

/**
 * @brief Print 2D array.
 * 
 * @param array 2D array to print
 * @param sizes 1D array with lengths of rows
 * @param size length of 1D array
 */
static void
print2d(int **array, int *sizes, int size)
{
    printf("\n");
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < sizes[i]; j += 976) {
            printf("%d ", array[i][j]);
        }
        printf("\n------------------\n");
    }
    printf("\n");
}

int
main(int argc, char **argv)
{
    /* Number of files have to be sorted. */
    int files_count = argc - 1;
    /* 2D array: file <-> corresponding numbers. */
    int **numbers = (int **)calloc(files_count, sizeof(int *));
    /* Array with count of numbers for every file. */
    int *c_numbers = (int *)calloc(files_count, sizeof(int));
    /* Destination file name. */
    char *dest_file = "output.txt";

    /* Read files and save numbers to 2D array. */
    for (int i = 0; i < files_count; ++i) {
        printf("Pointer before memory allocation: %p\n", numbers[i]);
        c_numbers[i] = read_file(argv[i + 1], &numbers[i]);
        printf("Number of numbers in %s: %d\n", argv[i + 1], c_numbers[i]);
        me_mergesort(numbers[i], 0, c_numbers[i] - 1);
        printf("Pointer after memory allocation: %p\n", numbers[i]);
    }

    /* Check if everything is ok. */
    print2d(numbers, c_numbers, files_count);

    /* Merge to one file. */
    write_result_to_file(dest_file, numbers, c_numbers, files_count);
    
    /* Do not forget to free allocated memory. */
    for (int i = 0; i < files_count; ++i) {
        if (numbers[i] != NULL) {
            free(numbers[i]);
        }
    }
    free(numbers);
    free(c_numbers);

    return 0;
}