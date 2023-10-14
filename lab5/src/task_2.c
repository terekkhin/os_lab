#include <stdio.h>
#include <pthread.h>
#include <getopt.h>
#include <stdbool.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned long long result = 1;

struct Args{
  int k;
  int pnum;
  int mod;
  int thread_id;
};

void *factorial_mod(void *params) {
    struct Args *p = (struct Args *)params;
    int k = p->k;
    int mod = p->mod;
    int start = (k / p->pnum) * p->thread_id + 1;
    int end = (p->thread_id == p->pnum - 1) ? k : (k / p->pnum) * (p->thread_id + 1);
    unsigned long long local_result = 1;

    for (int i = start; i <= end; i++) {
        local_result = (local_result * i) % mod;
    }

    pthread_mutex_lock(&mutex);
    result = (result * local_result) % mod;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char **argv)
{
   int k = -1;
   int pnum = -1;
   int mod = -1;
   while (true) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {{"k", required_argument, 0, 0},
                                    {"pnum", required_argument, 0, 0},
                                    {"mod", required_argument, 0, 0},
                                    {0, 0, 0, 0}};       
    int option_index = 0;
    int c = getopt_long(argc, argv, "k", options, &option_index);	
    if (c == -1) break;
    switch(c){
      case 0:
        switch(option_index){
          case 0:
            k = atoi(optarg);
            break;
          case 1:
            pnum = atoi(optarg);
            break;
          case 2:
            mod = atoi(optarg);
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
     }
   }
   if (k == -1 || pnum == -1 || mod == -1) {
    printf("Usage: %s --k \"num\" --pnum \"num\" --mod \"num\" \n",
           argv[0]);
    return 1;
  }
   
   pthread_t threads[pnum];
   struct Args args[pnum];
   
   
   for (int i = 0; i < pnum; i++) {
        args[i].k = k;
        args[i].pnum = pnum;
        args[i].mod = mod;
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, factorial_mod, &args[i]);
    }

    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%d! mod %d = %llu\n", k, mod, result);

    return 0;
}
