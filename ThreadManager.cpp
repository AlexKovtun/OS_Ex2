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

void ThreadManager::switchToThread (int tid)
{
  int retVal = sigsetjmp(m_current_thread->getEnv (), 1);
  printf ("%d\n", retVal);
  fflush (stdout);
  if (retVal == 1)
    {
      return;
    }
  m_current_thread = m_threads.at (tid);
  //sigsetjmp(env, 1);// todo: figure out mask param
  siglongjmp (m_current_thread->getEnv (), 1);//TODO: check return value?
  return;
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
      auto *pt = new UThread (num_quantums);
      m_threads.insert ({0, pt});
      m_current_thread = pt;
      installTimer ();
      setTimer (pt->getQuantumTime ());
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
  printf ("we got here :)\n");

  auto *current_thread = thread_manager->getCurrentThread ();
  printf ("Timer expired of %d\n", current_thread->getId ());

  thread_manager->pushReadyQ (current_thread);
  auto *next_thread = thread_manager->popReadyQ ();
  thread_manager->switchToThread (next_thread->getId ());
  printf ("now starting running thread: %d\n", next_thread->getId ());
  fflush (stdout);
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
}

int ThreadManager::setTimer (int quantum_usecs)
{
  // Configure the timer to expire after 1 sec... */
  timer.it_value.tv_sec = quantum_usecs; // first time interval, seconds part
  timer.it_value.tv_usec = 0;        // first time interval, microseconds part

//  // configure the timer to expire every 3 sec after that.
  timer.it_interval.tv_sec = 3;    // following time intervals, seconds part
  timer.it_interval.tv_usec = 0; // following time intervals, microseconds part

  // Start a virtual timer. It counts down whenever this process is executing.
  if (setitimer (ITIMER_VIRTUAL, &timer, NULL))
    {
      //printf ("the timer error reason is: %s\n", strerror (errno));
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

int ThreadManager::block (int tid)
{
  UThread *thread_to_block = getThreadById (tid);
  if (thread_to_block == nullptr)
    {
      return FAILURE;
    }
  if (thread_to_block->getState () == THREAD_BLOCKED)
    {
      return 0;
    }
  if (thread_to_block->getState () == THREAD_RUNNING)
    {
      // get current timer state
      // save it
      // when return to ready list, resume the  time that left
      // switch to neext thread on the queue_ready_list

    }
  else if (thread_to_block->getState () == THREAD_READY)
    {
      //pop from queue, set state to blocked
    }
  return 0;
}



