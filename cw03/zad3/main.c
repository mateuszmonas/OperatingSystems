#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <wait.h>
#include <sys/resource.h>

#include "../zad2/matrix_multiplier.c"
void create_output_files(char* input_file_name){

    char *file_name_cpy = calloc(PATH_MAX, sizeof(char));
    strcpy(file_name_cpy, input_file_name);
    char *path = dirname(file_name_cpy);

    FILE *input_file = fopen(input_file_name, "r");
    char *line = calloc(256, sizeof(char));
    while (fgets(line, 256, input_file)) {
        char *matrix_a_file_name = calloc(PATH_MAX, sizeof(char));
        char *matrix_b_file_name = calloc(PATH_MAX, sizeof(char));;
        char *matrix_c_file_name = calloc(PATH_MAX, sizeof(char));
        snprintf(matrix_a_file_name, PATH_MAX, "%s/%s", path, strtok(line, " "));
        snprintf(matrix_b_file_name, PATH_MAX, "%s/%s", path, strtok(NULL, " "));

        char *output_file_name = strtok(NULL, " ");
        output_file_name[strlen(output_file_name) - 1] = '\0';
        snprintf(matrix_c_file_name, PATH_MAX, "%s/%s", path, output_file_name);

        FILE* A = fopen(matrix_a_file_name, "r");
        FILE* B = fopen(matrix_b_file_name, "r");
        FILE* output_file = fopen(matrix_c_file_name, "w");
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
        free(matrix_a_file_name);
        free(matrix_b_file_name);
        free(matrix_c_file_name);
        fclose(A);
        fclose(B);
        fclose(output_file);
    }
    free(line);
    free(file_name_cpy);
    fclose(input_file);
}

void print_and_update_time(struct timeval * prev_utime,struct timeval * curr_utime,struct timeval * prev_stime,struct timeval * curr_stime) {
    long user_time = (curr_utime->tv_sec - prev_utime->tv_sec) * 1000000 +
                     (curr_utime->tv_usec - prev_utime->tv_usec);
    long system_time = (curr_stime->tv_sec - prev_stime->tv_sec) * 1000000 +
                       (curr_stime->tv_usec - prev_stime->tv_usec);
    printf(" | user time: %ld.%06ld | system time: %ld.%06ld\n", user_time/1000000, user_time%1000000, system_time/1000000, system_time%1000000);
    prev_utime->tv_sec = curr_utime->tv_sec;
    prev_utime->tv_usec = curr_utime->tv_usec;
    prev_stime->tv_sec = curr_stime->tv_sec;
    prev_stime->tv_usec = curr_stime->tv_usec;
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

    struct timeval prev_utime = {0, 0};
    struct timeval prev_stime = {0, 0};
    int i = 0;
    while (i < process_count) {
        int pid;
        int stat = 0;
        if ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
            printf("Proces %d wykonał %d mnożeń macierzy ", pid, WEXITSTATUS(stat));
            getrusage(RUSAGE_CHILDREN, usage);
            print_and_update_time(&prev_utime, &usage->ru_utime, &prev_stime, &usage->ru_stime);
            i++;
        }
    }


    if (!shared_result_files) {
        char *file_name_cpy = calloc(PATH_MAX, sizeof(char));
        strcpy(file_name_cpy, input_file_name);
        char *path = dirname(file_name_cpy);

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
                    snprintf(command_part, 64, "%s/%s_%d ", path, output_file, j);
                    strcat(command, command_part);
                }
                snprintf(command_part, 64, "> %s/%s ", path, output_file);
                strcat(command, command_part);
                execl("/bin/sh", "sh", "-c", command, (char *) NULL);
            } else {
                wait(0);
                for (int j = 0; j < process_count; ++j) {
                    char *file_name = calloc(64, sizeof(char));
                    snprintf(file_name, 64, "%s/%s_%d", path, output_file, j);
                    remove(file_name);
                }
            }
        }
        free(file_name_cpy);
        free(line);
        fclose(input_file);
    }
    return 0;
}