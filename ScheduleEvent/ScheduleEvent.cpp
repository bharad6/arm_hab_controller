#include "ScheduleEvent.h"

/*
 * Constructor
 */
ScheduleEvent::ScheduleEvent(TaskManager *_task_manager, 
                             Task _task,
                             void *_context) {
  /* Setting up the task manager who to add the context and task to
     periodically. */
  task_manager = _task_manager;
  task = _task;
  context = _context;
}

/*
 * Schedule the task and context into the task manager
 */
void ScheduleEvent::handle() { 
  task_manager->schedule(task, context);
}
