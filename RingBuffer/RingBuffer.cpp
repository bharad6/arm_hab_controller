#include "RingBuffer.h"
#include <string.h>

/* Constructor */
template <class T>
RingBuffer<T>::RingBuffer(int _nElem) :
  nElem(_nElem + 1)
  /* +1 for one extra element spot that should be empty at all time */
{
  occupy_ind = 0;
  free_ind = 0;
  arr = new T [nElem];
  memset(arr, 0, sizeof(T)*nElem);
}

/* Destructor */
template <class T>
RingBuffer<T>::~RingBuffer() {
  delete [] arr;
}

/* Insert new object */
template <class T>
bool RingBuffer<T>::insert(T & obj) {
  if (isFull()) return false;
  arr[free_ind] = obj;
  free_ind = (free_ind + 1) % nElem;
  return true;
}

/* Pop object */
template <class T>
bool RingBuffer<T>::pop(T & obj) {
  if (isEmpty()) return false;
  obj = arr[occupy_ind];
  occupy_ind = (occupy_ind + 1) % nElem;
  return true;
}

/*
 * Clear
 */
template <class T>
void RingBuffer<T>::clear() {
  occupy_ind = 0;
  free_ind = 0;
}

template <class T>
int RingBuffer<T>::getNumElem() {
  if (occupy_ind <= free_ind) {
    return free_ind - occupy_ind;
  }

  return free_ind + nElem - occupy_ind;
}

/* Return true if buffer is empty */
template <class T>
bool RingBuffer<T>::isEmpty() {
  return free_ind == occupy_ind;
}

/* Return true if buffer is full */
template <class T>
bool RingBuffer<T>::isFull() {
  if (occupy_ind == 0) return free_ind == nElem - 1;
  return free_ind == occupy_ind - 1;
}

/* Return the index where the object is first found in queue 
   Returns -1 if not found.
*/
template <class T>
int RingBuffer<T>::findFirst(const T & obj) {
  /* Free index is to the right of the occupy index */
  if (free_ind >= occupy_ind) {
    return findInIncreasingIndex(occupy_ind, free_ind + 1, obj);
  } 
  
  /* Free index is on the left of the occupy index, data looped back */
  int found = findInIncreasingIndex(occupy_ind, nElem, obj);
  if (found != -1) return found;
  return findInIncreasingIndex(0, free_ind, obj);
}

template <class T>
int RingBuffer<T>::findInIncreasingIndex(int start, int last, const T & obj) {
  for (int i = start; i < last; i++) {
    if (arr[i] == obj) return i;
  }
  return -1;
}

/* explicit instantiations required for all types of RingBuffer to keep
  header and implementation files separate
   https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
 */
template class RingBuffer<char>;
template class RingBuffer<int>;
template class RingBuffer<Task>;
template class RingBuffer<void *>;
template class RingBuffer<task_context_t>;
