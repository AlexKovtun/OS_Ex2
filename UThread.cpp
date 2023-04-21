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
UThread::UThread (int num_quantums)
{
  //TODO: check existence of main thread
  this->m_tid = 0;
  this->m_stack_size = STACK_SIZE;
  this->m_stack = new char[STACK_SIZE];
  this->m_state = THREAD_RUNNING;
  this->m_quantum_time = num_quantums;
  sigsetjmp(this->m_env, 1);
}

UThread::UThread (int tid, thread_entry_point entry_point)
{
  this->m_tid = tid;
  this->m_stack_size = STACK_SIZE;
  this->m_stack = new char[STACK_SIZE];
  this->m_state = THREAD_READY;

  this->m_sp = (address_t) this->m_stack + STACK_SIZE - sizeof(address_t);
  this->m_pc = (address_t) entry_point;
  // initializes env[tid] to use the right stack, and to run from the
  // function 'entry_point', when we'll use siglongjmp to jump into the thread.

  sigsetjmp(this->m_env, 1);
  (this->m_env->__jmpbuf)[JB_SP] = translate_address(this->m_sp);
  (this->m_env->__jmpbuf)[JB_PC] = translate_address(this->m_sp);
  sigemptyset(&this->m_env->__saved_mask);
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
int UThread::getQuantumTime ()
{
  return this->m_quantum_time;
}
int UThread::getId ()
{
  return m_tid;
}


