#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <bits/types/FILE.h>
#include <time.h>

void generate_matrix(long columns, long rows, FILE* A, FILE* B){
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            fprintf(A, "%d ", (rand() % 200) - 100);
        }
        fwrite("\n", 1, sizeof(char), A);
    }
    for (int i = 0; i < columns; ++i) {
        for (int j = 0; j < rows; ++j) {
            fprintf(B, "%d ", (rand() % 200) - 100);
        }
        fwrite("\n", 1, sizeof(char), B);
    }
}

int main(int argc, char** argv) {
    srand(time(NULL));
    char *path = calloc(PATH_MAX, sizeof(char));
    getcwd(path, PATH_MAX);
    if(argc < 4) {
        return 1;
    }
    long matrix_count = strtol(argv[1], NULL, 10);
    long min_size = strtol(argv[2], NULL, 10);
    long max_size = strtol(argv[3], NULL, 10);

    char *filename_A = calloc(PATH_MAX, sizeof(char));
    char *filename_B = calloc(PATH_MAX, sizeof(char));
    FILE *A;
    FILE *B;
    for (int i = 0; i < matrix_count; ++i) {
        snprintf(filename_A, PATH_MAX, "matrix_%d_a", i);
        snprintf(filename_B, PATH_MAX, "matrix_%d_b", i);
        A = fopen(filename_A, "w");
        B = fopen(filename_B, "w");
        int columns = rand() % (max_size - min_size) + min_size;
        int rows = rand() % (max_size - min_size) + min_size;
        generate_matrix(columns, rows, A, B);
        fclose(A);
        fclose(B);
    }
    return 0;
}