#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <getopt.h>


#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
    int sockfd, n;
    char ipadr[16];
    int serv_port = -1;
    int bufsize = -1;
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

    char mesg[bufsize];

    if ((bufsize < 0) || (serv_port < 0)) {
        fprintf(stderr, "Usage: %s --bufsize bufsize --servport servport\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        exit(1);
    }

    memset(&servaddr, 0, SLEN);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serv_port);

    if (bind(sockfd, (SADDR *) &servaddr, SLEN) < 0) {
        perror("bind problem");
        exit(1);
    }
    printf("SERVER starts...\n");

    while (1) {
        unsigned int len = SLEN;

        if ((n = recvfrom(sockfd, mesg, bufsize, 0, (SADDR *) &cliaddr, &len)) < 0) {
            perror("recvfrom");
            exit(1);
        }
        mesg[n] = 0;

        printf("REQUEST %s      FROM %s : %d\n", mesg,
               inet_ntop(AF_INET, (void *) &cliaddr.sin_addr.s_addr, ipadr, 16),
               ntohs(cliaddr.sin_port));

        if (sendto(sockfd, mesg, n, 0, (SADDR *) &cliaddr, len) < 0) {
            perror("sendto");
            exit(1);
        }
    }
}