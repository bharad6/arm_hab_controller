#include "InterruptEvent.h"

/*
 * Constructor
 */
InterruptEvent::InterruptEvent(TaskManager *_task_manager, 
                               SerialTask _task,
                               void *_context,
                               void *_serial) {
  /* Setting up the task manager who to add the context and task to
     periodically. */
  task_manager = _task_manager;
  task = _task;
  context = _context;
  serial = _serial;
}

/*
 * Schedule the task and context into the task manager
 */
void InterruptEvent::handle() { 
  task(serial, context);
}
