#include "uthreads.h"
#include "ThreadManager.h"
#include <cstdio>
#include <new>


/**
 * @brief initializes the thread library.
 *
 * Once this function returns, the main thread (tid == 0) will be set as RUNNING. There is no need to
 * provide an entry_point or to create a stack for the main thread - it will be using the "regular" stack and PC.
 * You may assume that this function is called before any other thread library function, and that it is called
 * exactly once.
 * The input to the function is the length of a quantum in micro-seconds.
 * It is an error to call this function with non-positive quantum_usecs.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_init (int quantum_usecs)
{
  if (quantum_usecs <= 0)
    {
      fprintf (stderr, "thread library error: quantum_usecs should be "
                       "positive\n");
      return FAILURE;
    }
  try
    {
      thread_manager = new ThreadManager (quantum_usecs);
    }
  catch (std::bad_alloc)
    {
      fprintf (stderr, "system error: couldn't alloc thread\n");
      exit (1);
    }
  return SUCCESS;
}

/**
 * @brief Creates a new thread, whose entry point is the function entry_point with the signature
 * void entry_point(void).
 *
 * The thread is added to the end of the THREAD_READY threads list.
 * The uthread_spawn function should fail if it would cause the number of concurrent threads to exceed the
 * limit (MAX_THREAD_NUM).
 * Each thread should be allocated with a stack of size STACK_SIZE bytes.
 * It is an error to call this function with a null entry_point.
 *
 * @return On success, return the ID of the created thread. On failure, return -1.
*/

int uthread_spawn (thread_entry_point entry_point)
{
  thread_manager->timerStatus (SIG_BLOCK);
  if (!entry_point)
    {
      fprintf (stderr, "thread library error: entry point null\n");
      return FAILURE;
    }
  int tid = thread_manager->getAvailableId ();
  if (tid == FAILURE)
    {
      fprintf (stderr, "thread library error: too much threads\n");
      return FAILURE;
    }
  thread_manager->createThread (tid, entry_point);
  thread_manager->timerStatus (SIG_UNBLOCK);
  return tid;
}

int uthread_get_tid ()
{
  return thread_manager->getCurrentId ();
}

/**
 * @brief Blocks the thread with ID tid. The thread may be resumed later using uthread_resume.
 *
 * If no thread with ID tid exists it is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision should be made. Blocking a thread in
 * BLOCKED state has no effect and is not considered an error.
 *
 * @return On success, return 0. On failure, return -1.
*/


int uthread_block (int tid)
{
  thread_manager->timerStatus (SIG_BLOCK);
  if (thread_manager->isValidId (tid))
    {
      thread_manager->timerStatus (SIG_UNBLOCK);
      return FAILURE;
    }
  if (tid == 0)
    {
      fprintf (stderr, "thread library error: trying tot delete main thread\n");
      thread_manager->timerStatus (SIG_UNBLOCK);
      return FAILURE;
    }
  if(!thread_manager->isExistThread(tid)){
    fprintf (stderr, "thread library error: thread doesn't exist\n");
    thread_manager->timerStatus (SIG_UNBLOCK);
    return FAILURE;
  }
  thread_manager->blockThread (tid);
  thread_manager->timerStatus (SIG_UNBLOCK);
  return SUCCESS;
}

/**
 * @brief Resumes a blocked thread with ID tid and moves it to the THREAD_READY state.
 *
 * Resuming a thread in a RUNNING or THREAD_READY state has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered an error.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_resume (int tid)
{
  thread_manager->timerStatus (SIG_BLOCK);
  if(thread_manager->resume (tid) == FAILURE){
      thread_manager->timerStatus (SIG_UNBLOCK);
      return FAILURE;
  }
  thread_manager->timerStatus (SIG_UNBLOCK);
  return SUCCESS;
}

/**
 * @brief Terminates the thread with ID tid and deletes it from all relevant control structures.
 *
 * All the resources allocated by the library for this thread should be released. If no thread with ID tid exists it
 * is considered an error. Terminating the main thread (tid == 0) will result in the termination of the entire
 * process using exit(0) (after releasing the assigned library memory).
 *
 * @return The function returns 0 if the thread was successfully terminated and -1 otherwise. If a thread terminates
 * itself or the main thread is terminated, the function does not return.
*/
int uthread_terminate (int tid)
{
  thread_manager->timerStatus (SIG_BLOCK);
  if(thread_manager->terminateThread (tid) == FAILURE){
    return FAILURE;
  }

  if (tid == 0)
    {
      //delete (thread_manager);
      thread_manager->timerStatus (SIG_UNBLOCK);
      exit (0);
    }

  thread_manager->timerStatus (SIG_UNBLOCK);
  return 0;
}

/**
 * @brief Blocks the RUNNING thread for num_quantums quantums.
 *
 * Immediately after the RUNNING thread transitions to the BLOCKED state a scheduling decision should be made.
 * After the sleeping time is over, the thread should go back to the end of the THREAD_READY queue.
 * If the thread which was just RUNNING should also be added to the THREAD_READY queue, or if multiple threads wake up
 * at the same time, the order in which they're added to the end of the THREAD_READY queue doesn't matter.
 * The number of quantums refers to the number of times a new quantum starts, regardless of the reason. Specifically,
 * the quantum of the thread which has made the call to uthread_sleep isn’t counted.
 * It is considered an error if the main thread (tid == 0) calls this function.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_sleep (int num_quantums)
{
  thread_manager->timerStatus (SIG_BLOCK);
  if (thread_manager->getCurrentId () == 0)
    {
      fprintf (stderr, "thread library error: main thread cannot call sleep\n");
      thread_manager->timerStatus (SIG_UNBLOCK);
      return FAILURE;
    }

  thread_manager->threadSleep (num_quantums);
  thread_manager->timerStatus (SIG_UNBLOCK);
  return SUCCESS;
}

/**
 * @brief Returns the total number of quantums since the library was initialized, including the current quantum.
 *
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number should be increased by 1.
 *
 * @return The total number of quantums.
*/
int uthread_get_total_quantums ()
{
  return thread_manager->getTotalQuantum ();
}

/**
 * @brief Returns the number of quantums the thread with ID tid was in RUNNING state.
 *
 * On the first time a thread runs, the function should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state when this function is called, include
 * also the current quantum). If no thread with ID tid exists it is considered an error.
 *
 * @return On success, return the number of quantums of the thread with ID tid. On failure, return -1.
*/
int uthread_get_quantums (int tid)
{
  thread_manager->timerStatus (SIG_BLOCK);
  if(thread_manager->isValidId (tid) == FAILURE)
    return FAILURE;
  if(!thread_manager->isExistThread(tid)){
      fprintf (stderr,"thread library error: "
                      "thread library error:no thread with given tid\n");
      return FAILURE;
  }

  UThread *u_thread = thread_manager->getThreadById (tid);
  thread_manager->timerStatus (SIG_UNBLOCK);
  if (u_thread == nullptr)
    {
      return FAILURE;
    }
  return u_thread->getRunningQuantum ();
}