#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "libcoro.h"


#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})


/* BLOCK CONSTANTS */
const int CORO_COUNT = 6;
/* BLOCK CONSTANTS */

/* BLOCK PREDEFINED */
struct coro_args {
    char *coro_name;
    long long switch_count;

    int **array;
    int array_length;
    struct coro_file **files;
    int files_length;
    int *arrays_length; /* keeps length of every read array */
    
    struct timespec t_invoked; /* last invoked time */
    struct timespec t_summary;
};

/**
 * Calculates time elapsed by formula in microseconds:
 * `current - past`
 * 
 * @param last older time
 * @param curr newer time
 * @return struct timespec 
 */
struct timespec 
elapsed_time(struct timespec *last, struct timespec *curr)
{
    struct timespec elapsed;
    elapsed.tv_sec = curr->tv_sec - last->tv_sec;
    elapsed.tv_nsec = curr->tv_nsec - last->tv_nsec;

    if (elapsed.tv_nsec < 0) {
        elapsed.tv_nsec += 1000000000;
        elapsed.tv_sec -= 1;
    }

    return elapsed;
}

long
timespec_to_microsec(struct timespec *t)
{
    if (t->tv_nsec < 0) {
        t->tv_nsec += 1000000000;
        t->tv_sec -= 1;
    }
    
    return t->tv_sec * 1000000 + t->tv_nsec / 1000;
}

/**
 * @brief Add to timespec structures field by field.
 * Guarantee that there is no overflow.
 */
void
add_timespec(struct timespec *t, struct timespec *diff)
{
    t->tv_sec += diff->tv_sec;
    t->tv_nsec += diff->tv_nsec;

    if (t->tv_nsec < 0) {
        t->tv_nsec += 1000000000;
        t->tv_sec -= 1;
    }
}

static int
read_file(char *fname, int **array, struct coro_args* args);

// static void
// print2d(int **array, int *sizes, int size);
/* BLOCK PREDEFINED */

/* Struct to keep information if file was locked and performed by coroutine. */
struct coro_file {
    char *name;
    bool processed;
    int index;
};

/* Get any available file to process by coroutine. */
struct coro_file* get_file(struct coro_file** files, int size)
{
    for(int i = 0; i < size; ++i) {
        if (!files[i]->processed) {
            return files[i];
        }
    }

    return NULL;
}

static void
me_merge(int *array, int lhs, int mid, int rhs, char *name, struct coro_args* args) {
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
me_mergesort(int *array, int lhs, int rhs, char *name, struct coro_args* args)
{
    clock_gettime(CLOCK_MONOTONIC, &args->t_invoked);
    struct timespec t_time, elapsed;
    struct coro *this = coro_this();

    for (int curr_size = 1; curr_size <= rhs; curr_size = 2 * curr_size)
    {
        /* Pick starting point of different subarrays of current size. */
        for (int left_start = 0; left_start < rhs-1; left_start += 2 * curr_size)
        {
            /** 
             * Find ending point of left subarray. 
             * mid+1 is starting point of right
            */
            int mid = min(left_start + curr_size - 1, rhs-1);

            int right_end = min(left_start + 2 * curr_size - 1, rhs-1);

            /* Merge Subarrays arr[left_start...mid] & arr[mid+1...right_end] */
            me_merge(array, left_start, mid, right_end, name, args);
            
            // clock_gettime(CLOCK_MONOTONIC, &t_time);
            // elapsed = elapsed_time(&args->t_invoked, &t_time);
            // add_timespec(&args->t_summary, &elapsed);
            // coro_yield();
            // clock_gettime(CLOCK_MONOTONIC, &args->t_invoked);
        }
        clock_gettime(CLOCK_MONOTONIC, &t_time);
        elapsed = elapsed_time(&args->t_invoked, &t_time);
        add_timespec(&args->t_summary, &elapsed);
        coro_yield();
        clock_gettime(CLOCK_MONOTONIC, &args->t_invoked);
    }

    args->switch_count = coro_switch_count(this);
    clock_gettime(CLOCK_MONOTONIC, &t_time);
    elapsed = elapsed_time(&args->t_invoked, &t_time);
    add_timespec(&args->t_summary, &elapsed);
}

static int
intermediate(void *args)
{
    struct coro_args *temp = args;
    struct timespec t_time, elapsed;
    printf("Start coroutine %s\n", temp->coro_name);
    for(;;) {
        clock_gettime(CLOCK_MONOTONIC, &temp->t_invoked);

        struct coro_file* file = get_file(temp->files, temp->files_length);
        if (file == NULL) {
            printf("Exit %s\n", temp->coro_name);
            break;
        }
        file->processed = true;
        printf("Reading file by %s...\n", temp->coro_name);
        int array_len = read_file(file->name, &temp->array[file->index], temp);
        temp->arrays_length[file->index] = array_len;
        
        clock_gettime(CLOCK_MONOTONIC, &t_time);
        elapsed = elapsed_time(&temp->t_invoked, &t_time);
        add_timespec(&temp->t_summary, &elapsed);
        coro_yield();
        clock_gettime(CLOCK_MONOTONIC, &temp->t_invoked);

        printf("Sorting file by %s...\n", temp->coro_name);
        me_mergesort(temp->array[file->index], 0, array_len, temp->coro_name, temp);
        printf("Get next file by %s...\n", temp->coro_name);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_time);
    elapsed = elapsed_time(&temp->t_invoked, &t_time);
    add_timespec(&temp->t_summary, &elapsed);
    return 0;
}

/**
 * @brief Read file and returns number of scanned numbers.
 * 
 * @param fname file name to read from
 * @param array array to save numbers
 * @return int number of scanned numbers
 */
static int
read_file(char *fname, int **array, struct coro_args* args)
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
            temp = new_temp;
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
// static void
// print2d(int **array, int *sizes, int size)
// {
//     printf("\n");
//     for (int i = 0; i < size; ++i) {
//         for (int j = 0; j < sizes[i]; j += 976) {
//             printf("%d ", array[i][j]);
//         }
//         printf("\n------------------\n");
//     }
//     printf("\n");
// }

int
main(int argc, char **argv)
{
    struct timespec start_time, end_time, time_elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    /* Number of files have to be sorted. */
    int files_count = argc - 1;
    /* Read number of coroutines should be started. */
    int coro_param = CORO_COUNT;
    if (argc >= 3 && strcmp(argv[1], "-c") == 0) {
        coro_param = atoi(argv[2]);
        files_count -= 2;
    }

    /* 2D array: file <-> corresponding numbers. */
    int **numbers = (int **)calloc(files_count, sizeof(int *));
    /* Array with count of numbers for every file. */
    int *c_numbers = (int *)calloc(files_count, sizeof(int));
    /* Destination file name. */
    char *dest_file = "output.txt";

    printf("Files count: %d\n", files_count);
    struct coro_file **coro_files = (struct coro_file**)calloc(files_count, sizeof(struct coro_file*));
    for (int i = 0; i < files_count; ++i) {
        int start_idx = argc - files_count;
        struct coro_file* f = (struct coro_file*)malloc(sizeof(*f));
        f->name = argv[start_idx + i];
        f->processed = false;
        f->index = i;
        coro_files[i] = f;
    }

    /* Initialize our coroutine global cooperative scheduler. */
    // clock_gettime(CLOCK_MONOTONIC, &start_time);
	coro_sched_init();
    struct coro_args* coro_args_list[coro_param];
	/* Start several coroutines. */
	for (int i = 0; i < coro_param; ++i) {
		/*
		 * The coroutines can take any 'void *' interpretation of which
		 * depends on what you want. Here as an example I give them
		 * some names.
		 */
		char name[16];
		sprintf(name, "coro_%d", i);
		/*
		 * I have to copy the name. Otherwise all the coroutines would
		 * have the same name when they finally start.
		 */
        struct coro_args *temp = malloc(sizeof(*temp));
        temp->array = numbers;
        temp->array_length = files_count;
        temp->files = coro_files;
        temp->files_length = files_count;
        temp->arrays_length = c_numbers;
        temp->coro_name = strdup(name);
        clock_gettime(CLOCK_MONOTONIC, &temp->t_invoked);
        clock_gettime(CLOCK_MONOTONIC, &temp->t_summary);
        temp->t_summary.tv_sec = 0;
        temp->t_summary.tv_nsec = 0;
        temp->switch_count = 0;
        coro_args_list[i] = temp;
        coro_new(intermediate, temp);
	}

	/* Wait for all the coroutines to end. */
	struct coro *c;
	for (; (c = coro_sched_wait()) != NULL;) {
		/*
		 * Each 'wait' returns a finished coroutine with which you can
		 * do anything you want. Like check its exit status, for
		 * example. Don't forget to free the coroutine afterwards.
		 */
		coro_delete(c);
	}
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    /* Print results and free memory. */
    for (int i = 0; i < coro_param; ++i) {
        printf("Coro name: %s,\tSwitch count: %lld,\texecution time in micro sec: %lu\n", 
                coro_args_list[i]->coro_name,
                coro_args_list[i]->switch_count,
                timespec_to_microsec(&coro_args_list[i]->t_summary)
        );
        free(coro_args_list[i]->coro_name);
        free(coro_args_list[i]);
    }
	/* All coroutines have finished. */
    time_elapsed = elapsed_time(&start_time, &end_time);
    printf("Exectuion time before merging: %lu micro sec\n", 
            timespec_to_microsec(&time_elapsed));

    /* Check if everything is ok. */
    // print2d(numbers, c_numbers, files_count);

    /* Merge to one file. */
    write_result_to_file(dest_file, numbers, c_numbers, files_count);

    /* Do not forget to free allocated memory. */
    for (int i = 0; i < files_count; ++i) {
        if (numbers[i] != NULL) {
            free(numbers[i]);
        }
        free(coro_files[i]);
    }
    free(numbers);
    free(c_numbers);
    free(coro_files);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    time_elapsed = elapsed_time(&start_time, &end_time);
    printf("Total time of program execution: %lu micro sec\n", 
            timespec_to_microsec(&time_elapsed));

    return 0;
}