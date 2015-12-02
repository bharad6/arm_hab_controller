#ifndef _PROGRAM_INCLUDES_H
#define _PROGRAM_INCLUDES_H

typedef void (*Task)(const void *);
typedef void (*SerialTask)(const void *, const void *);

/* Structure that wraps the task and context to avoid race condition
 * where task exists and context doesn't or vice versa.
 */
struct task_context {
  Task task;
  void *context;

  inline bool operator == (const struct task_context & t) const {
    return task == t.task;
  }
} typedef task_context_t;


#endif /* _PROGRAM_INCLUDES_H */
