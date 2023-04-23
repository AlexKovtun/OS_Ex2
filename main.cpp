#include <iostream>
#include <unistd.h>
#include "uthreads.h"

void thread1(void)
{
  printf("Starting thread: %d\n",1);
  fflush (stdout);
  while (1){
  }
}


void thread2(void)
{
  printf ("thread is sleeping: %d\n", 2);
  uthread_sleep(20);
  printf ("thread is got back: %d\n", 2);
  printf ("Starting thread: %d\n", 2);
  fflush (stdout);
  int i = 0;

  while (1)
    {
    }
}

void thread3(void)
{
  while (1)
    {
    }
}




int main ()
{
  std::cout << "Hello, World!" << std::endl;
  uthread_init( 1);
  std::cout << "          spawn f at (1) " << uthread_spawn(thread1) << std::endl;
  std::cout << "          spawn g at (2) " << uthread_spawn(thread2) << std::endl;
  std::cout << "          spawn g at (3) " << uthread_spawn(thread3) << std::endl;
  for (;;)
    {
      //printf("we are in main therad!\n");
    }
  return 0;
}



