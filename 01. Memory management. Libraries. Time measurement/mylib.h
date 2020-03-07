#ifndef OPERATINGSYSTEMS_MYLIB_H
#define OPERATINGSYSTEMS_MYLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct operation {
    unsigned int length;
    char *text;
};

struct block {
    char *file_pair;
    unsigned int length;
    struct operation **operations;
};

struct block_array {
    struct block **blocks;
    unsigned int size;
};


struct block_array *create_block_array(int size);

void add_file_sequence(struct block_array *array, int length, char **file_pairs);

void compare_files(struct block_array *array);

unsigned int save_block(struct block_array *array, char *file_pair);

unsigned int diff_length(struct block_array *array, int block_index);

void remove_block(struct block_array *array, int block_index);

void remove_operation(struct block_array *array, int block_index, int operation_index);


#endif //OPERATINGSYSTEMS_MYLIB_H
