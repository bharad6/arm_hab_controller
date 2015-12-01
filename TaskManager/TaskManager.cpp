#include "TaskManager.h"

/* 
 * Constructor
 */
TaskManager::TaskManager(int nTaskInQueue)
{
  /* Setting up the queues. */
  queue = new RingBuffer<task_context_t>(nTaskInQueue);
}

/*
 * Loop through the queue and executes tasks continuously.
 */
void TaskManager::run() {
  Task task;
  void *context;
  task_context_t cur;

  while (true) {
    if (queue->pop(cur)) {
      task = cur.task;
      context = cur.context;
      task(context);
    }
  }
}

/*
 * Push the given task and context to the queue.
 */
void TaskManager::schedule(Task task, void *context) {
  task_context_t new_elem = { task, context };
  if (!queue->insert(new_elem)) {
    /* Error queue insertion failed, queue full */
  }
}
