#include "mylib.h"
#include <unistd.h>
#include <zconf.h>

struct block_array *create_block_array(int size) {
    struct block_array *array = malloc(sizeof *array);
    array->blocks = malloc(size * sizeof(struct block *));
    array->size = size;
    return array;
}

char *get_working_dir() {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    return cwd;
}

void add_file_sequence(struct block_array *array, int length, char **file_pairs) {
    for (int i = 0; i < length; ++i) {
        struct block *new_block = malloc(sizeof(struct block));
        new_block->file_pair = calloc(strlen(file_pairs[i]), sizeof(char));
        strcpy(new_block->file_pair, file_pairs[i]);
        array->blocks[i] = new_block;
    }
}

char *create_temp_file_name(unsigned int block_index) {
    char *filename = calloc(32, sizeof(char));
    snprintf(filename, 32, "temp%d.txt", block_index);
    return filename;
}

void compare_files(struct block_array *array) {
    for (int i = 0; i < array->size; ++i) {
        if (array->blocks[i] != NULL) {
            char *file_pair_cpy = calloc(strlen(array->blocks[i]->file_pair), sizeof(char));
            strcpy(file_pair_cpy, array->blocks[i]->file_pair);
            char *file1 = strtok(file_pair_cpy, ":");
            char *file2 = strtok(NULL, ":");
            char *output_file = create_temp_file_name(i);
            char *command = calloc(256, sizeof(char));
            snprintf(command, 256, "cd '%s' && diff '%s' '%s' > '%s'",
                     get_working_dir(), file1, file2, output_file);
            system(command);
        }
    }
}

struct operation *create_operation(char *operation_text) {
    struct operation *o = malloc(sizeof(struct operation));
    o->text = operation_text;
    return o;
}

unsigned int save_block(struct block_array *array, char *file_pair) {
    struct block *curr_block = NULL;
    int block_index = 0;
    for (int i = 0; i < array->size; ++i) {
        if (array->blocks[i] != NULL && strcmp(array->blocks[i]->file_pair, file_pair) == 0) {
            curr_block = array->blocks[i];
            block_index = i;
            break;
        }
    }
    if (curr_block == NULL) {
        return -1;
    }
    char *file_name = create_temp_file_name(block_index);
    curr_block->operations = malloc(sizeof(struct operation *));
    char *buffer = calloc(256, sizeof(char));
    char *path = calloc(256, sizeof(char));
    char *result = calloc(256, sizeof(char));
    snprintf(path, 256, "%s/%s", get_working_dir(), file_name);
    FILE *file = fopen(path, "r");
    int index = 0;
    fgets(buffer, 256, file);
    while (fgets(buffer, 256, file)) {
        if (isdigit(buffer[0])) {
            curr_block->operations[index] = create_operation(result);
            index++;
            curr_block->operations = realloc(curr_block->operations, (index + 1) * sizeof(struct operation *));
            curr_block->length = index;
            result = (char *) calloc(256, sizeof(char));
            continue;
        }
        strcat(result, buffer);
    }
    curr_block->operations[index] = create_operation(result);
    curr_block->length = index + 1;
    fclose(file);
    remove(path);
    return block_index;
}

unsigned int diff_length(struct block_array *array, int block_index) {
    return array->blocks[block_index]->length;
}

void remove_block(struct block_array *array, int block_index) {
    free(array->blocks[block_index]);
}

void remove_operation(struct block_array *array, int block_index, int operation_index) {
    free(array->blocks[block_index]->operations[operation_index]);
}


