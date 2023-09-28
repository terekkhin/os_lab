#include <stdio.h>
#include <unistd.h>

int main(int argc, int *argv[])
{
  int pid = fork();
  if(pid == 0)
  {
    execv("sequential_min_max", argv);
  }
  return 0;
}
