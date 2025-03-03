#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>

#include "MultModulo.h"
//./client --k 12 --mod 13 --servers servers.txt
//./client --k 100 --mod 202 --servers servers.txt
uint64_t k = -1;
uint64_t mod = -1;
int servers_number = -1;

pid_t child_pids[64];
int active_child_processes = 0;

struct Server {
  char ip[255];
  int port;
  int number;
};


bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

int countServers(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }
    int count = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == ':') {
            count++;
        }
    }
    fclose(file);
    return count;
}

int parse(const char *servers, struct Server *to) {
    FILE *file = fopen(servers, "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }
    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* ip = strtok(line, ":");
        char* portStr = strtok(NULL, ":");
        strncpy(to[i].ip, ip, sizeof(to[i].ip)-1);
        to[i].ip[sizeof(to[i].ip)-1] = '\0';
        to[i].port = atoi(portStr);
        to[i].number = i;
        i++;
    }

    fclose(file);
    return 0;
}

void *perform(void *args) {
    struct Server *to = (struct Server *) args;
    int threadNumber = to->number;

    struct hostent const *hostname = gethostbyname(to->ip);
    sleep(1);
    if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", to->ip);
        exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to->port);
    server.sin_addr.s_addr = *((unsigned long *) hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        exit(1);
    }

    if (connect(sck, (struct sockaddr *) &server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        exit(1);
    }

    uint64_t partitionSize = k / servers_number;
    uint64_t begin = threadNumber * partitionSize + 1;
    uint64_t end = (threadNumber == servers_number - 1) ? k : begin + partitionSize - 1;

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));
    printf("Thread %d sends begin: %lu end: %lu mod: %lu to %s:%d\n", threadNumber, begin, end, mod, to->ip, to->port);

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        exit(1);
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Recieve failed\n");
    }

    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    close(sck);

    return (void *) answer;
}

int main(int argc, char **argv) {
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        if (!ConvertStringToUI64(optarg, &k)) {
          printf("k error\n");
          return 1;
        }
        break;
      case 1:
        if (!ConvertStringToUI64(optarg, &mod)) {
          printf("mod error\n");
          return 1;
        }
        break;
      case 2:
        memcpy(servers, optarg, strlen(optarg));
        if (!fopen(servers, "r")) {
          printf("servers error\n");
          return 1;
        }
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // TODO: for one server here, rewrite with servers from file
  servers_number = countServers(servers);
  struct Server *to = malloc(sizeof(struct Server) * servers_number);
  if (parse(servers, to) == 1) {
    free(to);
    return 1;
  }

  for (int i = 0; i < servers_number; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      if(child_pid != 0){
        child_pids[active_child_processes] = child_pid;
        active_child_processes += 1;
        int status = 1;
        waitpid(child_pid, &status, WNOHANG);
      }
      if (child_pid == 0) {
        char portName[10];
        sprintf(portName, "%d", to[i].port);

        execl("./server", "server", "--port", portName, "--tnum", "6", NULL);
        perror("Failed to start server");
        exit(1);
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }
  
  pthread_t threads[servers_number];
  for (int i = 0; i < servers_number; i++) {
        printf("Thread %d is running at %s:%d\n", i, to[i].ip, to[i].port);
        if (pthread_create(&threads[i], NULL, perform, (void *) &to[i])) {
            printf("Error: pthread_create failed!\n");
            return 1;
      }
  }

  uint64_t total = 1;
  for (uint32_t i = 0; i < servers_number; i++) {
      uint64_t result = 0;
      pthread_join(threads[i], (void **) &result);
      total = MultModulo(total, result, mod);
      printf("Thread %d, total: %lu, result: %lu\n", i, total, result);
  }

  free(to);
  
  printf("Stopping servers...\n");
  for (int i = 0; i < active_child_processes; i++) {
        kill(child_pids[i], SIGKILL);
    }
  printf("Answer: %lu\n", total);

  return 0;
}