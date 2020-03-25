#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <wait.h>
#include <sys/resource.h>

#include "../zad2/matrix_multiplier.c"
void create_output_files(char* input_file_name){
    FILE *input_file = fopen(input_file_name, "r");
    char *line = calloc(256, sizeof(char));
    while (fgets(line, 256, input_file)) {
        FILE* A = fopen(strtok(line, " "), "r");
        FILE* B = fopen(strtok(NULL, " "), "r");
        char *output_file_name = strtok(NULL, " ");
        output_file_name[strlen(output_file_name) - 1] = '\0';
        FILE* output_file = fopen(output_file_name, "w");
        int rows = 0;
        int cols = 0;
        char *matrix_line = calloc(2048, sizeof(char));
        while(fgets(matrix_line, 2048, A)) rows++;
        fgets(matrix_line, 2048, B);
        strtok(matrix_line, " ");
        while(strtok(NULL, " ")) cols++;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                fprintf(output_file, "0 ");
            }
            fprintf(output_file, "\n");
        }

        free(matrix_line);
        fclose(A);
        fclose(B);
        fclose(output_file);
    }
    free(line);
    fclose(input_file);
}

int main(int argc, char** argv) {
    if(argc < 4) {
        return -1;
    }
    char* input_file_name = argv[1];
    long process_count = strtol(argv[2], NULL, 10);
    long time = strtol(argv[3], NULL, 10);
    bool shared_result_files = strcmp(argv[4], "shared") == 0;
    long cpu_limit = strtol(argv[5], NULL, 10);
    long memory_limit = strtol(argv[6], NULL, 10);

    if(shared_result_files){
        create_output_files(input_file_name);
    }

    for (int i = 0; i < process_count; ++i) {
        int p;
        if((p = fork()) == 0){
            struct rlimit * limit = calloc(1, sizeof *limit);
            limit->rlim_cur = cpu_limit;
            limit->rlim_max = cpu_limit;
            setrlimit(RLIMIT_CPU, limit);
            limit->rlim_cur = memory_limit*1048576;
            limit->rlim_max = memory_limit*1048576;
            setrlimit(RLIMIT_AS, limit);
            if(shared_result_files) multiply_shared_output_file(input_file_name, i, process_count, time);
            else multiply_separate_output_files(input_file_name, i, process_count, time);
            free(limit);
            return 0;
        } else{
            printf("starting process %d\n", p);
        }
    }

    struct rusage * usage = calloc(1, sizeof * usage);

    long prev_utime = 0;
    long prev_stime = 0;
    int i = 0;
    while (i < process_count) {
        int pid;
        int stat = 0;
        if ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
            printf("Proces %d wykonał %d mnożeń macierzy ", pid, WEXITSTATUS(stat));
            getrusage(RUSAGE_CHILDREN, usage);
            printf(" | user time: %lds | system time: %lds\n", usage->ru_utime.tv_sec - prev_utime,
                   usage->ru_stime.tv_sec - prev_stime);
//            prev_utime = usage->ru_utime.tv_usec;
//            prev_stime = usage->ru_stime.tv_usec;
            i++;
        }
    }


    if (!shared_result_files) {
        FILE *input_file = fopen(input_file_name, "r");
        char *line = calloc(256, sizeof(char));
        while (fgets(line, 256, input_file)) {
            strtok(line, " ");
            strtok(NULL, " ");
            char *output_file = strtok(NULL, " ");
            output_file[strlen(output_file) - 1] = '\0';
            if (fork() == 0) {
                char *command = calloc(2048, sizeof(char));
                strcpy(command, "paste -d '' ");
                char *command_part = calloc(64, sizeof(char));
                for (int j = 0; j < process_count; ++j) {
                    snprintf(command_part, 64, "%s_%d ", output_file, j);
                    printf("%s\n", command_part);
                    strcat(command, command_part);
                }
                snprintf(command_part, 64, "> %s ", output_file);
                strcat(command, command_part);
                execl("/bin/sh", "sh", "-c", command, (char *) NULL);
            } else {
                wait(0);
                for (int j = 0; j < process_count; ++j) {
                    char *file_name = calloc(64, sizeof(char));
                    snprintf(file_name, 64, "%s_%d", output_file, j);
                    remove(file_name);
                }
            }
        }
        free(line);
        fclose(input_file);
    }
    return 0;
}