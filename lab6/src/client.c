#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
  char ip[255];
  int port;
};

/*
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}
*/

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

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
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
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
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

  FILE * f;
  FILE * f1;
  /*
  size_t lines_count = 0;
  f = fopen(servers, "r");
  while (! feof(f))
  {
      if (fgetc(f) == '\n')
          lines_count++;
  }
  lines_count++;
  fclose(f);
  */
  int servers_num = 3;
  struct Server *to = malloc(sizeof(struct Server) * servers_num);

  f1 = fopen(servers, "r");
  
  char * line = NULL;
  size_t len = 0;

  for (int j = 0; j < servers_num; j++)
  {
      getline(&line, &len, f1);
      memcpy(to[j].ip, line, sizeof(line) + 1);
      getline(&line, &len, f1);
      ConvertStringToUI64(line, &to[j].port);
  }
  //printf("0\n%s\n", to[0].ip);
  //printf("1\n%s\n", to[1].ip);
  //printf("2\n%s\n", to[2].ip);
  //printf("0\n%d\n", to[0].port);
  //printf("1\n%d\n", to[1].port);
  //printf("2\n%d\n", to[2].port);

  fclose(f1);
  if (line)
      free(line);
  
  uint64_t answer = 1;
  uint64_t server_response = 0;

  // TODO: work continiously, rewrite to make parallel
  int *sck = malloc(sizeof(int) * servers_num); 
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    sck[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck[i], (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    // TODO: for one server
    // parallel between servers
    uint64_t begin = (k / servers_num) * i + 1;
    uint64_t end = (i == servers_num - 1) ? k : (k / servers_num) * (i + 1);

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck[i], task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }
    }
    
  for (int i = 0; i < servers_num; i++) {
    char response[sizeof(uint64_t)];
    if (recv(sck[i], response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }
   
    // TODO: from one server
    // unite results
    memcpy(&server_response, response, sizeof(uint64_t));
    answer = MultModulo(answer, server_response, mod);

    close(sck[i]);
  }
  printf("answer: %llu\n", answer);
  free(to);

  return 0;
}
