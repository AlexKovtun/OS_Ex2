#include "uthreads.h"
#include "ThreadManager.h"




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
int uthread_init(int quantum_usecs){
  thread_manager = new ThreadManager(quantum_usecs);
  thread_manager->startRunning();
  return 0;
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

int uthread_spawn(thread_entry_point entry_point){
  thread_manager->createThread(thread_manager->getAvailableId(),entry_point);
  return 0;
}



int uthread_get_tid(){
  thread_manager->getCurrentId();
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


int uthread_block(int tid){
  if(tid == 0){
    return FAILURE;
  }
  thread_manager->blockThread (tid);
}



/**
 * @brief Resumes a blocked thread with ID tid and moves it to the THREAD_READY state.
 *
 * Resuming a thread in a RUNNING or THREAD_READY state has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered an error.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid){
  thread_manager->resume(tid);
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
  //std::cout<< "terminating"<<std::endl;
  int result = thread_manager->terminateThread (tid);
  if (tid == 0)
  {
    delete (thread_manager);
    exit (0);
  }
  if (tid != thread_manager->getCurrentId ())
  {
    return result;
  }
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
int uthread_sleep(int num_quantums){
  if(thread_manager->getCurrentId() == 0){
    return FAILURE;
  }
  thread_manager->sleepThread(num_quantums);
  return SUCCESS;
}