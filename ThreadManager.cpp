//
// Created by alexk on 19/04/2023.
//

#include "ThreadManager.h"

#ifndef _INSTANCE_MANAGER
#define _INSTANCE_MANAGER
ThreadManager *thread_manager = nullptr;
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
      int retVal = sigsetjmp(m_current_thread->getEnv (), 1);
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
  return 0;
}

ThreadManager::ThreadManager (int num_quantums)
{
  //checks if there is no thread yet(if 0 terminated we terminate the program
  if (m_threads.empty ())
    {
      auto *pt = new UThread ();
      m_threads.insert ({0, pt});
      m_current_thread = pt;
      m_num_quantums = num_quantums;
      m_total_num_of_quantum = 1;
      installTimer ();
      initId ();
      sigemptyset (&set);
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
      //printf ("the sigaction error reason is: %s\n", errno);
      printf ("the sigaction error reason is\n");
      return -1;
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
      printf ("timer error.");
      return -1;
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
  UThread *thread_to_block = getThreadById (tid);
  if (thread_to_block == nullptr)
    {
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
  UThread *resume_thread = getThreadById (tid);
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
//TODO maybe this is error
  if (tid < 0 || tid > MAX_THREAD_NUM)
    {
      return -1;
    }
  if (tid != 0)
    {
      return terminateByState (tid);
    }
  for (auto const &thrd: m_threads)
    {
      delete (thrd.second);
    }
  m_threads.clear ();
  m_ready_threads.clear ();
  exit (0);
}

int ThreadManager::terminateByState (int tid)
{
  UThread *to_terminate = m_threads.at (tid); //if tid not in map throw error
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
      switchThread ();
    }
  delete (to_terminate);
  m_threads.erase (tid);
  return SUCCESS;
}

void ThreadManager::threadSleep (int num_quantums)
{
  timerStatus (SIG_BLOCK);
  m_current_thread->setState (THREAD_SLEEPING);
  m_current_thread->setSleepTime (num_quantums);
  m_sleeping_threads.insert ({m_current_thread->getId (), m_current_thread});
  switchThread ();
  //++m_total_num_of_quantum;
  resetTimer ();
  timerStatus (SIG_UNBLOCK);
}

void ThreadManager::updateSleepTime ()
{
  std::list<int> return_back;
  for (const auto &thread: m_sleeping_threads)
    {
      thread.second->decreaseTimeLeft ();
      if (thread.second->getQuantumSleep () <= 0)
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
      exit (1);//TODO: error message
    }
}





