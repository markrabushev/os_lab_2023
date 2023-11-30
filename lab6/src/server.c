#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#include "MultModulo.h"

struct FactorialArgs {
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ changes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// uint64_t Factorial(const struct FactorialArgs *data) {
//     uint64_t ans = 1;
// //    printf("begin: %lu, end: %lu, mod: %lu\n", data->begin, data->end, data->mod);

//     for (uint64_t i = data->begin; i <= data->end; i++)
//         ans = (ans * i) % data->mod;

// //    printf("ans: %lu\n", ans);

//     return ans;
// }
uint64_t Factorial(const struct FactorialArgs *args) {
    unsigned long long partialResult = 1;
    for (unsigned long long i = args->begin; i <= args->end; i++)
        partialResult = (partialResult * i) % args->mod;

    return partialResult;
}


void *ThreadFactorial(void *args) {
    struct FactorialArgs const *fargs = (struct FactorialArgs *) args;
    return (void *) (uint64_t *) Factorial(fargs);
}


int main(int argc, char **argv) {
    int tnum = -1;
    int port = -1;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"port", required_argument, 0, 0},
                                          {"tnum", required_argument, 0, 0},
                                          {0, 0,                      0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        port = atoi(optarg);
                        if (port <= 0 || port > 65535) {
                            printf("port argument error\n");
                            return 1;
                        }
                        break;
                    case 1:
                        tnum = atoi(optarg);
                        if (tnum < 0 || tnum > 10) {
                            printf("tnum argument error\n");
                            return 1;
                        }
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            }
                break;

            case '?':
                printf("Unknown argument\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (port == -1 || tnum == -1) {
        fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Can not create server socket!");
        return 1;
    }

    printf("Port: %d, Threads: %d\n", port, tnum);


    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t) port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    int err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err < 0) {
        fprintf(stderr, "Can not bind to socket!");
        return 1;
    }

    err = listen(server_fd, 128);
    if (err < 0) {
        fprintf(stderr, "Could not listen on socket\n");
        return 1;
    }

    printf("Server listening at %d\n", port);

    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr, "Could not establish new connection\n");
            continue;
        }

        while (true) {
            unsigned int buffer_size = sizeof(uint64_t) * 3;
            char from_client[buffer_size];
            int read = recv(client_fd, from_client, buffer_size, 0);

            if (!read)
                break;
            if (read < 0) {
                fprintf(stderr, "Client read failed\n");
                break;
            }
            if (read < buffer_size) {
                fprintf(stderr, "Client send wrong data format\n");
                break;
            }

            pthread_t threads[tnum];

            uint64_t begin = 0;
            uint64_t end = 0;
            uint64_t mod = 0;
            memcpy(&begin, from_client, sizeof(uint64_t));
            memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
            memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

            fprintf(stdout, "Receive at port %d: %lu %lu %lu\n", port, begin, end, mod);
//            printf("Port: %d receives: %lu %lu %lu\n", port, begin, end, mod);

            struct FactorialArgs args[tnum];
            uint64_t partitionSize = (end - begin) / tnum + 1;
            for (uint32_t i = 0; i < tnum; i++) {
                args[i].begin = begin + i * partitionSize;
                args[i].end = (i == tnum - 1) ? end : begin + (i + 1) * partitionSize - 1;
                args[i].mod = mod;
                printf("Factorial Thread %d, begin: %lu, end: %lu, mod: %lu\n", i, args[i].begin, args[i].end, args[i].mod);
                if (pthread_create(&threads[i], NULL, ThreadFactorial,
                                   (void *) &args[i])) {
                    printf("Error: pthread_create failed!\n");
                    return 1;
                }
            }
            uint64_t total = 1;
            for (uint32_t i = 0; i < tnum; i++) {
                uint64_t result = 0;
                pthread_join(threads[i], (void **) &result);
                total = MultModulo(total, result, mod);
                printf("Thread %d, total: %lu, result: %lu\n", i, total, result);
            }

//            printf("Total: %llu\n", total);

            char buffer[sizeof(total)];
            memcpy(buffer, &total, sizeof(total));
            err = send(client_fd, buffer, sizeof(total), 0);
            if (err < 0) {
                fprintf(stderr, "Can't send data to client\n");
                break;
            }
        }

        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }

    return 0;
}