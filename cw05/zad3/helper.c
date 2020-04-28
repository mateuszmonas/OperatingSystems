#include <zconf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char** argv) {
    mkfifo("fifo", 0666);
    char* producent[] = {"./out/producent", "fifo", "", "25", NULL};
    char* konsument[] = {"./out/konsument", "fifo", "out.txt", "100", NULL};
    char *file_name = calloc(16, sizeof(char));
    for (int i = 1; i < 6; ++i) {
        snprintf(file_name, 16, "in%d.txt", i);
        producent[2] = file_name;
        if (fork() == 0) {
            execvp(producent[0], producent);
            return errno;
        }
    }
    execvp(konsument[0], konsument);
    return errno;
}