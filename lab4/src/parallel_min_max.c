#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t child_pids[16];
int active_child_processes = 0;

void kill_children() {
    for (int i = 0; i < active_child_processes; i++) {
        printf("Killing child %d\n", child_pids[i]);
        kill(child_pids[i], SIGKILL);
    }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int by_files = 0;
  bool with_files = false;
  int pipefd[2]; //0 - чтение, 1 - запись
  int timeout = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed is a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size is a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (array_size <= 0) {
              printf("pnum is a positive number\n");
              return 1;
            }
            break;
          case 3:
            by_files = atoi(optarg);
            if (by_files != 0 && by_files != 1) {
              printf("by_files must be 0 or 1\n");
              return 1;
            }
            with_files = by_files == 1 ? 1 : 0;
            break;
          case 4:
            if (optarg == NULL) {
              timeout = 0;
              break;
            }
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("timeout is a positive number\n");
              return 1;
            }
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("%d %d Has at least one no option argument\n", argc, optind);
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  signal(SIGALRM, &kill_children); 
  alarm(timeout);


  if (!with_files) {
      if (pipe(pipefd) == -1) {
          perror("pipe");
          return 1;
      }
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      child_pids[active_child_processes] = child_pid;
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        unsigned int begin = i * (array_size / pnum);
        unsigned int end;
        if((i + 1) == pnum) end = array_size;
        else end = (i + 1) * (array_size / pnum);
        struct MinMax min_max_w = GetMinMax(array, begin, end);

        if (with_files) {
          FILE *fp;
          if((fp = fopen("data.txt", "a")) != NULL)
          {
            fwrite(&min_max_w, sizeof(struct MinMax), 1, fp);
            fclose(fp);
          }
        } else {
          close(pipefd[0]);
          write(pipefd[1], &min_max_w, sizeof(struct MinMax));
          close(pipefd[1]);
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    struct MinMax min_max_r;

    if (with_files) {
      FILE *fp;
      if((fp = fopen("data.txt", "r")) != NULL) {
        fseek(fp, i * sizeof(struct MinMax), SEEK_SET);
        fread(&min_max_r, sizeof(struct MinMax), 1, fp);
        fclose(fp);
      }
    } else {
      close(pipefd[1]);
      read(pipefd[0], &min_max_r, sizeof(struct MinMax));
    }

    if (min_max_r.min < min_max.min) min_max.min = min_max_r.min;
    if (min_max_r.max > min_max.max) min_max.max = min_max_r.max;
  }


  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  remove("data.txt");
  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
