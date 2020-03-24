#include <signal.h>
#include <stdbool.h>
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int start_column;
int columns_to_multiply;
int rows;
int multiplication_count = 0;

static void finish_process(){
    exit(multiplication_count);
}

void set_number_of_columns(FILE* file, int process_number, int process_count){
    rows = 1;
    size_t size = 0;
    char *lineptr = NULL;
    int columns_count = 0;
    getline(&lineptr, &size, file);
    for (int i = 0; lineptr[i]; ++i) {
        columns_count += (lineptr[i] == ' ');
    }
    while (getline(&lineptr, &size, file) != -1){
        rows++;
    }
    start_column = columns_count / process_count * process_number;
    columns_to_multiply = process_number == process_count - 1 ? columns_count - start_column : columns_count / process_count;
    free(lineptr);
    fseek(file, 0, SEEK_SET);
}

void multiply(char* input_file_name, int process_number, int process_count, long time){
    signal(SIGALRM, finish_process);
    alarm(time);
    FILE* input_file = fopen(input_file_name, "r");
    char *line = calloc(256, sizeof(char));

    while (fgets(line, 256, input_file)){
        char *matrix_a_file_name = strtok(line, " ");
        char *matrix_b_file_name = strtok(NULL, " ");;
        char *out_file_name = calloc(64, sizeof(char));
        snprintf(out_file_name, 64, "%s_%d", strtok(NULL, " "), process_number);


        FILE* output_file = fopen(out_file_name, "w");
        FILE *matrix_a = fopen(matrix_a_file_name, "r");
        FILE *matrix_b = fopen(matrix_b_file_name, "r");

        set_number_of_columns(matrix_b, process_number, process_count);

        long *results = calloc(columns_to_multiply, sizeof(long));
        char *matrix_row_a = calloc(256, sizeof(char));
        char *matrix_row_b = calloc(256, sizeof(char));
        while (fgets(matrix_row_a, 256, matrix_a)){
            int *row_matrix_a_nums = calloc(rows, sizeof(int));
            row_matrix_a_nums[0] = strtol(strtok(matrix_row_a, " "), NULL, 10);
            for (int j = 1; j < rows; ++j) {
                row_matrix_a_nums[j] = strtol(strtok(NULL, " "), NULL, 10);
            }

            long result;
            char *current_num1;
            for (int current_column = 0; current_column < columns_to_multiply; ++current_column) {
                fseek(matrix_b, 0, SEEK_SET);

                // read correct column from b matrix
                fgets(matrix_row_b, 256, matrix_b);
                current_num1 = strtok(matrix_row_b, " ");
                for (int i = 0; i < start_column + current_column; ++i) {
                    current_num1 = strtok(NULL, " ");
                }

                result = row_matrix_a_nums[0] * strtol(current_num1, NULL, 10);

                for (int current_row = 1; current_row < rows; ++current_row) {
                    fgets(matrix_row_b, 256, matrix_b);
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
                printf("%ld ", results[k]);
            }
            fprintf(output_file, "\n");
            printf("\n");
            free(row_matrix_a_nums);
        }
        fclose(matrix_a);
        fclose(matrix_b);
        fclose(output_file);
        free(out_file_name);
        free(results);
        free(matrix_row_a);
        free(matrix_row_b);
        multiplication_count++;
    }
    free(line);
    fclose(input_file);
    while (true) {}
}
