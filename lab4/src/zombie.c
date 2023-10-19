#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t child_pid;
    child_pid = fork();

    if(child_pid > 0) {
        printf("Parent process: sleep for 20 seconds, child: %d\n", child_pid);
        sleep(20);
        printf("Parent process: exit\n");
    }
    else {
        printf("Child process: sleep for 10 seconds\n");
        sleep(10);
        printf("Child process: exit, becomes zombie\n");
        exit(0);
    }

    return 0;
}