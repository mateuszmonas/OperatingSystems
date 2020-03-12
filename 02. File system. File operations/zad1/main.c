#include <bits/types/FILE.h>
#include <sys/resource.h>
#include <time.h>
#include <stdbool.h>
#include <zconf.h>
#include <stdio.h>
#include <fcntl.h>
#include "stdlib.h"
#include "string.h"

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

int main(){
    char *file1 = get_file_path("destination.txt");
    char *file2 = get_file_path("destination1.txt");
//    generate(file1, 3, 256);
//    copy_sys(file1, file2, 3, 16);
//    sort_lib(file2, 3, 16);
    sort_sys(file2, 3, 16);
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
    fseek(file, end * length * sizeof(char), SEEK_SET);
    char *pivot = calloc(length, sizeof(char));
    fread(pivot, sizeof(char), length, file);

    char *j_line = calloc(length, sizeof(char));
    char *i_line = calloc(length, sizeof(char));

    int i = start;
    fseek(file, i * length * sizeof(char), SEEK_SET);
    fread(i_line, sizeof(char), length, file);

    for (int j = start; j < end; ++j) {
        fseek(file, j * length * sizeof(char), SEEK_SET);
        fread(j_line, sizeof(char), length, file);

        if (strcmp(j_line, pivot) < 0) {
            swap_lib(file, i_line, j_line, i, j, length);
            i++;
            fseek(file, i * length * sizeof(char), SEEK_SET);
            fread(i_line, sizeof(char), length, file);
        }
    }
    swap_lib(file, i_line, pivot, i, end, length);
    free(pivot);
    free(i_line);
    free(j_line);
    sort_lib_helper(file, start, i - 1, length);
    sort_lib_helper(file, i + 1, end, length);
}

void sort_lib(char *path, int lines, int length){
    FILE *file = fopen(path, "r+");
    sort_lib_helper(file, 0, lines - 1, length);
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
    lseek(file, end * length * sizeof(char), SEEK_SET);
    char *pivot = calloc(length, sizeof(char));
    read(file, pivot, length);

    char *j_line = calloc(length, sizeof(char));
    char *i_line = calloc(length, sizeof(char));

    int i = start;
    lseek(file, i * length * sizeof(char), SEEK_SET);
    read(file, i_line, length);

    for (int j = start; j < end; ++j) {
        lseek(file, j * length * sizeof(char), SEEK_SET);
        read(file, j_line, length);

        if (strcmp(j_line, pivot) < 0) {
            swap_sys(file, i_line, j_line, i, j, length);
            i++;
            lseek(file, i * length * sizeof(char), SEEK_SET);
            read(file, i_line, length);
        }
    }
    swap_sys(file, i_line, pivot, i, end, length);
    free(pivot);
    free(i_line);
    free(j_line);
    sort_sys_helper(file, start, i - 1, length);
    sort_sys_helper(file, i + 1, end, length);
}

void sort_sys(char *path, int lines, int length){
    int file = open(path, O_RDWR);
    sort_sys_helper(file, 0, lines - 1, length);
}

