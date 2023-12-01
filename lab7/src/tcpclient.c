#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <getopt.h>

#define SADDR struct sockaddr
#define SIZE sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
    int fd;
    int nread;
    int bufsize = -1;
    int serv_port = -1;
    char address[16] = {'\0'};
    struct sockaddr_in servaddr;

    struct option long_options[] = {
            {"bufsize",  required_argument, NULL, 'b'},
            {"servaddr", required_argument, NULL, 'a'},
            {"servport", required_argument, NULL, 'p'},
            {NULL, 0,                       NULL, 0}
    };

    int opt;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "b:a:p:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'b':
                bufsize = atoi(optarg);
                break;
            case 'a':
                strcpy(address, optarg);
            case 'p':
                serv_port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s --bufsize bufsize --servaddr servaddr --servport servport\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    char buf[bufsize];

    if ((bufsize < 0) || (serv_port < 0) || (strlen(address) == 0)) {
        fprintf(stderr, "Usage: %s --bufsize bufsize --servaddr servaddr --servport servport\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creating");
        exit(1);
    }

    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        perror("bad address");
        exit(1);
    }

    memset(&servaddr, 0, SIZE);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serv_port);


    if (connect(fd, (SADDR *) &servaddr, SIZE) < 0) {
        perror("connect");
        exit(1);
    }

    write(1, "Input message to send\n", 22);
    while ((nread = read(0, buf, bufsize)) > 0) {
        if (write(fd, buf, nread) < 0) {
            perror("write");
            exit(1);
        }
    }

    close(fd);
    exit(0);
}