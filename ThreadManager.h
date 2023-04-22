//
// Created by alexk on 19/04/2023.
//

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include <vector>
#include <list>
#include <map>
#include "UThread.h"
#include "uthreads.h"

#define FAILURE -1
#define SUCCESS 0


class ThreadManager {
 private:
  std::map<int, UThread *> m_threads;
  std::list<UThread *> m_ready_threads;
  std::vector<int> m_available_id;
  UThread *m_current_thread;

  void initId ();

 public:
  std::map<int, UThread *> getThreads ()
  { return m_threads; }
  void switchThread ();
  ThreadManager (int num_quantums);
  void createThread (int tid, thread_entry_point entry_point);
  int getAvailableId ();
  void startRunning ();
  void pushReadyQ(UThread* threadToInsert);
  UThread * popReadyQ();

  struct sigaction sa = {0};
  struct itimerval timer = {0};
  int m_num_quantums;
  int installTimer ();
  int setTimer ();
  int getCurrentId () { return m_current_thread->getId();};
  UThread *getCurrentThread () { return m_current_thread;};
  int getQuantumTime () {return this->m_num_quantums;};
  UThread *getThreadById ();
  UThread *getThreadById (int tid);
  int blockThread (int tid);
  int block (UThread *thread);
  int resume (int tid);
};



extern ThreadManager *thread_manager;
#endif _THREADMANAGER_H_

