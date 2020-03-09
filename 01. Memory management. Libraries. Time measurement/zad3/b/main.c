#include <sys/resource.h>
#include <time.h>
#include <zconf.h>
#include "../../zad1/mylib.h"

void writeResult(FILE* result_file, char* test_title, struct timespec * start_time, struct timespec * end_time, struct rusage *start_usage, struct rusage *end_usage){
    printf("%s\n", test_title);
    printf("REAL_TIME: %ldns\n", end_time->tv_nsec - start_time->tv_nsec);
    printf("USER_TIME: %ldµs\n", end_usage->ru_utime.tv_usec - start_usage->ru_utime.tv_usec);
    printf("SYSTEM_TIME: %ldµs\n", end_usage->ru_stime.tv_usec - start_usage->ru_stime.tv_usec);


    fprintf(result_file, "%s\n", test_title);
    fprintf(result_file, "REAL_TIME: %ldns\n", end_time->tv_nsec - start_time->tv_nsec);
    fprintf(result_file, "USER_TIME: %ldµs\n", end_usage->ru_utime.tv_usec - start_usage->ru_utime.tv_usec);
    fprintf(result_file, "SYSTEM_TIME: %ldµs\n", end_usage->ru_stime.tv_usec - start_usage->ru_stime.tv_usec);
}

int main(int argc, char **argv) {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    char *path = calloc(256, sizeof(char));
    char *test_title;
    snprintf(path, 256, "%s/%s", cwd, "raport3b.txt");
    FILE *result_file = fopen(path, "a");
    struct rusage *start_usage = calloc(1, sizeof * start_usage);
    struct rusage *end_usage = calloc(1, sizeof * end_usage);
    struct timespec *start_time = calloc(1, sizeof *start_time);
    struct timespec *end_time = calloc(1, sizeof *end_time);
    if(strcmp(argv[1], "create_table")!=0) {
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
            clock_gettime(CLOCK_REALTIME, start_time);
            getrusage(RUSAGE_SELF, start_usage);
        }
        else if(strcmp(argv[i], "stop_time")==0){
            clock_gettime(CLOCK_REALTIME, end_time);
            getrusage(RUSAGE_SELF, end_usage);
            writeResult(result_file, test_title, start_time, end_time, start_usage, end_usage);
        }
    }
    fclose(result_file);
    return 0;
}
