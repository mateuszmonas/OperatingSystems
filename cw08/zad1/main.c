#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

struct PGM_data{
    long width;
    long height;
    long max_value;
    long **values;
};

struct histogram_func_args{
    struct PGM_data *pgm_data;
    long thread_number;
    long thread_count;
    long *results;
};

int read_pgm_file(struct PGM_data *pgm_data, char *pgm_file) {
    int buffer_size = 256;
    char *buffer = malloc(buffer_size * sizeof(char));
    FILE *input_file = fopen(pgm_file, "r");
    fgets(buffer, buffer_size, input_file);
    if (strcmp(buffer, "P2") == 0) {
        free(buffer);
        fclose(input_file);
        return 1;
    }
    do {
        fgets(buffer, buffer_size, input_file);
    } while (buffer[0] == '#');
    pgm_data->width = strtol(strtok(buffer, " "), NULL, 10);
    pgm_data->height = strtol(strtok(NULL, " "), NULL, 10);
    fgets(buffer, buffer_size, input_file);
    pgm_data->max_value = strtol(strtok(buffer, " "), NULL, 10);
    if (errno != 0) {
        return errno;
    }
    pgm_data->values = malloc(pgm_data->height * sizeof(long *));
    for (int i = 0; i < pgm_data->height; ++i) {
        pgm_data->values[i] = malloc(pgm_data->width * sizeof(long *));
        for (int j = 0; j < pgm_data->width; ++j) {
            fscanf(input_file, "%s", buffer);
            pgm_data->values[i][j] = strtol(buffer, NULL, 10);
        }
    }
    free(buffer);
    fclose(input_file);
    return 0;
}

struct timespec histogram_sign(struct histogram_func_args* arguments) {
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    struct PGM_data *pgm_data = arguments->pgm_data;
    long thread_number = arguments->thread_number;
    long thread_count = arguments->thread_count;
    long *results = arguments->results;
    for (long i = 0; i < pgm_data->width; ++i) {
        for (long j = 0; j < pgm_data->height; ++j) {
            if(thread_number * ((pgm_data->max_value + 1) / thread_count) <= pgm_data->values[j][i] && pgm_data->values[j][i] < (thread_number + 1) * ((pgm_data->max_value + 1) / thread_count))
                results[pgm_data->values[j][i]]++;
        }
    }
    return start_time;
}

struct timespec histogram_block(struct histogram_func_args* arguments) {
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    struct PGM_data *pgm_data = arguments->pgm_data;
    long thread_number = arguments->thread_number;
    long thread_count = arguments->thread_count;
    long *results = arguments->results;
    long start = thread_number * (long) ceil((double) pgm_data->width / (double) thread_count);
    long end = (thread_number + 1) * (long) ceil((double) pgm_data->width / (double) thread_count);
    for (long i = start; i < end; ++i) {
        for (long j = 0; j < pgm_data->height; ++j) {
            results[pgm_data->values[j][i]]++;
        }
    }
    return start_time;
}

struct timespec histogram_interval(struct histogram_func_args* arguments) {
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    struct PGM_data *pgm_data = arguments->pgm_data;
    long thread_number = arguments->thread_number;
    long thread_count = arguments->thread_count;
    long *results = arguments->results;
    long start = thread_number;
    long end = pgm_data->width;
    long interval = thread_count;
    for (long i = start; i < end; i += interval) {
        for (long j = 0; j < pgm_data->height; ++j) {
            results[pgm_data->values[j][i]]++;
        }
    }
    return start_time;
}

void print_time(struct timespec *start_time) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);
    long time = (end_time.tv_sec - start_time->tv_sec) * 1000000000 + (end_time.tv_nsec - start_time->tv_nsec);
    printf("time: %ld.%09lds\n", time / 1000000000, time % 1000000000);
}

int main(int argc, char **argv){
    if (argc < 5) {
        perror("not enough arguments");
    }
    long thread_count = strtol(argv[1], NULL, 10);
    char *input_file_name = argv[3];
    char *output_file_name = argv[4];
    struct timespec (*histogram_function)(struct histogram_func_args *);

    if (strcmp(argv[2], "sign") == 0) {
        histogram_function = &histogram_sign;
    }else if (strcmp(argv[2], "block") == 0) {
        histogram_function = &histogram_block;
    }else if (strcmp(argv[2], "interval") == 0) {
        histogram_function = &histogram_interval;
    }else {
        fprintf(stderr, "wrong division method");
    }
    struct PGM_data pgm_data;
    read_pgm_file(&pgm_data, input_file_name);

    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);

    long results[thread_count][pgm_data.max_value + 1];
    for (int k = 0; k < thread_count; ++k) {
        for (int i = 0; i < pgm_data.max_value + 1; ++i) {
            results[k][i] = 0;
        }
    }
    struct histogram_func_args *args = malloc(thread_count* sizeof(struct histogram_func_args));
    pthread_t threads[thread_count];
    for (int i = 0; i < thread_count; ++i) {
        args[i].pgm_data = &pgm_data;
        args[i].thread_number = i;
        args[i].thread_count = thread_count;
        args[i].results = results[i];
        pthread_create(&threads[i], NULL, (void *) histogram_function, &args[i]);
    }
    for (int i = 0; i < thread_count; ++i) {
        struct timespec time;
        pthread_join(threads[i], (void *) &time);
        printf("%ld ", threads[i]);
        print_time(&time);
    }
    long *histogram = results[0];
    for (int i = 1; i < thread_count; ++i) {
        for (int j = 0; j < pgm_data.max_value + 1; ++j) {
            histogram[j] += results[i][j];
        }
    }
    printf("total ");
    print_time(&start_time);
    for (int l = 0; l < pgm_data.max_value + 1; ++l) {
        printf("%ld ", histogram[l]);
    }
}