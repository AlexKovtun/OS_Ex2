//
// Created by alexk on 19/04/2023.
//

#include "ThreadManager.h"
#include <cstdio>
#include <new>

#ifndef _INSTANCE_MANAGER
#define _INSTANCE_MANAGER
ThreadManager *thread_manager = nullptr;

const char *LIB_ERROR = "thread library error: %s";
const char *SYS_ERROR = "system error: %s";

const char *OUT_RANGE_MSG = "the thread id is invalid "
                            "(it needs to be  between 0 to 99)\n";
const char *TIMER_MSG = "Error in the timer\n";
const char *BAD_ALLOC_MSG = "Bad allocation error\n";
const char *EMPTY_SIG_SET_MSG = "Empty signal set error\n";
const char *TER_THREAD_MSG = "problem terminating thread \n";
const char *EXIST_THREAD_MSG =
    "thread library error:no thread with given tid\n";
const char *POS_QUANTUM_SEC_MSG =
    "thread library error: quantum_usecs should be positive\n";

#endif _INSTANCE_MANAGER

int ThreadManager::createThread (int tid, thread_entry_point entry_point)
{
  auto *thread = new UThread (tid, entry_point);
  m_threads.insert ({tid, thread});
  m_ready_threads.push_back (thread);
  return tid;
}

void ThreadManager::switchThread ()
{
  if (m_current_thread != nullptr)
    {
      int retVal = sigsetjmp (m_current_thread->getEnv (), 1);
      if (retVal == 1)
        {
          return;
        }
    }
  thread_manager->updateSleepTime ();
  m_total_num_of_quantum++;
  this->popReadyQ (); //switching the current thread to a new one
  m_current_thread->upRunningQuantum ();
  timerStatus (SIG_UNBLOCK);
  siglongjmp (m_current_thread->getEnv (), 1);//TODO: check return value?
}

int ThreadManager::getAvailableId ()
{
  for (int i = 1; i < m_available_id.size (); ++i)
    {
      if (m_available_id[i] == 0)
        {
          m_available_id[i] = 1;
          return i;
        }
    }
  return FAILURE;
}

ThreadManager::ThreadManager (int num_quantums)
{
  //checks if there is no thread yet(if 0 terminated we terminate the program
  if (m_threads.empty ())
    {
      try
        {
          auto *pt = new UThread ();
          m_threads.insert ({0, pt});
          m_current_thread = pt;
        }
      catch (std::bad_alloc &)
        {
          fprintf (stderr, "system error: couldn't alloc thread\n");
          exit (1);
        }
      m_num_quantums = num_quantums;
      m_total_num_of_quantum = 1;
      installTimer ();
      initId ();
      if (sigemptyset (&set))
        {
          HandleExit (SYS_ERROR, EMPTY_SIG_SET_MSG);
        }
      setTimer ();
      sigaddset (&set, SIGVTALRM);
    }
}

void ThreadManager::initId ()
{
  for (int i = 0; i < MAX_THREAD_NUM; ++i)
    {
      m_available_id.push_back (0);
    }
}

/************************* START OF HANDLING TIMER **************************/

void timer_handler (int sig)
{
  thread_manager->timerStatus (SIG_BLOCK);
  //thread_manager->increaseTotalQuantum ();
  auto *current_thread = thread_manager->getCurrentThread ();
  thread_manager->pushReadyQ (current_thread);
  thread_manager->switchThread ();
  thread_manager->resetTimer ();
}

int ThreadManager::installTimer ()
{
  // Install timer_handler as the signal handler for SIGVTALRM.
  sa.sa_handler = &timer_handler;
  if (sigaction (SIGVTALRM, &sa, NULL) == -1)
    {
      HandleExit (SYS_ERROR, TIMER_MSG);
    }
  return 0;
}

int ThreadManager::setTimer ()
{
  // Configure the timer to expire after 1 sec... */
  timer.it_value.tv_sec = 0; // first time interval, seconds part
  timer.it_value.tv_usec = m_num_quantums;        // first time interval, microseconds part

//  // configure the timer to expire every 3 sec after that.
  timer.it_interval.tv_sec = 0;    // following time intervals, seconds part
  timer.it_interval.tv_usec = m_num_quantums; // following time intervals, microseconds part


  // Start a virtual timer. It counts down whenever this process is executing.
  return resetTimer ();
}

int ThreadManager::resetTimer ()
{
  // Start a virtual timer. It counts down whenever this process is executing.
  if (setitimer (ITIMER_VIRTUAL, &timer, NULL))
    {
      HandleExit (SYS_ERROR, TIMER_MSG);
    }
  return 0;
}

/************************* END OF HANDLING TIMER **************************/

void ThreadManager::pushReadyQ (UThread *threadToInsert)
{
  m_ready_threads.push_back (threadToInsert);
  threadToInsert->setState (THREAD_READY);
}

UThread *ThreadManager::popReadyQ ()
{
  m_current_thread = m_ready_threads.front ();
  m_current_thread->setState (THREAD_RUNNING);
  m_ready_threads.pop_front ();
  return m_current_thread;
}

UThread *ThreadManager::getThreadById (int tid)
{
  auto search = m_threads.find (tid);
  if (search != m_threads.end ())
    {
      return search->second;
    }
  return nullptr;
}

/************************* START OF HANDLING BLOCK **************************/
int ThreadManager::blockThread (int tid)
{
  if (isValidId (tid) == FAILURE)
    {
      return FAILURE;
    }
  UThread *thread_to_block = getThreadById (tid);
  if (thread_to_block == nullptr)
    {
      fprintf (stderr, LIB_ERROR, EXIST_THREAD_MSG);
      return FAILURE;
    }
  int block_state = thread_to_block->getState ();
  if (block_state == THREAD_BLOCKED || block_state == THREAD_SLEEPING_BLOCKED)
    {
      return SUCCESS;
    }
  if (block_state == THREAD_RUNNING)
    {
      thread_to_block->setState (THREAD_BLOCKED);
      switchThread ();
      //++m_total_num_of_quantum;
      resetTimer ();
    }
  else if (block_state == THREAD_READY)
    {
      thread_to_block->setState (THREAD_BLOCKED);
      m_ready_threads.remove (thread_to_block);
    }
  else if (block_state == THREAD_SLEEPING)
    {
      thread_to_block->setState (THREAD_SLEEPING_BLOCKED);
    }
  timerStatus (SIG_UNBLOCK);
  return 0;
}

int ThreadManager::resume (int tid)
{
  if (isValidId (tid) == FAILURE)
    {
      return FAILURE;
    }
  if(!isExistThread(tid)){
      fprintf(stderr,LIB_ERROR,EXIST_THREAD_MSG);
      return FAILURE;
  }
  UThread *resume_thread = getThreadById (tid);
  if (resume_thread == nullptr)
    {
      fprintf (stderr, LIB_ERROR, EXIST_THREAD_MSG);
      return FAILURE;
    }

  int state = resume_thread->getState ();
  if (state == THREAD_READY || state == THREAD_RUNNING)
    {
      return SUCCESS;
    }
  else if (state == THREAD_BLOCKED)
    {
      resume_thread->setState (THREAD_READY);
      m_ready_threads.push_back (resume_thread);
    }
  else if (state == THREAD_SLEEPING_BLOCKED)
    {
      resume_thread->setState (THREAD_SLEEPING);
    }
  return SUCCESS;
}

/************************* END OF HANDLING BLOCK **************************/


int ThreadManager::terminateThread (int tid)
{
  if (isValidId (tid) == FAILURE)
    {
      return FAILURE;
    }
  if (tid != 0)
    {
      try
        {
          return terminateByState (tid);
        }
      catch (std::exception)
        {
          HandleExit (SYS_ERROR, TER_THREAD_MSG);
        }
    }
  else{
      terminateAll ();
  }
//  catch (std::exception)
//    {
//      fprintf (stderr, SYS_ERROR,
//               "problem clearing, couldn't trminate ""all\n");
//      exit (1);
//    }
  exit (0);
}

int ThreadManager::terminateAll ()
{
    auto tmp = this->m_threads.begin();
    for(;tmp != this->m_threads.end(); tmp ++){
        delete tmp->second;
        tmp->second = nullptr;
    }

  m_ready_threads.clear ();
    return SUCCESS;
}

int ThreadManager::terminateByState (int tid)
{
  if (isValidId (tid) == FAILURE)
    {
      return FAILURE;
    }
  auto tmp = m_threads.find (tid);
  if (tmp == m_threads.end ())
    {
      fprintf (stderr, LIB_ERROR, EXIST_THREAD_MSG);
      return FAILURE;
    }

  UThread *to_terminate = tmp->second; //if tid not in map throw error
  m_threads.erase (tid);
  if (to_terminate == nullptr)
    {
      fprintf (stderr, LIB_ERROR, EXIST_THREAD_MSG);
      return FAILURE;
    }
  m_available_id[tid] = 0;
  int thread_state = to_terminate->getState ();

  if (thread_state == THREAD_READY)
    m_ready_threads.remove (to_terminate);
  else if (thread_state == THREAD_SLEEPING_BLOCKED ||
           thread_state == THREAD_SLEEPING)
    m_sleeping_threads.erase (tid);
  else if (thread_state == THREAD_RUNNING)
    {
      //++m_total_num_of_quantum;
      m_current_thread = nullptr;
      switchThread ();
    }
  delete (to_terminate);
  to_terminate = nullptr;
  return SUCCESS;
}

int ThreadManager::threadSleep (int num_quantums)
{
  if (num_quantums <= 0)
    {
      fprintf (stderr, LIB_ERROR, POS_QUANTUM_SEC_MSG);
      return FAILURE;
    }
  timerStatus (SIG_BLOCK);
  m_current_thread->setState (THREAD_SLEEPING);
  m_current_thread->setSleepTime (num_quantums);
  m_sleeping_threads.insert ({m_current_thread->getId (), m_current_thread});
  switchThread ();
  //++m_total_num_of_quantum;
  resetTimer ();
  timerStatus (SIG_UNBLOCK);
  return SUCCESS;
}

void ThreadManager::updateSleepTime ()
{
  std::list<int> return_back;
  for (const auto &thread: m_sleeping_threads)
    {
      thread.second->decreaseTimeLeft ();
      if (thread.second->getQuantumSleep () == 0)
        {
          if (thread.second->getState () != THREAD_SLEEPING_BLOCKED)
            {
              thread.second->setState (THREAD_READY);
              return_back.push_back (thread.first);
            }
          else
            {
              thread.second->setState (THREAD_BLOCKED);
            }
        }
    }
  for (const auto &tid: return_back)
    {
      m_sleeping_threads.erase (tid);
      auto *thread_to_return = getThreadById (tid);
      pushReadyQ (thread_to_return);
    }
}

void ThreadManager::timerStatus (int block_flag)
{
  if (sigprocmask (block_flag, &set, nullptr))
    {
      HandleExit (SYS_ERROR, TIMER_MSG);
    }
}

int ThreadManager::isValidId (int tid)
{
  if (tid < 0 || tid > MAX_THREAD_NUM)
    {
      fprintf (stderr, LIB_ERROR, OUT_RANGE_MSG);
      return FAILURE;
    }
  return SUCCESS;
}

void ThreadManager::HandleExit (const char *type, const char *msg)
{
  terminateAll ();
  fprintf (stderr, type, msg);
  exit (1);
}

bool ThreadManager::isExistThread(int tid){
  auto tmp = m_threads.find (tid);
  if (tmp == m_threads.end ())
    {
      fprintf (stderr, LIB_ERROR, EXIST_THREAD_MSG);
      return false;
    }
  return true;
}