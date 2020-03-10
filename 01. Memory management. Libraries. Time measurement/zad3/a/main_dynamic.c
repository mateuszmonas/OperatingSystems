#include <sys/resource.h>
#include <time.h>
#include <zconf.h>
#include <dlfcn.h>
#include <bits/types/FILE.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void writeResult(FILE* result_file, char* test_title, struct timespec * start_time, struct timespec * end_time, struct rusage *start_usage, struct rusage *end_usage){
    printf("%s\n", test_title);
    printf("REAL_TIME: %ldns\n", end_time->tv_nsec - start_time->tv_nsec);
    printf("USER_TIME: %ldµs\n", end_usage->ru_utime.tv_usec - start_usage->ru_utime.tv_usec);
    printf("SYSTEM_TIME: %ldµs\n", end_usage->ru_stime.tv_usec - start_usage->ru_stime.tv_usec);


    fprintf(result_file, "%s\n", test_title);
    fprintf(result_file, "REAL_TIME: %ldns\n", end_time->tv_nsec - start_time->tv_nsec);
    fprintf(result_file, "USER_TIME: %ldµs\n", end_usage->ru_utime.tv_usec - start_usage->ru_utime.tv_usec);
    fprintf(result_file, "SYSTEM_TIME: %ldµs\n", end_usage->ru_stime.tv_usec - start_usage->ru_stime.tv_usec);
}

int main(int argc, char **argv) {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    char *path = calloc(256, sizeof(char));
    char *test_title;
    snprintf(path, 256, "%s/%s", cwd, "raport3a.txt");
    FILE *result_file = fopen(path, "a");
    memset(path, 0, 256);

    struct rusage *start_usage = calloc(1, sizeof * start_usage);
    struct rusage *end_usage = calloc(1, sizeof * end_usage);
    struct timespec *start_time = calloc(1, sizeof *start_time);
    struct timespec *end_time = calloc(1, sizeof *end_time);

    snprintf(path, 256, "%s/%s", cwd, "../../zad1/out/libmylib.so");
    void * mylib = dlopen(path, RTLD_LAZY);
    struct block_array *(*create_block_array)(int) = (struct block_array *(*)(int)) dlsym(mylib, "create_block_array");
    void (*add_file_sequence)(struct block_array *, int, char **) = (void (*)(struct block_array *, int, char **)) dlsym(mylib, "add_file_sequence");
    void (*remove_block_array)(struct block_array *) = (void (*)(struct block_array *)) dlsym(mylib, "remove_block_array");
    void (*compare_files)(struct block_array *) = (void (*)(struct block_array *)) dlsym(mylib, "compare_files");
    unsigned int (*save_block)(struct block_array *, char*) = (unsigned int (*)(struct block_array *, char*)) dlsym(mylib, "save_block");
    unsigned int (*remove_block)(struct block_array *, int) = (unsigned int (*)(struct block_array *, int)) dlsym(mylib, "remove_block");
    unsigned int (*remove_operation)(struct block_array *, int, int) = (unsigned int (*)(struct block_array *, int, int)) dlsym(mylib, "remove_operation");
    if (argc < 2 || strcmp(argv[1], "create_table") != 0) {
        // error create table has to be first arg
        return 1;
    }
    int size = atoi(argv[2]);
    struct block_array * array = create_block_array(size);

    for (int i = 3; i < argc; ++i) {
        if(strcmp(argv[i], "compare_pairs")==0){
            int start = ++i;
            int length = 1;
            while (i + 1 < argc &&
                   strcmp(argv[i + 1], "save_block") != 0 &&
                   strcmp(argv[i + 1], "compare_pairs") != 0 &&
                   strcmp(argv[i + 1], "remove_block") != 0 &&
                   strcmp(argv[i + 1], "start_time") != 0 &&
                   strcmp(argv[i + 1], "stop_time") != 0 &&
                   strcmp(argv[i + 1], "remove_operation") != 0) {
                length++;
                i++;
            }
            add_file_sequence(array, length, &argv[start]);
            compare_files(array);
        }
        else if(strcmp(argv[i], "remove_block")==0){
            remove_block(array, atoi(argv[++i]));
        }
        else if(strcmp(argv[i], "save_block")==0){
            save_block(array, argv[++i]);
        }
        else if(strcmp(argv[i], "remove_operation")==0){
            int block_to_delete_from = ++i;
            int operation_to_delete = ++i;
            remove_operation(array, atoi(argv[block_to_delete_from]), atoi(argv[operation_to_delete]));
        }
        else if(strcmp(argv[i], "start_time")==0){
            test_title = argv[++i];
            clock_gettime(CLOCK_REALTIME, start_time);
            getrusage(RUSAGE_SELF, start_usage);
        }
        else if(strcmp(argv[i], "stop_time")==0){
            clock_gettime(CLOCK_REALTIME, end_time);
            getrusage(RUSAGE_SELF, end_usage);
            writeResult(result_file, test_title, start_time, end_time, start_usage, end_usage);
        }
    }
    remove_block_array(array);
    dlclose(mylib);
    fclose(result_file);
    return 0;
}