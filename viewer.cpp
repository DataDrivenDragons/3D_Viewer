#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <pthread.h>
#include "utils.h"

char PUSH[] = "Pushed";
char POP[] = "Popped";

// ACTUAL APPLICATION CODE

void print_triangle(char *comment, float *triangle)
{
  printf("%s (%f %f %f %f) (%f %f %f %f) (%f %f %f %f)\n", comment,
      triangle[0], triangle[1], triangle[2], triangle[3],
      triangle[4], triangle[5], triangle[6], triangle[7],
      triangle[8], triangle[9], triangle[10], triangle[11]);
}

void *parse(void *arg)
{
  Shared *shared = (Shared *) arg;
  float *triangle1 = (float *) malloc(12 * sizeof(float));
  float *triangle2 = (float *) malloc(12 * sizeof(float));
  float *triangle3 = (float *) malloc(12 * sizeof(float));
  triangle1[0] = 0.1;
  triangle2[0] = 0.2;
  triangle3[0] = 0.3;
  queue_push(shared->queue, triangle1);
  queue_push(shared->queue, triangle2);
  queue_push(shared->queue, triangle3);
  pthread_exit(NULL);
}

void *render(void *arg)
{
  int i;
  Shared *shared = (Shared *) arg;
  float *triangle;
  for (i = 0; i < 3;) {
    triangle = queue_pop(shared->queue);
    //print_triangle(POP, triangle);
  }
  pthread_exit(NULL);
}

int main()
{
  Shared *shared = make_shared();
  pthread_t parser = make_thread(parse, shared);
  pthread_t renderer = make_thread(render, shared);
  
  join_thread(renderer);
  join_thread(parser);
  return 0;
}