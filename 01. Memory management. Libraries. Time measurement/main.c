#include "mylib.h"

int main(int argc, char **argv) {

    if(strcmp(argv[0], "create_table")!=0) {
        // error create table has to be first arg
    }
    int size = atoi(argv[1]);
    struct block_array * array = create_block_array(size);

    for (int i = 2; i < argc; ++i) {
        if(strcmp(argv[i], "compare_pairs")==0){
            char **file_pairs = &argv[++i];
            int length = 0;
            while (strcmp(argv[i], "compare_pairs")!=0 || strcmp(argv[i], "remove_block")!=0 || strcmp(argv[i], "remove_operation")!=0){
                length++;
                i++;
            }
            add_file_sequence(array, length, file_pairs);
            compare_files(array);
            for (int j = 0; j < length; ++j) {
                save_block(array, file_pairs[j]);
            };
        }
        else if(strcmp(argv[i], "remove_block")==0){
            remove_block(array, atoi(argv[++i]));
        }
        else if(strcmp(argv[i], "remove_operation")==0){
            int block_to_delete_from = ++i;
            int operation_to_delete = ++i;
            remove_operation(array, atoi(argv[block_to_delete_from]), atoi(argv[operation_to_delete]));
        }
    }

    return 0;
}
