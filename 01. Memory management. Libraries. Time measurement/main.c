#include "mylib.h"

int main(int argc, char **argv) {

    const char *c[2];
    c[0] = "a.txt:b.txt";
    c[1] = "c.txt:d.txt";
    struct block_array *array = create_block_array(5);
    add_file_sequence(array, 2, c);
    compare_files(array);
    save_block(array, "a.txt:b.txt");
    save_block(array, "c.txt:d.txt");
    printf("%d", diff_length(array, 1));

    return 0;
}
