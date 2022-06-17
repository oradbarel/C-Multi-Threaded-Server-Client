#ifndef _HW3_QUEUE
#define _HW3_QUEUE

#include "segel.h"

typedef struct conn_var
{
    int connfd;
    struct timeval enter_time;
    struct timeval leave_time;
} *ConnVar;

/**
 * setConVarr: Sets the struct with connfd and current time.
 * @param var
 * @param conn_num
 * @return 0 if sucess. -1 if fails.
 */
int setConVarr(ConnVar var, int conn_num);

// A structure to represent a queue
typedef struct queue {
    int front, rear, size;
    unsigned capacity;
    ConnVar* array;
} *Queue;

/**
 * createQueue: Create a queue of given capacity. It initializes size of queue as 0.
 * @param queue The queue.
 * @return The new queue. NULL if allocation failed.
 */
Queue createQueue(unsigned capacity);

/**
 * destroyQueue: Frees all the memory and destroys the queue.
 * @param queue The queue.
 */
void destroyQueue(Queue queue);

/**
 * isFull: Queue is full when size becomes equal to the capacity
 * @param queue The queue.
 * @return 1 if full. 0 if there is a space. -1 if NULL.
 */
int isFull(Queue queue);

/**
 * isFull: Queue is empty when size is 0
 * @param queue The queue.
 * @return 1 if empty. 0 if there are elemnts inside. -1 if NULL.
 */
int isEmpty(Queue queue);

/**
 * queueEnqueue: Enqueue the ConnVar argument. If queue is full, waits until there is a space.
 * @param queue The queue.
 * @param var An allocated variable to enqueue. (ConnVar is a pointer and its content should br malloced by the caller).
 */
void queueEnqueue(Queue queue, ConnVar var);

/**
 * queueDequeue: Removes oldest element from queue, if queue not empty.
 * @param queue The queue.
 */
void queueDequeue(Queue queue);

/**
 * queueDequeueSomeElements: Removes the n oldest elements from queue, if there are at least n elemnts.
 * NOTE: This function make `Close()` and `free()` to each elemnt it removes.
 * @param queue The queue.
 * @param n Number of elemnts to remove.
 * @return 0, if sucess. -1, if (n > queue_size)
 */
int queueDequeueSomeElements(Queue queue, int n);

/**
 * queueFront: Return front elenemt(the "oldest" element) in the queue.
 * @param queue The queue.
 * @return The oldest element. NULL if queue is null or if queue is empty.
 */
ConnVar queueFront(Queue queue);




/**
 * getOldestElement: Returns the oldest requsts currently waiting in the queue.
 * Complexity - O(queue_size).
 * @param queue The queue.
 * @return Oldest request.
 */
/*ConnVar getOldestElement(Queue queue);*/


#endif // _HW3_QUEUE