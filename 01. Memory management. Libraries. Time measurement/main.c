#include <sys/times.h>
#include <zconf.h>
#include "mylib.h"

double timeDifference(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void writeResult(clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    printf("\tREAL_TIME: %fl\n", timeDifference(start,end));
    printf("\tUSER_TIME: %fl\n", timeDifference(t_start->tms_utime, t_end->tms_utime));
    printf("\tSYSTEM_TIME: %fl\n", timeDifference(t_start->tms_stime, t_end->tms_stime));

//    fprintf(resultFile, "\tREAL_TIME: %fl\n", timeDifference(start, end));
//    fprintf(resultFile, "\tUSER_TIME: %fl\n", timeDifference(t_start->tms_utime, t_end->tms_utime));
//    fprintf(resultFile, "\tSYSTEM_TIME: %fl\n", timeDifference(t_start->tms_stime, t_end->tms_stime));
}

int main(int argc, char **argv) {
    clock_t program_start, program_end;
    struct tms *program_start_tms = calloc(1, sizeof * program_start_tms);
    struct tms *program_end_tms = calloc(1, sizeof * program_end_tms);
    program_start = times(program_start_tms);

    if(strcmp(argv[1], "create_table")!=0) {
        // error create table has to be first arg
    }
    int size = atoi(argv[2]);
    struct block_array * array = create_block_array(size);

    for (int i = 3; i < argc; ++i) {
        if(strcmp(argv[i], "compare_pairs")==0){
            int start = ++i;
            int length = 0;
            while (i < argc && (strcmp(argv[i], "compare_pairs")!=0 || strcmp(argv[i], "remove_block")!=0 || strcmp(argv[i], "remove_operation")!=0)){
                length++;
                i++;
            }
            add_file_sequence(array, length, &argv[start]);
            compare_files(array);
            for (int j = start; j < start + length; ++j) {
                save_block(array, argv[j]);
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
    program_end = times(program_end_tms);
    writeResult(program_start, program_end, program_start_tms, program_end_tms);
    return 0;
}
