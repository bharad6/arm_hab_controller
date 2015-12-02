#ifndef _INTERRUPT_EVENT_H
#define _INTERRUPT_EVENT_H

#include "TaskManager.h"

/*
 * Interrupt Event: consists the callback function, task, and the optional
 *                 context that may be needed for the callback.
 * 
 * Interrupt Event is called when a UART received some data, the handle
 * will put the hardware memory into software memory. The context should
 * be the Iridium object.
 */

class InterruptEvent {
public:
  /*
   * Constructor: Interrupt Event that is called when UART data arrived
   */
  InterruptEvent(TaskManager *task_manager, SerialTask task, void *context, void *serial);

  /*
   * Handles the copy of hardware memory into software memory inside Iridium
   * object.
   */
  void handle();

private:
  TaskManager *task_manager;
  void *context;
  void *serial;
  SerialTask task;
};

#endif /* _INTERRUPT_EVENT_H */
