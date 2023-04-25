//
// Created by alexk on 19/04/2023.
//

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include "UThread.h"
#include "uthreads.h"

#define FAILURE -1
#define SUCCESS 0



class ThreadManager
{
 private:
  std::unordered_map<int, UThread *> m_threads;
  std::map<int, UThread *> m_sleeping_threads;
  std::list<UThread *> m_ready_threads;
  std::vector<int> m_available_id;
  UThread *m_current_thread;
  int m_total_num_of_quantum;
  int m_num_quantums;
  sigset_t set;

  void initId ();
  int terminateAll ();

 public:
  void switchThread ();
  ThreadManager (int num_quantums);
  int createThread (int tid, thread_entry_point entry_point);
  int getAvailableId ();
  void pushReadyQ (UThread *threadToInsert);
  UThread *popReadyQ ();

  struct sigaction sa = {0};
  struct itimerval timer = {0};

  int installTimer ();
  int setTimer ();
  int resetTimer ();
  int getCurrentId ()
  { return m_current_thread->getId (); };
  UThread *getCurrentThread ()
  { return m_current_thread; };
  int getQuantumTime ()
  { return this->m_num_quantums; };
  UThread *getThreadById ();
  UThread *getThreadById (int tid);
  int blockThread (int tid);
  int resume (int tid);
  int terminateThread (int tid);
  int threadSleep (int num_quantums);
  void updateSleepTime ();
  void increaseTotalQuantum ()
  { ++m_total_num_of_quantum; }
  int getTotalQuantum ()
  {
    return m_total_num_of_quantum;

  };
  int isValidId (int tid);
  void timerStatus (int block_flag);
  int terminateByState (int tid);
  void HandleExit (const char *type, const char *msg);
  bool isExistThread (int tid);
};

extern ThreadManager *thread_manager;
#endif _THREADMANAGER_H_

