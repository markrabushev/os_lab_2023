#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>
#include <pthread.h>

#include "sum.h"
#include "utils.h"

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  static struct option options[] = {
            {"seed",        required_argument, 0, 0},
            {"array_size",  required_argument, 0, 0},
            {"threads_num", required_argument, 0, 0},
            {0, 0, 0, 0}
  };

  int option_index = 0;
  while (getopt_long(argc, argv, "", options, &option_index) != -1) {
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
              threads_num = atoi(optarg);
              if (array_size <= 0) {
                printf("pnum is a positive number\n");
                return 1;
              }
               break;

          default:
              printf("Invalid options amount\n");
              return 1;
      }
  }
  if (threads_num == 0 || array_size == 0) {
      printf("Invalid arguments\n");
      return 1;
  }

  pthread_t threads[threads_num];
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  struct SumArgs args[threads_num];

  unsigned int split_size = array_size / threads_num;
  for (uint32_t i = 0; i < threads_num; i++) {
      args[i].array = array;
      args[i].begin = i * split_size;
      args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * split_size;
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (uint32_t i = 0; i < threads_num; i++) {
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;


  free(array);
  printf("Total: %d\n", total_sum);
  //printf("Time spent: %.5f milliseconds\n", elapsed_time);
  printf("Elapsed time: %fms\n", elapsed_time);
  return 0;
}
