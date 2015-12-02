#ifndef _TASK_MANAGER_H
#define _TASK_MANAGER_H

/*
 * Task Manager: provides a queue of tasks that may be scheduled (added)
 *               and be continuously executed.
 */
#include "RingBuffer.h"

class TaskManager {
public:
  /* 
   * Constructor
   */
  TaskManager(int nTaskInQueue);

  /*
   * Loop through the queue and executes tasks continuously.
   */
  void run();

  /*
   * Push the given task and context to the queue.
   */
  void schedule(Task task, void *context);

private:
  RingBuffer<task_context_t> *queue;
};

#endif /* _TASK_MANAGER_H */
