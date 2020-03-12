#include <bits/types/FILE.h>
#include <sys/resource.h>
#include <time.h>
#include <stdbool.h>
#include <zconf.h>
#include <stdio.h>
#include <fcntl.h>
#include "stdlib.h"
#include <libexplain/open.h>

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
    generate(file1, 3, 256);
    copy_sys(file1, file2, 3, 256);
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
