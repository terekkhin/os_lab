#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", optional_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};
                                      
    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);	
    if (c == -1) break;
    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            // error handling
            break;
          case 3:
            timeout = atoi(optarg);
            break;
          case 4:
            with_files = true;
            break;
  
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
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

  void alarm_signal(int var)
  {
    kill(0, SIGKILL);
  }
  
  if (timeout != -1)
  {
    signal(SIGALRM, alarm_signal);
    alarm(timeout);
  }
  
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  
  int batch_size = array_size / pnum;
  int start_index;
  int end_index;
  struct MinMax local_min_max;

  FILE *file_min;
  file_min = fopen("min.txt", "w");
  fclose(file_min);
  FILE *file_max;
  file_max = fopen("max.txt", "w");
  fclose(file_max);
  
  int val_in_file;
  
  int (*min_pipes)[2] = malloc(sizeof(int[2]) * pnum);
  int (*max_pipes)[2] = malloc(sizeof(int[2]) * pnum);
  for(int i = 0; i < pnum; i++){
    if(pipe(min_pipes[i]) == -1 || pipe(max_pipes[i]) == -1){
      perror("pipe");
      return 1;
    }
  }


  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        // parallel somehow
        
        if (with_files) {
          // use files here
          file_min = fopen("min.txt", "a+");
  	  file_max = fopen("max.txt", "a+");
  	  
          start_index = i * batch_size;
          end_index = (i == pnum - 1) ? array_size : (i+1) * batch_size;
          local_min_max = GetMinMax(array, start_index, end_index);

          fprintf(file_min, "%d\n", local_min_max.min);
          
          fprintf(file_max, "%d\n", local_min_max.max);
          
          fclose(file_min);
          fclose(file_max);
        } else {
          // use pipe here
          close(min_pipes[i][0]);
          close(max_pipes[i][0]);
          
          start_index = i * batch_size;
          end_index = (i == pnum - 1) ? array_size : (i+1) * batch_size;
          local_min_max = GetMinMax(array, start_index, end_index);
          
          write(min_pipes[i][1], &local_min_max.min, sizeof(int));
          write(max_pipes[i][1], &local_min_max.max, sizeof(int));

          close(min_pipes[i][1]);
          close(max_pipes[i][1]);
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }
  while (active_child_processes > 0) {
    // your code here
    int status;
    wait(&status);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  file_min = fopen("min.txt", "r");
  file_max = fopen("max.txt", "r");
  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      fscanf(file_min, "%d", &min);
      fscanf(file_max, "%d", &max);
    } else {
      // read from pipes
      read(min_pipes[i][0], &min, sizeof(int));
      read(max_pipes[i][0], &max, sizeof(int));
      close(min_pipes[i][0]);
      close(max_pipes[i][0]);
    }
    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }
  fclose(file_min);
  fclose(file_max);
  
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(min_pipes);
  free(max_pipes);
  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
