#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

//./udpserver --bufsize 1024 --servport 20001
//./udpclient --bufsize 1024 --servaddr 127.0.0.1 --servport 20001
#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
    int sockfd, n;
    int serv_port = -1;
    int bufsize = -1;
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

    char sendline[bufsize], recvline[bufsize + 1];

    if ((bufsize < 0) || (serv_port < 0) || (strlen(address) == 0)) {
        fprintf(stderr, "Usage: %s --bufsize bufsize --servaddr servaddr --servport servport\n", argv[0]);
        return EXIT_FAILURE;
    }


    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serv_port);

    if (inet_pton(AF_INET, address, &servaddr.sin_addr) < 0) {
        perror("inet_pton problem");
        exit(1);
    }
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        exit(1);
    }

    write(1, "Enter string\n", 13);

    while ((n = read(0, sendline, bufsize)) > 0) {
        if (sendto(sockfd, sendline, n, 0, (SADDR *) &servaddr, SLEN) == -1) {
            perror("sendto problem");
            exit(1);
        }

        if (recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL) == -1) {
            perror("recvfrom problem");
            exit(1);
        }

        printf("REPLY FROM SERVER= %s\n", recvline);
    }
    close(sockfd);
}