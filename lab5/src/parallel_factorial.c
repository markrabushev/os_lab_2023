#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

unsigned long k;   
int pnum;
unsigned long mod;
unsigned long long result = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

// struct SumArgs {
//     int *array;
//     int begin;
//     int end;
// };
// int Sum(const struct SumArgs *args) {
//     int sum = 0;
//     for (int i = args->begin; i < args->end; ++i) {
//         sum += args->array[i];
//     }
//     return sum;
// }

struct FactorialBE {
    unsigned long begin, end;
};

int Factorial(const struct FactorialBE *args) {
    unsigned long long partialResult = 1;
    for (unsigned long i = args->begin; i <= args->end; i++)
        partialResult = (partialResult * i) % mod;

    pthread_mutex_lock(&mut);
    result = (result * partialResult) % mod;
    pthread_mutex_unlock(&mut);
    return 1;
}
void *ThreadFactorial(void *args) {
  struct FactorialBE *f_args = (struct FactorialBE *)args;
  return (void *)(size_t)Factorial(f_args);
}

int main(int argc, char **argv) {

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            k = atoi(optarg);
            break;
          case 1:
            pnum = atoi(optarg);
            break;
          case 2:
            mod = atoi(optarg);
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

  pthread_t threads[pnum];  
  
  struct FactorialBE args[pnum];

  unsigned int split_size = k / pnum;
  for (int i = 0; i < pnum; i++) {
      args[i].begin = i * split_size + 1;
      args[i].end = (i == pnum - 1) ? k : args[i].begin + split_size - 1;
  }
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i]);
  }

  for (int i = 0; i < pnum; i++) {
    pthread_join(threads[i], NULL);
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;


  printf("%ld! mod %ld равен %lld. Затраченное время: %fms\n", k, mod, result, elapsed_time);
  fflush(NULL);
  return 0;
}
