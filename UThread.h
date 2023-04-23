//
// Created by alexk on 18/04/2023.
//

#ifndef _UTHREAD_H_
#define _UTHREAD_H_

#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <unistd.h>




#define THREAD_READY 0
#define THREAD_RUNNING 1
#define THREAD_BLOCKED 2
#define THREAD_SLEEPING 3
#define THREAD_SLEEPING_BLOCKED 4

//pointer to function that accepts zero or many parameters and return void
using thread_entry_point = void (*)();
//typedef void (*thread_entry_point)(void);


struct UThread {
  void setState (int state);
  int getState ();
  void decreaseTimeLeft ();
  void setSleepTime(int num_quantums);
 public:
  UThread ();
  /**
   * creating thread with stack and state
   * @param tid thread id that's free to capture
   * @param stack_size size of the stack for this thread
   * @param entry_point pointer to function that we running on it the thread
   */
  UThread (int tid, thread_entry_point entry_point);
  /**
   *  destructor that frees the stack memory
   */
  ~UThread();

  /**
   * saves current state of the thread
   * if succeeded doing it return 0, otherwise return 0
   */
  int save_current_state();
  sigjmp_buf& getEnv();
  int getId();
  int getQuantumSleep ();
  int getRunningQuantum();
  int upRunningQuantum();

 private:
  int m_tid;
  int m_state;
  char *m_stack;
  int m_stack_size;
  int m_quantum_left_sleep;
  sigjmp_buf m_env;
  int running_quantum_time;
};
#endif //_UTHREAD_H_
