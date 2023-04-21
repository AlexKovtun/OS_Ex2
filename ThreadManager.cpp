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
  //m_threads.insert (std::pair<int,UThread*>(tid,thread));
}

void ThreadManager::switchToThread (int tid)
{
  if (m_current_thread->save_current_state () != 0)
    {
      //TODO: error?
    }
  UThread *thread = m_threads.at (tid);
  sigjmp_buf &env = thread->getEnv ();
  siglongjmp (env, 1);//TODO: check return value?
}

ThreadManager::ThreadManager (int num_quantums)
{
  //checks if there is no thread yet(if 0 terminated we terminate the program
  if (m_threads.empty ())
    {
      auto* pt = new UThread(num_quantums);
      m_threads.insert ({0, pt});
      m_ready_threads.push (pt);
      m_current_thread = pt;
      installTimer();
      setTimer(pt->getQuantumTime ());
      initId();
    }
}

int ThreadManager::getAvailableId ()
{
  for(const auto& id : m_available_id){
    if(id == 0){
      return id;
    }
  }
  return 0;
}

void ThreadManager::initId ()
{
  for(int i = 1; i < MAX_THREAD_NUM; ++i){
    m_available_id.push_back(i);
  }
}

/************************* timer section                    */

void timer_handler (int sig)
{
  printf("we got here :)\n");
  fflush (stdout);
  //gotit = 1;
  auto* current_thread = thread_manager->getCurrentThread();
  printf ("Timer expired of %d\n", current_thread->getId());
}


int ThreadManager::installTimer(){
  // Install timer_handler as the signal handler for SIGVTALRM.
  sa.sa_handler = &timer_handler;
  if (sigaction (SIGVTALRM, &sa, NULL) == -1)
    {
      //printf ("the sigaction error reason is: %s\n", errno);
      printf("the sigaction error reason is\n");
      return -1;
    }
}

int ThreadManager::setTimer(int quantum_usecs){
  // Configure the timer to expire after 1 sec... */
  timer.it_value.tv_sec = quantum_usecs; // first time interval, seconds part
  timer.it_value.tv_usec = 0;        // first time interval, microseconds part

  // configure the timer to expire every 3 sec after that.
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


/*************************                  */

void ThreadManager::startRunning ()
{

}



