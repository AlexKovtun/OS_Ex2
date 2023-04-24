//
// Created by alexk on 19/04/2023.
//

#include <setjmp.h>
#include <signal.h>


#include "uthreads.h"
#include "UThread.h"


#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
  address_t ret;
  asm volatile("xor    %%fs:0x30,%0\n"
               "rol    $0x11,%0\n"
  : "=g" (ret)
  : "0" (addr));
  return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#endif


//this will create  the main thread
UThread::UThread ()
{
  //TODO: check existence of main thread
  this->m_tid = 0;
  this->m_stack_size = STACK_SIZE;
  this->m_state = THREAD_RUNNING;
  this->running_quantum_time = 1;
  sigsetjmp(this->m_env, 1);
}

UThread::UThread (int tid, thread_entry_point entry_point)
{
  this->m_tid = tid;
  this->m_stack_size = STACK_SIZE;
  this->m_stack = new char[STACK_SIZE];
  this->m_state = THREAD_READY;
  this->running_quantum_time = 0;
  auto pc = (address_t) entry_point;
  address_t sp = (address_t) this->m_stack + STACK_SIZE - sizeof(address_t);

  // initializes env[tid] to use the right stack, and to run from the
  // function 'entry_point', when we'll use siglongjmp to jump into the thread.

  sigsetjmp(this->m_env, 1);
  (this->m_env->__jmpbuf)[JB_SP] = translate_address(sp);
  (this->m_env->__jmpbuf)[JB_PC] = translate_address(pc);
  sigemptyset(&this->m_env->__saved_mask);//TODO: check success
}
UThread::~UThread ()
{
  delete [] m_stack;
}
int UThread::save_current_state ()
{
  return sigsetjmp(this->m_env, 1);// todo: figure out mask param
}
sigjmp_buf& UThread::getEnv ()
{
  return this->m_env;
}
int UThread::getId ()
{
  return m_tid;
}
void UThread::setState (int state)
{
  m_state = state;
}
int UThread::getState ()
{
  return m_state;
}

int UThread::getQuantumSleep(){
  return m_quantum_left_sleep;
}
void UThread::decreaseTimeLeft ()
{
  --m_quantum_left_sleep;
}
void UThread::setSleepTime (int num_quantums)
{
  m_quantum_left_sleep = num_quantums;
}

int UThread::getRunningQuantum(){
  return running_quantum_time;
}
int UThread::upRunningQuantum(){
  running_quantum_time++;
}

