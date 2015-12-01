#ifndef _SCHEDULE_EVENT_H
#define _SCHEDULE_EVENT_H

#include "TaskManager.h"

/*
 * Schedule Event: consists the callback function, task, and the optional
 *                 context that may be needed for the callback.
 *
 * Schedule Event is called on a recurring time interval to put tasks into main
 * queue.
 */

class ScheduleEvent {
public:
  /*
   * Constructor: Schedule Event adds the given task and context into the queue
   *              periodically.
   */
  ScheduleEvent(TaskManager *task_manager, Task task, void *context);

  /*
   * Schedule the task and context into the task manager
   */
  void handle();

private:
  TaskManager *task_manager;
  void *context;
  Task task;
};

#endif /* _SCHEDULE_EVENT_H */
