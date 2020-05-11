#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

struct PGM_data{
    long width;
    long height;
    long max_value;
    long **values;
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

int create_pgm_file(struct PGM_data *pgm_data, char *pgm_file){

}

long histogram_sign(struct PGM_data *pgm_data, long thread_number, long thread_count) {
    long value = 0;
    for (long i = 0; i < pgm_data->width; ++i) {
        for (long j = 0; j < pgm_data->height; ++j) {
            if (thread_number * (pgm_data->max_value / thread_count) <= pgm_data->values[j][i] &&
                pgm_data->values[j][i] < thread_number == (thread_count - 1) ? pgm_data->max_value :
                (thread_number + 1) * (pgm_data->max_value / thread_count)) {
                value += pgm_data->values[j][i];
            }

        }
    }
    return value;
}

long histogram_block(struct PGM_data *pgm_data, long thread_number, long thread_count) {
    long value = 0;
    long start = thread_number * (long) ceil((double) pgm_data->width / (double) thread_count);
    long end = (thread_number + 1) * (long) ceil((double) pgm_data->width / (double) thread_count);
    for (long i = start; i < end; ++i) {
        for (long j = 0; j < pgm_data->height; ++j) {
                value += pgm_data->values[j][i];
        }
    }
    return value;
}

long histogram_interval(struct PGM_data *pgm_data, long thread_number, long thread_count) {
    long value = 0;
    long start = thread_number;
    long end = pgm_data->width;
    long interval = thread_count;
    for (long i = start; i < end; i += interval) {
        for (long j = 0; j < pgm_data->height; ++j) {
            value += pgm_data->values[j][i];
        }
    }
    return value;
}

struct histogram_func_args{
    struct PGM_data *pgm_data;
    long thread_number;
    long thread_count;
}args;

int main(int argc, char **argv){
    if (argc < 5) {
        fprintf(stderr, "not enough arguments");
    }
    long thread_count = strtol(argv[1], NULL, 10);
    char *input_file_name = argv[3];
    char *output_file_name = argv[4];
    long (*histogram_function)(struct PGM_data *, long, long);

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
//    struct histogram_func_args args = {.pgm_data = &pgm_data, .thread_count = thread_count};
    args.pgm_data = &pgm_data;
    args.thread_count = thread_count;
    pthread_t threads[thread_count];
    for (int i = 0; i < thread_count; ++i) {
        args.thread_number = i;
        pthread_create(&threads[i], NULL, (void *) histogram_function, &args);
    }
    long sum = 0;
    for (int i = 0; i < thread_count; ++i) {
        long result;
        pthread_join(threads[i], (void *) &result);
        sum += result;
    }
    printf("%ld", sum);
}