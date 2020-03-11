#include <zconf.h>
#include <sys/times.h>
#include "../../zad1/mylib.h"

void write_result(FILE* result_file, char* test_title, clock_t st_time, clock_t en_time, struct tms st_cpu, struct tms en_cpu){
    printf("%s\n", test_title);
    printf("REAL_TIME: %ld\n", en_time - st_time);
    printf("USER_TIME: %ld\n", en_cpu.tms_utime - st_cpu.tms_utime);
    printf("SYSTEM_TIME: %ld\n", en_cpu.tms_stime - st_cpu.tms_stime);


    fprintf(result_file, "%s\n", test_title);
    fprintf(result_file, "REAL_TIME: %ld\n", en_time - st_time);
    fprintf(result_file, "USER_TIME: %ld\n", en_cpu.tms_utime - st_cpu.tms_utime);
    fprintf(result_file, "SYSTEM_TIME: %ld\n", en_cpu.tms_stime - st_cpu.tms_stime);
}

int main(int argc, char **argv) {

    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;

    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    char *path = calloc(256, sizeof(char));
    char *test_title;
    snprintf(path, 256, "%s/%s", cwd, "raport3b.txt");
    FILE *result_file = fopen(path, "a");
    if (argc < 2 || strcmp(argv[1], "create_table") != 0) {
        // error create table has to be first arg
        return 1;
    }
    int size = atoi(argv[2]);
    struct block_array * array = create_block_array(size);

    for (int i = 3; i < argc; ++i) {
        if(strcmp(argv[i], "compare_pairs")==0){
            int start = ++i;
            int length = 1;
            while (i + 1 < argc &&
                   strcmp(argv[i + 1], "save_block") != 0 &&
                   strcmp(argv[i + 1], "compare_pairs") != 0 &&
                   strcmp(argv[i + 1], "remove_block") != 0 &&
                   strcmp(argv[i + 1], "start_time") != 0 &&
                   strcmp(argv[i + 1], "stop_time") != 0 &&
                   strcmp(argv[i + 1], "remove_operation") != 0) {
                length++;
                i++;
            }
            add_file_sequence(array, length, &argv[start]);
            compare_files(array);
        }
        else if(strcmp(argv[i], "remove_block")==0){
            remove_block(array, atoi(argv[++i]));
        }
        else if(strcmp(argv[i], "save_block")==0){
            save_block(array, argv[++i]);
        }
        else if(strcmp(argv[i], "remove_operation")==0){
            int block_to_delete_from = ++i;
            int operation_to_delete = ++i;
            remove_operation(array, atoi(argv[block_to_delete_from]), atoi(argv[operation_to_delete]));
        }
        else if(strcmp(argv[i], "start_time")==0){
            test_title = argv[++i];
            st_time = times(&st_cpu);
        }
        else if(strcmp(argv[i], "stop_time")==0){
            en_time = times(&en_cpu);
            write_result(result_file, test_title, st_time, en_time, st_cpu, en_cpu);
        }
    }
    remove_block_array(array);
    fclose(result_file);
    return 0;
}
