#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "utils.h"

char MALLOC_FAILED[] = "malloc failed";
char MAKE_LOCK_FAILED[] = "make_lock failed";
char LOCK_FAILED[] = "lock failed";
char UNLOCK_FAILED[] = "unlock failed";
char QUEUE_IS_FULL[] = "queue is full";
char QUEUE_IS_EMPTY[] = "queue is empty";
char PTHREAD_CREATE_FAILED[] = "pthread_create failed";
char PTHREAD_JOIN_FAILED[] = "pthread_join failed";
char MAKE_COND_FAILED[] = "make_cond failed";
char COND_WAIT_FAILED[] = "cond_wait failed";
char COND_SIGNAL_FAILED[] = "cond_signal failed";

// UTILITY CODE

void perror_exit(char *s)
{
  perror(s);  
  exit(-1);
}

void *check_malloc(int size)
{
  void *p = malloc(size);
  if (p == NULL) perror_exit(MALLOC_FAILED);
  return p;
}

// MUTEX

Mutex *make_mutex()
{
  Mutex *mutex = (Mutex *) check_malloc(sizeof(Mutex));
  int n = pthread_mutex_init(mutex, NULL);
  if (n != 0) perror_exit(MAKE_LOCK_FAILED);
  return mutex;
}

void mutex_lock(Mutex *mutex)
{
  int n = pthread_mutex_lock(mutex);
  if (n != 0) perror_exit(LOCK_FAILED);
}

void mutex_unlock(Mutex *mutex)
{
  int n = pthread_mutex_unlock(mutex);
  if (n != 0) perror_exit(UNLOCK_FAILED);
}

// CONDITION VARIABLES

Cond *make_cond()
{
  Cond *cond = (Cond *) check_malloc(sizeof(Cond));
  int n = pthread_cond_init(cond, NULL);
  if (n != 0) perror_exit(MAKE_COND_FAILED);
  return cond;
}

void cond_wait(Cond *cond, Mutex *mutex)
{
  int n = pthread_cond_wait(cond, mutex);
  if (n != 0) perror_exit(COND_WAIT_FAILED); 
}

void cond_signal(Cond *cond)
{
  int n = pthread_cond_signal(cond);
  if (n != 0) perror_exit(COND_SIGNAL_FAILED); 
}

// QUEUE

Queue *make_queue(int length)
{
  Queue *queue = (Queue *) malloc(sizeof(Queue));
  queue->length = length;
  queue->buffer = (float **) malloc(length * sizeof(float *));
  queue->next_in = 0;
  queue->next_out = 0;
  queue->mutex = make_mutex();
  queue->nonempty = make_cond();
  queue->nonfull = make_cond();
  return queue;
}

int queue_incr(Queue *queue, int i)
{
  return (i+1) % queue->length;
}

int queue_empty(Queue *queue)
{
  int res = (queue->next_in == queue->next_out);
  return res;
}

int queue_full(Queue *queue)
{
  int res = (queue_incr(queue, queue->next_in) == queue->next_out);
  return res;
}

void queue_push(Queue *queue, float *triangle) {
  mutex_lock(queue->mutex);
  while (queue_full(queue)) {
    cond_wait(queue->nonfull, queue->mutex);
  }
  queue->buffer[queue->next_in] = triangle;
  queue->next_in = queue_incr(queue, queue->next_in);
  printf("A %f %i %i %p %p %p\n", triangle[0], queue->next_in, queue->next_out,
    queue->buffer[0], queue->buffer[1], queue->buffer[2]);
  mutex_unlock(queue->mutex);
  cond_signal(queue->nonempty);
}

float *queue_pop(Queue *queue) {
  mutex_lock(queue->mutex);
  while (queue_empty(queue)) {
    cond_wait(queue->nonempty, queue->mutex);
  }
  float *triangle = queue->buffer[queue->next_out];
  queue->next_out = queue_incr(queue, queue->next_out);
  printf("B %f %i %i %p %p %p\n", triangle[0], queue->next_in, queue->next_out,
    queue->buffer[0], queue->buffer[1], queue->buffer[2]);
  mutex_unlock(queue->mutex);
  cond_signal(queue->nonfull);
  return triangle;
}

// SHARED

Shared *make_shared()
{
  Shared *shared = (Shared *) check_malloc(sizeof(Shared));
  shared->queue = make_queue(QUEUE_LENGTH);
  return shared;
}

// THREADS

pthread_t make_thread(void *(*entry)(void *), Shared *shared)
{
  int ret;
  pthread_t thread;

  ret = pthread_create(&thread, NULL, entry, shared);
  if (ret != 0) perror_exit(PTHREAD_CREATE_FAILED);
  return thread;
}

void join_thread(pthread_t thread)
{
  int ret = pthread_join(thread, NULL);
  if (ret == -1) perror_exit(PTHREAD_JOIN_FAILED);
}