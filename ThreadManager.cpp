//
// Created by alexk on 19/04/2023.
//

#include "ThreadManager.h"

void ThreadManager::createThread (int tid, thread_entry_point entry_point)
{
  auto *thread = new UThread (tid, entry_point);
  m_threads.insert ({tid, thread});
  //m_threads.insert (std::pair<int,UThread*>(tid,thread));
}

void ThreadManager::switchToThread (int tid)
{
  if (m_current_thread.save_current_state () != 0)
    {
      //TODO: error?
    }
  UThread *thread = m_threads.at (tid);
  sigjmp_buf &env = thread->getEnv ();
  siglongjmp (env, 1);//TODO: check return value?
}

ThreadManager::ThreadManager ()
{
  //checks if there is no thread yet(if 0 terminated we terminate the program
  if (m_threads.empty ())
    {
      m_threads.insert ({0, new UThread ()});
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
