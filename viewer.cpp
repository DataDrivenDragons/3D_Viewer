#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include "utils.h"

char PUSH[] = "Pushed";
char POP[] = "Popped";

// ACTUAL APPLICATION CODE

void print_triangle(char *comment, float *triangle)
{
  printf("%s (%f %f %f ) (%f %f %f ) (%f %f %f)\n", comment,
      triangle[0], triangle[1], triangle[2],
      triangle[3], triangle[4], triangle[5], 
      triangle[6], triangle[7], triangle[8]);
}

void *parse(void *arg)
{
  Shared *shared = (Shared *) arg;
  float *triangle1 = (float *) malloc(9 * sizeof(float));
  float *triangle2 = (float *) malloc(9 * sizeof(float));
  float *triangle3 = (float *) malloc(9 * sizeof(float));
  triangle1[0] = .5;
  triangle1[1] = 1;
  triangle1[2] = 1;
  triangle1[3] = .5;
  triangle1[4] = .5;
  triangle1[5] = .5;
  triangle1[6] = -.5;
  triangle1[7] = -.5;
  triangle1[8] = -.5;


  triangle2[0] = .5;
  triangle2[1] = 1;
  triangle2[2] = 1;
  triangle2[3] = .5;
  triangle2[4] = .5;
  triangle2[5] = .5;
  triangle2[6] = 1;
  triangle2[7] = -.5;
  triangle2[8] = -.5;


  triangle3[0] = .5;
  triangle3[1] = .5;
  triangle3[2] = 1;
  triangle3[3] = .5;
  triangle3[4] = -.5;
  triangle3[5] = .5;
  triangle3[6] = -.5;
  triangle3[7] = -.5;
  triangle3[8] = -.5;

  queue_push(shared->queue, triangle1);
  queue_push(shared->queue, triangle2);
  queue_push(shared->queue, triangle3);
  pthread_exit(NULL);
}

std::vector<float> global;
void print_global(){
  for(std::vector<int>::size_type i = 0; i != global.size(); i++) {
    std::cout << "element= " << global[i] << std::endl;
  }
}

void add_triangle(float *input){
  std::vector<float> v(input, input + sizeof(input)+1);
  global.insert( global.end(), v.begin(), v.end() );
}


void *render(void *arg)
{
  int i;
  Shared *shared = (Shared *) arg;
  float *triangle;
  for (i = 0; i < 3; i =i +1) {
    triangle = queue_pop(shared->queue);
    print_triangle(POP, triangle);
    add_triangle(triangle);

  }
  print_global();
  pthread_exit(NULL);
}

void mydisplay() {

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glColor3f(0,0,1);
  glBegin(GL_TRIANGLES);
    for(std::vector<int>::size_type i = 0; i != global.size(); i=i+3) {
      glVertex3f(global[i], global[i+1], global[i+2]); 
    }

  glEnd();
  glFlush();
  glutSwapBuffers();
  glutPostRedisplay();
} 

// void init(void) 
// {
  
// }

int main(int argc, char** argv)
{
  Shared *shared = make_shared();
  pthread_t parser = make_thread(parse, shared);
  pthread_t renderer = make_thread(render, shared);
  
  join_thread(renderer);
  join_thread(parser);

  glutInit(&argc, argv);

  //then your rest of code follows 

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
  glutInitWindowSize(500, 500);
  glutCreateWindow("simple");      
  glutDisplayFunc(mydisplay);     
  glutMainLoop(); 
  return 0;
}