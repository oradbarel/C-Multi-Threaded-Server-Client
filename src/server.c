// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)> <threads> <queue_size> <schedalg>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

#include "segel.h"
#include "request.h"
#include "queue.h"

/** __TOSCHEDALG(ADDR, STR):
 *  Convert thr string STR to enum schedalg
 */
#define __TOSCHEDALG(ADDR, STR)          \
    if (strcmp(STR, "block") == 0)       \
        *ADDR = BLOCK;                   \
    else if (strcmp(STR, "dt") == 0)     \
        *ADDR = DT;                      \
    else if (strcmp(STR, "dh") == 0)     \
        *ADDR = DH;                      \
    else if (strcmp(STR, "random") == 0) \
        *ADDR = RANDOM;                  \
    else                                 \
        *ADDR = SCHEDALG_ERROR;

/** ENUM schedalg(10,11,12,13,14):
 *  Used for replacing the string representing the overloading argument */
typedef enum
{
    BLOCK = 10,
    DT,
    DH,
    RANDOM,
    SCHEDALG_ERROR
} schedalg;

//======== Global Variables:

Queue waiting_queue;
// Queue handled_queue;
pthread_cond_t working_cond;
pthread_cond_t main_cond;
pthread_mutex_t lock;
int waiting_queue_size = 0;
int handaled_num = 0;

//=========================

int ceilVal(double num) {
    int inum = (int)num;
    if (num == (double)inum) {
        return inum;
    }
    return inum + 1;
}

/**
 * getargs: get the arguments: portnum, threads, queue_size, schedalg.
 * @param port a pointer to an integer.
 * @param threads a pointer to an integer.
 * @param queue_size a pointer to an integer.
 * @param schedalg a buffer.
 */
void getargs(int *port, int *threads, int *queue_size, schedalg *sched_alg, int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    __TOSCHEDALG(sched_alg, argv[4]);
}

/**
 * treadRequestHandle: The function pthread_create takes as a parameter. Handles requests.
 * @param thread_id 
 * @return A useless value.
 */
void *treadRequestHandle(void *thread_id)
{
    Stats thread_stats;
    thread_stats->thread_id = *thread_id;
    thread_stats->thread_dynamic_req_count = thread_stats->thread_static_req_count = thread_stats->thread_req_count = 0;
    while (1)
    {
        //===== enter to critical code:
        pthread_mutex_lock(&lock);
        while (waiting_queue_size == 0)
        {
            pthread_cond_wait(&working_cond, &lock);
        }
        //fprintf(stderr, "queue is not empty\n"); // debug
        waiting_queue_size--;
        handaled_num++;
        ConnVar curr = queueFront(waiting_queue);
        queueDequeue(waiting_queue);
        gettimeofday(&(curr->leave_time), NULL);
        pthread_mutex_unlock(&lock);
        //===== exit from critical code.

        //fprintf(stderr, "now thread is tring to hanldle\n"); // debug
        requestHandle(curr, thread_stats);
        Close(curr->connfd);
        free(curr); // queueDequeue does not free the node.

        pthread_mutex_lock(&lock);
        handaled_num--;
        pthread_cond_signal(&main_cond); //TODO: waik main thread, if waits for thus cond.
        pthread_mutex_unlock(&lock);
        //fprintf(stderr, "thread goes to next iteration\n"); // debug
        thread_all_requests++;
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    int threads_num, queue_size;
    schedalg sched_alg;
    getargs(&port, &threads_num, &queue_size, &sched_alg, argc, argv);
    // fprintf(stderr, "port: %d, threads_num: %d, queue_size: %d, schedalg: %s\n", port, threads_num, queue_size, schedalg); // DEBUG: check args:
    /*
    //DEBUG:
    ConnVar vars = malloc(4 * sizeof(*vars));
    for (int i = 0; i < 4; i++)
    {
        vars[i].connfd = i;
        vars[i].enter_time.tv_sec = vars[i].enter_time.tv_usec = i * 2;
        vars[i].leave_time.tv_sec = vars[i].leave_time.tv_usec = i * 10;
    }
    Queue my_queue = createQueue(3);
    if (!my_queue)
    {
        fprintf(stderr, "malloc error\n");
    }
    fprintf(stderr, "here\n");
    ConnVar tmp;
    queueEnqueue(my_queue, &vars[0]);
    tmp = queueFront(my_queue);
    fprintf(stderr, "var 1: %d\n", tmp->connfd);
    queueEnqueue(my_queue, &vars[1]);
    queueEnqueue(my_queue, &vars[2]);
    queueDequeue(my_queue);
    tmp = queueFront(my_queue);
    fprintf(stderr, "var 2: %d\n", tmp->connfd);
    queueDequeue(my_queue);
    tmp = queueFront(my_queue);
    fprintf(stderr, "var 3: %d\n", tmp->connfd);
    queueEnqueue(my_queue, &vars[3]);
    queueDequeue(my_queue);
    tmp = queueFront(my_queue);
    fprintf(stderr, "var 4: %d\n", tmp->connfd);
    */

    //
    // HW3: Create some threads...
    //

    //==================== TODO: create queue(of integers) for requests waiting  to be picked up by a worker thread.
    waiting_queue = createQueue(queue_size);
    if (!waiting_queue)
    {
        fprintf(stderr, "ERROR: memory not allocated");
        exit(1);
    }

    /*
    //==================== TODO: create queue(of integers) for requests currently handled by some worker thread.
    handled_queue = createQueue(queue_size);
    if (!handled_queue)
    {
        fprintf(stderr, "ERROR: memory not allocated");
        exit(1);
    }*/

    //====================initialize cond and mutex:
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&working_cond, NULL);
    pthread_cond_init(&main_cond, NULL);

    //==================== allocate threads:
    pthread_t *tid = malloc(threads_num * sizeof(pthread_t));
    if (tid == NULL)
    {
        fprintf(stderr, "ERROR: memory not allocated");
        exit(1);
    }

    //==================== TODO: run threads:
    for (int i = 0; i < threads_num; i++)
    {
        if (pthread_create(&tid[i], NULL, treadRequestHandle, i) != 0)
        {
            fprintf(stderr, "Failed to create thread\n");
        }
    }

    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);

        //======= push to waiting queue:
        ConnVar curr = malloc(sizeof(*curr));
        if (!curr)
        {
            fprintf(stderr, "ERROR: memory not allocated");
            exit(1);
        }
        setConVarr(curr, connfd);

        //fprintf(stderr, "main thraed wants to enter critical code\n"); // debug
        //===== enter to critical code:
        pthread_mutex_lock(&lock);
        if (waiting_queue_size + handaled_num >= queue_size)
        {
            fprintf(stderr, "=== FULL ==="); // TODO: handle...
            switch (sched_alg)
            {
            case BLOCK:
                //===== main thread blocked until there is an avaiable place in queue:
                //fprintf(stderr, "main thraed is blockede\n"); // debug
                while (waiting_queue_size + handaled_num >= queue_size)
                {
                    pthread_cond_wait(&main_cond, &lock);
                }
                //fprintf(stderr, "main thraed adding to queue\n"); // debug
                queueEnqueue(waiting_queue, curr);
                waiting_queue_size++;
                break;

            case DT:
                //===== main thread throws this very last request:
                Close(curr->connfd);
                free(curr);
                break;

            case DH:
                //===== main thread throws the oldest request(if possible), and iserts the new one:
                if (waiting_queue_size == 0)
                {
                    //===== queue is empty - thus throw this very last requst
                    Close(curr->connfd);
                    free(curr);
                }
                else
                {
                    //===== throw the oldest request and insert the new one.
                    queueDequeueSomeElements(waiting_queue, 1);
                    //there is no need to decrement queue size.
                    queueEnqueue(waiting_queue, curr);
                }
                
                /*
                ConnVar oldest = queueFront(waiting_queue);
                if (oldest)
                {
                    //===== queue is empty - thus throw this very last requst
                    Close(curr->connfd);
                    free(curr);
                }
                else
                {
                    //===== throw the oldest request and insert the new one.
                    queueDequeue(waiting_queue);
                    Close(oldest->connfd);
                    free(oldest);
                    queueEnqueue(waiting_queue, curr);
                }*/

                break;

            case RANDOM:
                //===== main thread throws 30% of requsts and then inserts the new one:
                if (waiting_queue == 0)
                {
                    //===== queue is empty - thus throw this very last requst
                    Close(curr->connfd);
                    free(curr);
                }
                else
                {
                    int drop_num = ceilVal(0.3 * waiting_queue_size); 
                    queueDequeueSomeElements(waiting_queue, drop_num);
                    waiting_queue_size -= drop_num;
                    queueEnqueue(waiting_queue, curr);
                    waiting_queue++; 
                }
                break;

            default:
                break;
            }
        }
        else
        {
            //fprintf(stderr, "main thraed adding to queue\n"); // debug
            queueEnqueue(waiting_queue, curr);
            waiting_queue_size++;
        }
        pthread_cond_signal(&working_cond);
        pthread_mutex_unlock(&lock);
        //===== exit critical code.
        //fprintf(stderr, "main thraed goes to next iteration\n"); // debug
    }
    destroyQueue(waiting_queue);
    return 0;
}
