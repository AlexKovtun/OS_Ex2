//
// Created by alexk on 19/04/2023.
//

#include "ThreadManager.h"

#ifndef _INSTANCE_MANAGER
#define _INSTANCE_MANAGER
ThreadManager *thread_manager = nullptr;
#endif _INSTANCE_MANAGER

void ThreadManager::createThread (int tid, thread_entry_point entry_point)
{
  auto *thread = new UThread (tid, entry_point);
  m_threads.insert ({tid, thread});
  m_ready_threads.push_back (thread);
}

void ThreadManager::switchThread ()
{
  thread_manager->updateSleepTime ();
  int retVal = sigsetjmp(m_current_thread->getEnv (), 1);
  if (retVal == 1)
    {
      return;
    }
  this->popReadyQ ();
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
      setTimer ();
      initId ();
    }
}

void ThreadManager::initId ()
{
  for (int i = 1; i < MAX_THREAD_NUM; ++i)
    {
      m_available_id.push_back (0);
    }
}

/************************* START OF HANDLING TIMER **************************/

void timer_handler (int sig)
{
  auto *current_thread = thread_manager->getCurrentThread ();
  printf ("Timer expired of %d\n", current_thread->getId ());

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
  timer.it_value.tv_sec = m_num_quantums; // first time interval, seconds part
  timer.it_value.tv_usec = 0;        // first time interval, microseconds part

//  // configure the timer to expire every 3 sec after that.
  timer.it_interval.tv_sec = 1;    // following time intervals, seconds part
  timer.it_interval.tv_usec = 0; // following time intervals, microseconds part

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

void ThreadManager::startRunning ()
{

}
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
      return 0;
    }
  if (block_state == THREAD_RUNNING)
    {
      thread_to_block->setState (THREAD_BLOCKED);
      m_ready_threads.remove (thread_to_block);
      switchThread ();
      resetTimer ();
    }
  else if (block_state == THREAD_READY)
    {
      m_ready_threads.remove (thread_to_block);
    }
  else if (block_state == THREAD_SLEEPING)
    {
      thread_to_block->setState (THREAD_SLEEPING_BLOCKED);
    }
  return 0;
}

void catch_int (int sigNum)
{
  // Install catch_int as the signal handler for SIGINT.

  //itimerval tmp;
  //getitimer (ITIMER_VIRTUAL, &);
  printf (" Don't do that!\n");
  fflush (stdout);
}

int ThreadManager::block (UThread *thread)
{
  sa.sa_handler = &catch_int;
  if (sigaction (SIGINT, &sa, NULL) < 0)
    {
      printf ("sigaction error.");
    }
  return 0;
}
int ThreadManager::resume (int tid)
{
  //TODO: handle multiple pushing the same thread
  UThread *resume_thread = getThreadById (tid);
  if (resume_thread != nullptr)
    {
      resume_thread->setState (THREAD_READY);
      m_ready_threads.push_back (resume_thread);
      return SUCCESS;
    }
  return FAILURE;
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
      UThread *u_thread = m_threads.at (tid); //if tid not in map throw error
      m_available_id[tid] = 0;
      m_ready_threads.remove (u_thread);
      delete (u_thread);
      m_threads.erase (tid);
      return 0; //TODO: change
    }
  for (auto const &thrd: m_threads)
    {
      delete (thrd.second);
    }
  m_threads.clear ();
  m_ready_threads.clear ();
}

void ThreadManager::sleepThread (int num_quantums)
{
  m_current_thread->setState (THREAD_SLEEPING);
  m_current_thread->setSleepTime(num_quantums);
  m_sleeping_threads.insert ({m_current_thread->getId(), m_current_thread});
  switchThread ();
  resetTimer ();
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





