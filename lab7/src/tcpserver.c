#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <getopt.h>

#define SADDR struct sockaddr

int main(int argc, char **argv) {
    const size_t kSize = sizeof(struct sockaddr_in);
    int lfd, cfd;
    int nread;
    int bufsize = -1;
    int serv_port = -1;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    struct option long_options[] = {
            {"bufsize",  required_argument, NULL, 'b'},
            {"servport", required_argument, NULL, 'p'},
            {NULL, 0,                       NULL, 0}
    };

    int opt;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "b:p:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'b':
                bufsize = atoi(optarg);
                break;
            case 'p':
                serv_port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s --bufsize bufsize --servport servport\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    char buf[bufsize];

    if ((bufsize < 0) || (serv_port < 0)) {
        fprintf(stderr, "Usage: %s --bufsize bufsize --servport servport\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, kSize);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serv_port);

    if (bind(lfd, (SADDR *) &servaddr, kSize) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(lfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    while (1) {
        unsigned int clilen = kSize;

        if ((cfd = accept(lfd, (SADDR *) &cliaddr, &clilen)) < 0) {
            perror("accept");
            exit(1);
        }
        printf("connection established\n");

        while ((nread = read(cfd, buf, bufsize)) > 0) {
            write(1, &buf, nread);
        }

        if (nread == -1) {
            perror("read");
            exit(1);
        }
        close(cfd);
    }
}