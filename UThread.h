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


#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

#endif

#define THREAD_READY 0
#define THREAD_RUNNING 1
#define THREAD_BLOCKED 2

//pointer to function that accepts zero or many parameters and return void
using thread_entry_point = void (*)();


struct UThread {
  int getQuantumTime ();
  void setState (int state);
  bool getState ();
 public:
  UThread (int num_quantums);
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

 private:
  int m_tid;
  int m_state;
  int m_quantum_time;
  address_t  m_pc;
  address_t m_sp;
  char *m_stack;
  int m_stack_size;
  sigjmp_buf m_env;

};
#endif //_UTHREAD_H_
