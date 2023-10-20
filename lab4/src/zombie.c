#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t child_pid = fork();

    if (child_pid > 0) {
        printf("Родительский процесс\n");
        sleep(10); 
        wait(NULL); 
        printf("Завершение родительского процесса\n");
    } else if (child_pid == 0) {
        printf("Дочерний процесс\n");
        sleep(5);
        printf("Завершение дочернего процесса\n");
        exit(0);
    } else {
        printf("Ошибка при создании дочернего процесса\n");
        return 1;
    }

    return 0;
}