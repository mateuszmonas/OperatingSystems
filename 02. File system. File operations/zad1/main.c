#include <bits/types/FILE.h>
#include <sys/resource.h>
#include <time.h>
#include <stdbool.h>
#include <zconf.h>
#include <stdio.h>
#include "stdlib.h"

char* cwd(){
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    return cwd;
}

void generate(char *path, int lines, int length);

void sort(char *path, int lines, int length, bool using_sys);

void copy(char *path1, char *path2, int lines, int length, bool using_sys);

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



int main(){
//    struct rusage *start_usage = calloc(1, sizeof * start_usage);
//    struct rusage *end_usage = calloc(1, sizeof * end_usage);
//    struct timespec *start_time = calloc(1, sizeof *start_time);
//    struct timespec *end_time = calloc(1, sizeof *end_time);
//
//
//
//    clock_gettime(CLOCK_REALTIME, start_time);
//    getrusage(RUSAGE_SELF, start_usage);
//    clock_gettime(CLOCK_REALTIME, end_time);
//    getrusage(RUSAGE_SELF, end_usage)
//    writeResult("result_file", "test_title", start_time, end_time, start_usage, end_usage);
    char *path = calloc(256, sizeof(char));
    snprintf(path, 256, "%s/%s", cwd(), "destination.txt");
    generate(path, 3, 256);
    return 0;
}

void generate(char *path, int lines, int length) {
    FILE *rand = fopen("/dev/urandom", "r");
    FILE *destination = fopen(path, "w");
    char *buffer = malloc(length * sizeof(char));
    for (int i = 0; i < lines; ++i) {
        fread(buffer, sizeof(char), length, rand);
        fwrite(buffer, sizeof(char), length, destination);
    }
    free(buffer);
}
