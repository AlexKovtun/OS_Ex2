//
// Created by alexk on 19/04/2023.
//

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include <queue>
#include <map>
#include "UThread.h"
#include "uthreads.h"

class ThreadManager {
 private:
  std::map<int, UThread *> m_threads;
  std::queue<UThread *> m_ready_threads;
  std::vector<int> m_available_id;
  UThread *m_current_thread;

  void initId ();

 public:
  std::map<int, UThread *> getThreads ()
  { return m_threads; }
  void switchToThread (int tid);
  ThreadManager (int num_quantums);
  void createThread (int tid, thread_entry_point entry_point);
  int getAvailableId ();
  void startRunning ();

  struct sigaction sa = {0};
  struct itimerval timer = {0};
  int installTimer ();
  int setTimer (int quantum_usecs);
  int getCurrentId () { return m_current_thread->getId();};
  UThread *getCurrentThread () { return m_current_thread;};
};



extern ThreadManager *thread_manager;
#endif _THREADMANAGER_H_

