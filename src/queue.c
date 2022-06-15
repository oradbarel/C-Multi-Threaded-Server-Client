#include "queue.h"


int setConVarr(ConnVar var, int conn_num){
    var->connfd = conn_num;
    gettimeofday(&(var->enter_time), NULL);// TODO: maybe check the ruturn value...
    return 0;
}

Queue createQueue(unsigned capacity)
{
    Queue queue = (Queue)malloc(sizeof(*queue));
    if(queue == NULL)
    {
        return NULL;
    }
    queue->capacity = capacity;
    queue->front = -1;
    queue->rear = -1;
    queue->size = 0;
    queue->array = (ConnVar*)malloc((queue->capacity) * sizeof(*(queue->array)));
    return queue;
}

void destroyQueue(Queue queue)
{
    if (!queue)
    {
        return;
    }
    while (!(isEmpty(queue)))
    {
        ConnVar tmp = queueFront(queue);
        queueDequeue(queue);
        Close(tmp->connfd);
        free(tmp);
    }
    free(queue->array);
    free(queue);
}

int isFull(Queue queue)
{
    if(!queue)
    {
        return -1;
    }
    return (queue->size == queue->capacity);
}

int isEmpty(Queue queue)
{
    if(!queue)
    {
        return -1;
    }
    return (queue->size == 0);
}

void queueEnqueue(Queue queue, ConnVar var)
{
    if(isFull(queue))
    {
        return;
    }
    if(isEmpty(queue))
    {
        queue->front = 0;
    }
    queue->rear = (queue->rear + 1) % (queue->capacity); // this is a circular queue. 
    queue->array[queue->rear] = var;
    queue->size += 1;
}

void queueDequeue(Queue queue)
{
    if (isEmpty(queue))
    {
        return;
    }
    if(queue->front == queue->rear)
    {
        queue->front = queue->rear = -1;
    }
    else
    {
        queue->front = (queue->front + 1) % (queue->capacity);
    }
    queue->size -= 1;
}

int queueDequeueSomeElements(Queue queue, int n)
{
    if (n > queue->size)
    {
        return -1;
    }
    int drop_num = n;
    while (drop_num > 0)
    {
        ConnVar curr = queueFront(queue);
        queueDequeue(queue);
        Close(curr->connfd);
        free(curr);
        queue->size--;
        drop_num--;
    }
    return 0;
}

ConnVar queueFront(Queue queue)
{
    if(!queue || isEmpty(queue))
    {
        return NULL;
    }
    return queue->array[queue->front];
}

/*
ConnVar getOldestElement(Queue queue)
{
    if(queue == NULL || queue->size <= 0)
    {
        return NULL;
    }
    int curr = queue->front;
    while (curr != queue->rear)
    {
        
    }
    

}*/
