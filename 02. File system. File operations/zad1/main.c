#include <bits/types/FILE.h>
#include <zconf.h>
#include <stdio.h>
#include <fcntl.h>
#include "stdlib.h"
#include "string.h"
#include <sys/times.h>

char* get_file_path(char* file_name){
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    char *path = calloc(256, sizeof(char));
    snprintf(path, 256, "%s/%s", cwd, file_name);
    free(cwd);
    return path;
}

void generate(char *path, int lines, int length);

void sort_sys(char *path, int lines, int length);
void sort_lib(char *path, int lines, int length);

void copy_sys(char *source_path, char *destination_path, int lines, int length);
void copy_lib(char *source_path, char *destination_path, int lines, int length);

void write_result(FILE* result_file, char* test_title, clock_t st_time, clock_t en_time, struct tms st_cpu, struct tms en_cpu){
    printf("%s\n", test_title);
    printf("REAL_TIME: %ld\n", en_time - st_time);
    printf("USER_TIME: %ld\n", en_cpu.tms_utime - st_cpu.tms_utime);
    printf("SYSTEM_TIME: %ld\n", en_cpu.tms_stime - st_cpu.tms_stime);


    fprintf(result_file, "%s\n", test_title);
    fprintf(result_file, "REAL_TIME: %ld\n", en_time - st_time);
    fprintf(result_file, "USER_TIME: %ld\n", en_cpu.tms_utime - st_cpu.tms_utime);
    fprintf(result_file, "SYSTEM_TIME: %ld\n", en_cpu.tms_stime - st_cpu.tms_stime);
}

int main(int argc, char** argv){
    if (argc < 2) {
        return 1;
    }

    clock_t st_time;
    clock_t en_time;
    struct tms st_cpu;
    struct tms en_cpu;

    char *test_title = calloc(256, sizeof(char));
    char *result_path = get_file_path("wynik.txt");
    FILE* result_file = fopen(result_path, "a");

    int i = 1;
    if (strcmp(argv[i], "generate") == 0) {
        char *file = get_file_path(argv[++i]);
        int lines = atoi(argv[++i]);
        int length = atoi(argv[++i]);
        generate(file, lines, length);
    } else if (strcmp(argv[i], "sort") == 0) {
        char *file = get_file_path(argv[++i]);
        int lines = atoi(argv[++i]);
        int length = atoi(argv[++i]);
        sprintf(test_title, "sort %d %d %s", lines, length, argv[argc - 1]);
        st_time = times(&st_cpu);
        strcmp(argv[argc - 1], "sys") == 0 ? sort_sys(file, lines, length) : sort_lib(file, lines, length);
        en_time = times(&en_cpu);\
        write_result(result_file, test_title, st_time, en_time, st_cpu, en_cpu);
    } else if (strcmp(argv[i], "copy") == 0) {
        char *file1 = get_file_path(argv[++i]);
        char *file2 = get_file_path(argv[++i]);
        int lines = atoi(argv[++i]);
        int length = atoi(argv[++i]);
        sprintf(test_title, "copy %d %d %s", lines, length, argv[argc - 1]);
        st_time = times(&st_cpu);
        strcmp(argv[argc - 1], "sys") == 0 ? copy_sys(file1, file2, lines, length) : copy_lib(file1, file2, lines, length);
        en_time = times(&en_cpu);\
        write_result(result_file, test_title, st_time, en_time, st_cpu, en_cpu);
    }
    fclose(result_file);
    free(test_title);
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
    fclose(destination);
    fclose(rand);
}

void copy_sys(char *source_path, char *destination_path, int lines, int length){
    int source = open(source_path, O_RDONLY);
    int destination = open(destination_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    char *buffer = calloc(length, sizeof(char));
    for (int i = 0; i < lines; ++i) {
        read(source, buffer, length);
        write(destination, buffer, length);
    }
    free(buffer);
    close(destination);
    close(source);
}

void copy_lib(char *source_path, char *destination_path, int lines, int length){
    FILE *source = fopen(source_path, "r");
    FILE *destination = fopen(destination_path, "w");
    char *buffer = calloc(length, sizeof(char));
    for (int i = 0; i < lines; ++i) {
        fread(buffer, sizeof(char), length, source);
        fwrite(buffer, sizeof(char), length, destination);
    }
    free(buffer);
    fclose(destination);
    fclose(source);
}

void swap_lib(FILE* file, char* i_line, char* j_line, int i, int j, int length){
    fseek(file, i * length * sizeof(char), SEEK_SET);
    fwrite(j_line, sizeof(char), length, file);
    fseek(file, j * length * sizeof(char), SEEK_SET);
    fwrite(i_line, sizeof(char), length, file);
}

void sort_lib_helper(FILE* file, int start, int end, int length){
    if (end < start) {
        return;
    }
    char *pivot = calloc(length, sizeof(char));
    fseek(file, end * length * sizeof(char), SEEK_SET);
    fread(pivot, sizeof(char), length, file);

    char *j_line = calloc(length, sizeof(char));

    int i = start;

    for (int j = start; j < end; ++j) {
        fseek(file, j * length * sizeof(char), SEEK_SET);
        fread(j_line, sizeof(char), length, file);

        if (strcmp(j_line, pivot) < 0) {
            fseek(file, i * length * sizeof(char), SEEK_SET);
            fread(pivot, sizeof(char), length, file);
            swap_lib(file, pivot, j_line, i, j, length);
            i++;
            fseek(file, end * length * sizeof(char), SEEK_SET);
            fread(pivot, sizeof(char), length, file);
        }
    }
    fseek(file, i * length * sizeof(char), SEEK_SET);
    fread(j_line, sizeof(char), length, file);
    swap_lib(file, j_line, pivot, i, end, length);
    free(pivot);
    free(j_line);
    sort_lib_helper(file, start, i - 1, length);
    sort_lib_helper(file, i + 1, end, length);
}

void sort_lib(char *path, int lines, int length){
    FILE *file = fopen(path, "r+");
    sort_lib_helper(file, 0, lines - 1, length);
    fclose(file);
}

void swap_sys(int file, char* i_line, char* j_line, int i, int j, int length){
    lseek(file, i * length * sizeof(char), SEEK_SET);
    write(file, j_line, length);
    lseek(file, j * length * sizeof(char), SEEK_SET);
    write(file, i_line, length);
}

void sort_sys_helper(int file, int start, int end, int length){
    if (end < start) {
        return;
    }
    char *pivot = calloc(length, sizeof(char));
    lseek(file, end * length * sizeof(char), SEEK_SET);
    read(file, pivot, length);

    char *j_line = calloc(length, sizeof(char));

    int i = start;

    for (int j = start; j < end; ++j) {
        lseek(file, j * length * sizeof(char), SEEK_SET);
        read(file, j_line, length);

        if (strcmp(j_line, pivot) < 0) {
            lseek(file, i * length * sizeof(char), SEEK_SET);
            read(file, pivot, length);
            swap_sys(file, pivot, j_line, i, j, length);
            i++;
            lseek(file, end * length * sizeof(char), SEEK_SET);
            read(file, pivot, length);
        }
    }
    lseek(file, i * length * sizeof(char), SEEK_SET);
    read(file, j_line, length);
    swap_sys(file, j_line, pivot, i, end, length);
    free(pivot);
    free(j_line);
    sort_sys_helper(file, start, i - 1, length);
    sort_sys_helper(file, i + 1, end, length);
}

void sort_sys(char *path, int lines, int length){
    int file = open(path, O_RDWR);
    sort_sys_helper(file, 0, lines - 1, length);
    close(file);
}
