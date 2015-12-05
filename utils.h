#define QUEUE_LENGTH 100

// UTILITY CODE

void perror_exit(char *s);
void *check_malloc(int size);

// MUTEX

typedef pthread_mutex_t Mutex;

Mutex *make_mutex();
void mutex_lock(Mutex *mutex);
void mutex_unlock(Mutex *mutex);

// CONDITION VARIABLES

typedef pthread_cond_t Cond;

Cond *make_cond();
void cond_wait(Cond *cond, Mutex *mutex);
void cond_signal(Cond *cond);

// QUEUE

typedef struct {
  float **buffer;
  int length;
  int next_in;
  int next_out;
  Mutex *mutex;
  Cond *nonempty;
  Cond *nonfull;
} Queue;

Queue *make_queue(int length);
int queue_incr(Queue *queue, int i);
int queue_empty(Queue *queue);
int queue_full(Queue *queue);
void queue_push(Queue *queue, float *triangle);
float *queue_pop(Queue *queue);

// SHARED

typedef struct {
  Queue *queue;
} Shared;

Shared *make_shared();

// THREADS

pthread_t make_thread(void *(*entry)(void *), Shared *shared);
void join_thread(pthread_t thread);