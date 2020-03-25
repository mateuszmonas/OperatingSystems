#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <bits/types/FILE.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>

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

void generate(char* file_name, long count, long min_size, long max_size){
    char *file_name_cpy = calloc(PATH_MAX, sizeof(char));
    strcpy(file_name_cpy, file_name);
    char *path = dirname(file_name_cpy);
    char *filename_A = calloc(PATH_MAX, sizeof(char));
    char *filename_B = calloc(PATH_MAX, sizeof(char));
    FILE *A;
    FILE *B;
    FILE *MATRICES = fopen(file_name, "w");
    for (int i = 0; i < count; ++i) {
        snprintf(filename_A, PATH_MAX, "matrix_%d_a", i);
        snprintf(filename_B, PATH_MAX, "matrix_%d_b", i);
        fprintf(MATRICES, "%s %s matrix_%d_c\n", filename_A, filename_B, i);
        snprintf(filename_A, PATH_MAX, "%s/matrix_%d_a", path, i);
        snprintf(filename_B, PATH_MAX, "%s/matrix_%d_b", path, i);
        A = fopen(filename_A, "w");
        B = fopen(filename_B, "w");
        int columns = rand() % (max_size - min_size) + min_size;
        int rows = rand() % (max_size - min_size) + min_size;
        generate_matrix(columns, rows, A, B);
        fclose(A);
        fclose(B);
    }
    free(file_name_cpy);
    fclose(MATRICES);
}



bool check(FILE* A, FILE* B, FILE* C){
    char *matrix_line = calloc(2048, sizeof(char));

    int a_rows = 0;
    int a_cols = 0;
    while(fgets(matrix_line, 2048, A)) a_rows++;
    strtok(matrix_line, " ");
    while(strtok(NULL, " ")) a_cols++;
    fseek(A, 0, SEEK_SET);

    int b_rows = 0;
    int b_cols = 0;
    while(fgets(matrix_line, 2048, B)) b_rows++;
    strtok(matrix_line, " ");
    while(strtok(NULL, " ")) b_cols++;
    fseek(B, 0, SEEK_SET);

    int c_rows = 0;
    int c_cols = 0;
    while(fgets(matrix_line, 2048, C)) c_rows++;
    strtok(matrix_line, " ");
    while(strtok(NULL, " ")) c_cols++;
    fseek(C, 0, SEEK_SET);

    long **matrix_a = calloc(a_rows, sizeof(long *));
    long **matrix_b = calloc(b_rows, sizeof(long *));
    long **matrix_c = calloc(c_rows, sizeof(long *));
    for (int i = 0; i < a_rows; ++i) {
        matrix_a[i] = calloc(a_cols, sizeof(long));
    }
    for (int i = 0; i < b_rows; ++i) {
        matrix_b[i] = calloc(b_cols, sizeof(long));
    }
    for (int i = 0; i < c_rows; ++i) {
        matrix_c[i] = calloc(c_cols, sizeof(long));
    }
    for (int i = 0; i < a_rows; ++i) {
        fgets(matrix_line, 2048, A);
        matrix_a[i][0] = strtol(strtok(matrix_line, " "), NULL, 10);
        for (int j = 1; j < a_cols; ++j) {
            matrix_a[i][j] = strtol(strtok(NULL, " "), NULL, 10);
        }
    }
    for (int i = 0; i < b_rows; ++i) {
        fgets(matrix_line, 2048, B);
        matrix_b[i][0] = strtol(strtok(matrix_line, " "), NULL, 10);
        for (int j = 1; j < b_cols; ++j) {
            matrix_b[i][j] = strtol(strtok(NULL, " "), NULL, 10);
        }
    }
    for (int i = 0; i < c_rows; ++i) {
        fgets(matrix_line, 2048, C);
        matrix_c[i][0] = strtol(strtok(matrix_line, " "), NULL, 10);
        for (int j = 1; j < c_cols; ++j) {
            matrix_c[i][j] = strtol(strtok(NULL, " "), NULL, 10);
        }
    }

    bool result = true;
    long current = 0;
    for (int i = 0; i < a_rows && result; ++i) {
        for (int j = 0; j < b_cols && result; ++j) {
            for (int k = 0; k < b_rows; ++k) {
                current += matrix_a[i][k] * matrix_b[k][j];
            }
            result = matrix_c[i][j] == current;
        }
    }

    for (int i = 0; i < a_rows; ++i) {
        free(matrix_a[i]);
    }
    for (int i = 0; i < b_rows; ++i) {
        free(matrix_b[i]);
    }
    for (int i = 0; i < c_rows; ++i) {
        free(matrix_c[i]);
    }
    free(matrix_a);
    free(matrix_b);
    free(matrix_c);
    free(matrix_line);
    return current;
}

void check_all(char*input_file){
    char *file_name_cpy = calloc(PATH_MAX, sizeof(char));
    strcpy(file_name_cpy, input_file);
    char *path = dirname(file_name_cpy);
    char *line = calloc(2048, sizeof(char));
    FILE *input = fopen(input_file, "r");
    while (fgets(line, 2048, input)) {
        char *file_name_a = calloc(PATH_MAX, sizeof(char));
        char *file_name_b = calloc(PATH_MAX, sizeof(char));
        char *file_name_c = calloc(PATH_MAX, sizeof(char));
        snprintf(file_name_a, PATH_MAX, "%s/%s", path, strtok(line, " "));
        snprintf(file_name_b, PATH_MAX, "%s/%s", path, strtok(NULL, " "));
        char *c_name = strtok(NULL, " ");
        c_name[strlen(c_name) - 1] = '\0';
        snprintf(file_name_c, PATH_MAX, "%s/%s", path, c_name);
        FILE *A = fopen(file_name_a, "r");
        FILE *B = fopen(file_name_b, "r");
        FILE *C = fopen(file_name_c, "r");
        if(!check(A, B, C)){
            printf("%s not correct \n", c_name);
        }
        fclose(C);
        fclose(B);
        fclose(A);
    }
    free(file_name_cpy);
    fclose(input);
    free(line);
}

int main(int argc, char** argv) {
    if(strcmp(argv[1], "generate") == 0){
        srand(time(NULL));
        if(argc < 5) {
            return 1;
        }
        char *path = argv[2];
        long matrix_count = strtol(argv[3], NULL, 10);
        long min_size = strtol(argv[4], NULL, 10);
        long max_size = strtol(argv[5], NULL, 10);
        generate(path, matrix_count, min_size, max_size);
    } else{
        if(argc < 3) {
            return 1;
        }
        check_all(argv[2]);
    }
    return 0;
}