#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        execl("./sequential_min_max", "sequential_min_max", argv[1], argv[2], NULL);
    } 


    return 0;
}
