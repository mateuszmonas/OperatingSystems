#include <sys/resource.h>
#include <time.h>
#include <zconf.h>
#include "../zad1/mylib.h"

void writeResult(FILE* result_file, struct timespec * start_time, struct timespec * end_time, struct rusage *start_usage, struct rusage *end_usage){
    fprintf(result_file, "REAL_TIME: %ldns\n", end_time->tv_nsec - start_time->tv_nsec);
    fprintf(result_file, "USER_TIME: %ldµs\n", end_usage->ru_utime.tv_usec - start_usage->ru_utime.tv_usec);
    fprintf(result_file, "SYSTEM_TIME: %ldµs\n", end_usage->ru_stime.tv_usec - start_usage->ru_stime.tv_usec);
}

int main(int argc, char **argv) {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    char *path = calloc(256, sizeof(char));
    snprintf(path, 256, "%s/%s", cwd, "raport2.txt");
    FILE *result_file = fopen(path, "a");
    struct rusage *start_usage = calloc(1, sizeof * start_usage);
    struct rusage *end_usage = calloc(1, sizeof * end_usage);
    struct timespec *start_time = calloc(1, sizeof *start_time);
    struct timespec *end_time = calloc(1, sizeof *end_time);
    clock_gettime(CLOCK_REALTIME, start_time);
    getrusage(RUSAGE_SELF, start_usage);
    if(strcmp(argv[1], "create_table")!=0) {
        // error create table has to be first arg
    }
    int size = atoi(argv[2]);
    struct block_array * array = create_block_array(size);

    for (int i = 3; i < argc; ++i) {
        if(strcmp(argv[i], "compare_pairs")==0){
            int start = ++i;
            int length = 0;
            while (i < argc && (strcmp(argv[i], "save_block")!=0 || strcmp(argv[i], "compare_pairs")!=0 || strcmp(argv[i], "remove_block")!=0 || strcmp(argv[i], "remove_operation")!=0)){
                length++;
                i++;
            }
            add_file_sequence(array, length, &argv[start]);
            compare_files(array);
            for (int j = start; j < start + length; ++j) {
                save_block(array, argv[j]);
            }
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
    }
    clock_gettime(CLOCK_REALTIME, end_time);
    getrusage(RUSAGE_SELF, end_usage);
    writeResult(result_file, start_time, end_time, start_usage, end_usage);
    fclose(result_file);
    return 0;
}
