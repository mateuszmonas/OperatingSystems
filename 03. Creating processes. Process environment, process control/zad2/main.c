#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <wait.h>

#include "matrix_multiplier.c"

int main(int argc, char** argv) {
//    if(argc < 4) {
//        return -1;
//    }
//    char* input_file_name = argv[1];
//    long process_count = strtol(argv[2], NULL, 10);
//    long time = strtol(argv[3], NULL, 10);


    char* input_file_name = "matrices";
    long process_count = 2;
    long time = 3;

    int p;
    for (int i = 0; i < process_count; ++i) {
        if((p = fork()) == 0){
            multiply(input_file_name, i, process_count, time);
            break;
        } else{
            printf("starting process %d\n", p);
        }
    }
    if(p!=0){
        int i = 0;
        while (i < process_count) {
            int pid;
            int stat = 0;
            if ((pid = waitpid(-1, &stat, WNOHANG)) > 0){
                printf("Proces %d wykonał %d mnożeń macierzy\n", pid, WEXITSTATUS(stat));
                i++;
            }
        }
}
    return 0;
}