#include <iostream>
#include <unistd.h>
#include "uthreads.h"

void thread0(void)
{
  int i = 0;
  while (1)
    {
      ++i;
      printf("in thread0 (%d)\n", i);
      fflush (stdout);
      if (i % 3 == 0)
        {
          printf("thread0: yielding\n");
          //yield();
        }
      usleep(1);
    }
}


void thread1(void)
{
  int i = 0;
  while (1)
    {
      ++i;
      printf("in thread1 (%d)\n", i);
      fflush (stdout);
      if (i % 5 == 0)
        {
          printf("thread1: yielding\n");
         // yield();
        }
      usleep(1);
    }
}


int main ()
{
  std::cout << "Hello, World!" << std::endl;
  uthread_init( 1);
  std::cout << "          spawn f at (1) " << uthread_spawn(thread0) << std::endl;
  std::cout << "          spawn g at (2) " << uthread_spawn(thread1) << std::endl;
  for (;;)
    {

    }
  return 0;
}
