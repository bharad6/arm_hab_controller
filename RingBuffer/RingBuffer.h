/*
 * General Ring Buffer implementation
 */
#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H

#include "TaskContext.h"

template <class T>
class RingBuffer {
public:
  /*
   * Constructor: nElem is the maximum number of elements in buffer at one time.
   *              (does not include extra element that should remain emtpy).
   */
  RingBuffer(int nElem);  
  
  /* Destructor */
  ~RingBuffer();

  /* 
   * insert: Append the obj to the end of the buffer.
   *         Returns true if inserted.
   *         Returns false if buffer is full.
   */
  bool insert(T & obj);

  /*
   * pop: Pop the first non-empty element into the obj by reference
   *      Returns true if non-empty element is popped.
   *      Returns false if buffer is empty.
   */
  bool pop(T & obj);
  
  /*
   * clear: Clears the entire memory by setting the empty and occupy pointe
   *        to same index.
   */
  void clear();

  /*
   * getNumElem: gets the number of elements in the buffer
   */
  int getNumElem();

  /* 
   * isEmpty: Returns true if the buffer is empty.
   */
  bool isEmpty();

  /*
   * isFull: Returns true if the buffer is full.
   */
  bool isFull();

  /*
   * findFirst: Given an object, find the index of the first same object
   *            int the buffer in linear time.
   *            Returns -1 if not found.
   */
  int findFirst(const T & obj);
private:
  T *arr;
  int occupy_ind;
  int free_ind;
  int nElem;

  /* Private Helper Methods */
  int findInIncreasingIndex(int start, int last, const T & obj);
};

#endif /* _RING_BUFFER_H */
