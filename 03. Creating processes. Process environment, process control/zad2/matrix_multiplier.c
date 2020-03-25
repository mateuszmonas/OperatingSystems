#include <signal.h>
#include <stdbool.h>
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <libgen.h>

int start_column;
int columns_to_multiply;
int matrix_b_rows;
int multiplication_count = 0;

static void finish_process(){
    exit(multiplication_count);
}

void set_number_of_columns(FILE* file, int process_number, int process_count){
    matrix_b_rows = 1;
    size_t size = 0;
    char *lineptr = NULL;
    int columns_count = 0;
    getline(&lineptr, &size, file);
    for (int i = 0; lineptr[i]; ++i) {
        columns_count += (lineptr[i] == ' ');
    }
    while (getline(&lineptr, &size, file) != -1){
        matrix_b_rows++;
    }
    start_column = columns_count / process_count * process_number;
    columns_to_multiply = process_number == process_count - 1 ? columns_count - start_column : columns_count / process_count;
    free(lineptr);
    fseek(file, 0, SEEK_SET);
}

void multiply_shared_output_file(char* input_file_name, int process_number, int process_count, long time){
    signal(SIGALRM, finish_process);
    alarm(time);

    char *file_name_cpy = calloc(PATH_MAX, sizeof(char));
    strcpy(file_name_cpy, input_file_name);
    char *path = dirname(file_name_cpy);

    FILE* input_file = fopen(input_file_name, "r");
    char *line = calloc(2048, sizeof(char));

    while (fgets(line, 2048, input_file)){
        char *matrix_a_file_name = calloc(PATH_MAX, sizeof(char));
        char *matrix_b_file_name = calloc(PATH_MAX, sizeof(char));
        char *matrix_c_file_name = calloc(PATH_MAX, sizeof(char));
        snprintf(matrix_a_file_name, PATH_MAX, "%s/%s", path, strtok(line, " "));
        snprintf(matrix_b_file_name, PATH_MAX, "%s/%s", path, strtok(NULL, " "));

        char *output_file_name = strtok(NULL, " ");
        output_file_name[strlen(output_file_name) - 1] = '\0';

        snprintf(matrix_c_file_name, PATH_MAX, "%s/%s", path, output_file_name);

        FILE* output_file = fopen(matrix_c_file_name, "r+");
        FILE *matrix_a = fopen(matrix_a_file_name, "r");
        FILE *matrix_b = fopen(matrix_b_file_name, "r");

        set_number_of_columns(matrix_b, process_number, process_count);

        int rows = 0;
        int cols = 0;
        char *matrix_line = calloc(2048, sizeof(char));
        while(fgets(matrix_line, 2048, matrix_a)) rows++;
        fgets(matrix_line, 2048, matrix_b);
        strtok(matrix_line, " ");
        while(strtok(NULL, " ")) cols++;
        long **result_matrix = calloc(rows, sizeof(long *));
        for (int l = 0; l < rows; ++l) {
            result_matrix[l] = calloc(cols, sizeof(long));
        }
        fseek(matrix_a, 0, SEEK_SET);
        fseek(matrix_b, 0, SEEK_SET);

        char *matrix_row_a = calloc(2048, sizeof(char));
        char *matrix_row_b = calloc(2048, sizeof(char));
        int current_row = 0;
        while (fgets(matrix_row_a, 2048, matrix_a)){
            int *row_matrix_a_nums = calloc(matrix_b_rows, sizeof(int));
            row_matrix_a_nums[0] = strtol(strtok(matrix_row_a, " "), NULL, 10);
            for (int j = 1; j < matrix_b_rows; ++j) {
                row_matrix_a_nums[j] = strtol(strtok(NULL, " "), NULL, 10);
            }

            long result;
            char *current_num1;
            for (int current_column = 0; current_column < columns_to_multiply; ++current_column) {
                fseek(matrix_b, 0, SEEK_SET);

                // read correct column from b matrix
                fgets(matrix_row_b, 2048, matrix_b);
                current_num1 = strtok(matrix_row_b, " ");
                for (int i = 0; i < start_column + current_column; ++i) {
                    current_num1 = strtok(NULL, " ");
                }

                result = row_matrix_a_nums[0] * strtol(current_num1, NULL, 10);

                for (int current_row = 1; current_row < matrix_b_rows; ++current_row) {
                    fgets(matrix_row_b, 2048, matrix_b);
                    current_num1 = strtok(matrix_row_b, " ");
                    for (int i = 0; i < start_column + current_column; ++i) {
                        current_num1 = strtok(NULL, " ");
                    }
                    result += row_matrix_a_nums[current_row] * strtol(current_num1, NULL, 10);
                }
                result_matrix[current_row][start_column + current_column] = result;
            }
            current_row++;
            free(row_matrix_a_nums);
        }
        int fd = fileno(output_file);
        flock(fd, LOCK_EX);


        for (int k = 0; k < rows; ++k) {
            fgets(matrix_line, 2048, output_file);
            long temp = strtol(strtok(matrix_line, " "), NULL, 10);
            result_matrix[k][0] = start_column == 0 ? result_matrix[k][0] : temp;
            for (int i = 1; i < cols; ++i) {
                if (start_column <= i && i < start_column + columns_to_multiply) continue;
                result_matrix[k][i] = strtol(strtok(NULL, " "), NULL, 10);
            }
        }

        ftruncate(fd, 0);
        fseek(output_file, 0, SEEK_SET);
        for (int k = 0; k < rows; ++k) {
            for (int i = 0; i < cols; ++i) {
                fprintf(output_file, "%ld ", result_matrix[k][i]);
            }
            fprintf(output_file, "\n");
        }

        flock(fd, LOCK_UN);


        for (int l = 0; l < cols; ++l) {
            free(result_matrix[l]);
        }
        free(result_matrix);
        free(matrix_line);
        free(matrix_a_file_name);
        free(matrix_b_file_name);
        free(matrix_c_file_name);
        fclose(matrix_a);
        fclose(matrix_b);
        fclose(output_file);
        free(matrix_row_a);
        free(matrix_row_b);
        multiplication_count++;
    }
    free(file_name_cpy);
    free(line);
    fclose(input_file);
    while (true) {}
}

void multiply_separate_output_files(char* input_file_name, int process_number, int process_count, long time){
    signal(SIGALRM, finish_process);
    alarm(time);

    char *file_name_cpy = calloc(PATH_MAX, sizeof(char));
    strcpy(file_name_cpy, input_file_name);
    char *path = dirname(file_name_cpy);

    FILE* input_file = fopen(input_file_name, "r");
    char *line = calloc(2048, sizeof(char));

    while (fgets(line, 2048, input_file)){
        char *matrix_a_file_name = calloc(PATH_MAX, sizeof(char));
        char *matrix_b_file_name = calloc(PATH_MAX, sizeof(char));;
        char *matrix_c_file_name = calloc(PATH_MAX, sizeof(char));
        snprintf(matrix_a_file_name, PATH_MAX, "%s/%s", path, strtok(line, " "));
        snprintf(matrix_b_file_name, PATH_MAX, "%s/%s", path, strtok(NULL, " "));

        char *output_file_name = strtok(NULL, " ");
        output_file_name[strlen(output_file_name) - 1] = '\0';
        snprintf(matrix_c_file_name, PATH_MAX, "%s/%s_%d", path, output_file_name, process_number);

        FILE* output_file = fopen(matrix_c_file_name, "w");
        FILE *matrix_a = fopen(matrix_a_file_name, "r");
        FILE *matrix_b = fopen(matrix_b_file_name, "r");

        set_number_of_columns(matrix_b, process_number, process_count);

        long *results = calloc(columns_to_multiply, sizeof(long));
        char *matrix_row_a = calloc(2048, sizeof(char));
        char *matrix_row_b = calloc(2048, sizeof(char));
        while (fgets(matrix_row_a, 2048, matrix_a)){
            int *row_matrix_a_nums = calloc(matrix_b_rows, sizeof(int));
            row_matrix_a_nums[0] = strtol(strtok(matrix_row_a, " "), NULL, 10);
            for (int j = 1; j < matrix_b_rows; ++j) {
                row_matrix_a_nums[j] = strtol(strtok(NULL, " "), NULL, 10);
            }

            long result;
            char *current_num1;
            for (int current_column = 0; current_column < columns_to_multiply; ++current_column) {
                fseek(matrix_b, 0, SEEK_SET);

                // read correct column from b matrix
                fgets(matrix_row_b, 2048, matrix_b);
                current_num1 = strtok(matrix_row_b, " ");
                for (int i = 0; i < start_column + current_column; ++i) {
                    current_num1 = strtok(NULL, " ");
                }

                result = row_matrix_a_nums[0] * strtol(current_num1, NULL, 10);

                for (int current_row = 1; current_row < matrix_b_rows; ++current_row) {
                    fgets(matrix_row_b, 2048, matrix_b);
                    current_num1 = strtok(matrix_row_b, " ");
                    for (int i = 0; i < start_column + current_column; ++i) {
                        current_num1 = strtok(NULL, " ");
                    }
                    result += row_matrix_a_nums[current_row] * strtol(current_num1, NULL, 10);
                }
                results[current_column] = result;
            }

            for (int k = 0; k < columns_to_multiply; ++k) {
                fprintf(output_file, "%ld ", results[k]);
            }
            fprintf(output_file, "\n");
            free(row_matrix_a_nums);
        }
        free(matrix_a_file_name);
        free(matrix_b_file_name);
        free(matrix_c_file_name);
        fclose(matrix_a);
        fclose(matrix_b);
        fclose(output_file);
        free(results);
        free(matrix_row_a);
        free(matrix_row_b);
        multiplication_count++;
    }
    free(line);
    fclose(input_file);
    while (true) {}
}
