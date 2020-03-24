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
    long process_count = 3;
    long time = 1;
    bool shared_result_files = false;

    for (int i = 0; i < process_count; ++i) {
        int p;
        if((p = fork()) == 0){
            multiply(input_file_name, i, process_count, time);
            return 0;
            break;
        } else{
            printf("starting process %d\n", p);
        }
    }

    int i = 0;
    while (i < process_count) {
        int pid;
        int stat = 0;
        if ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
            printf("Proces %d wykonał %d mnożeń macierzy\n", pid, WEXITSTATUS(stat));
            i++;
        }
    }


    if (!shared_result_files) {
        FILE *input_file = fopen(input_file_name, "r");
        char *line = calloc(256, sizeof(char));
        while (fgets(line, 256, input_file)) {
            printf("%d %s\n", getpid(), line);
            strtok(line, " ");
            strtok(NULL, " ");
            char *output_file = strtok(NULL, " ");
            if (fork() == 0) {
                char *command = calloc(2048, sizeof(char));
                strcpy(command, "paste -d '' ");
                char *command_part = calloc(64, sizeof(char));
                for (int j = 0; j < process_count; ++j) {
                    snprintf(command_part, 64, "%s_%d ", output_file, j);
                    strcat(command, command_part);
                }
                snprintf(command_part, 64, "> %s ", output_file);
                strcat(command, command_part);
                printf("%s\n", command);
                execl("/bin/sh", "sh", "-c", command, (char *) NULL);
            } else {
                wait(0);
            }
        }
        free(line);
        fclose(input_file);
    }
    return 0;
}