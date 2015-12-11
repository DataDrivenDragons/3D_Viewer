#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <pthread.h>
#include <iostream>
#include <regex.h>
#include <string.h>
#include <vector>
#include "utils.h"
#include "wrlParser.h"

char PUSH[] = "Pushed";
char POP[] = "Popped";

// ACTUAL APPLICATION CODE

static GLfloat anglex = 0.0;
static GLfloat angley = 0.0;
static GLfloat anglez = 0.0;

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
  char path[] = "die.wrl";
  parseFunc(path, shared->queue);
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
  // for (i = 0; i < 10; i =i +1) {
  while (true){
    triangle = queue_pop(shared->queue);
    if (*triangle == 0.0){
      std::cout << "null " << std::endl;
      break;
    }
    print_triangle(POP, triangle);
    add_triangle(triangle);

  }
  std::cout << "done " << std::endl;
  // print_global();
  pthread_exit(NULL);
}

void mydisplay() {
  
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glColor3f(0,0,1);
  glScalef(300,300,300);
  glRotatef(2.0, anglex,0.0f,angley);
  glBegin(GL_TRIANGLES);
    for(std::vector<int>::size_type i = 0; i != global.size(); i=i+3) {
      glVertex3f(global[i], global[i+1], global[i+2]); 
    }

  glEnd();
  glFlush();
  glutSwapBuffers();

  anglex = 0.0;
  angley = 0.0;
  glutPostRedisplay();
} 

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
    case 'a':
      angley = 1.0f;
      break;
    case 'd':
      angley = -1.0f;
      break;
    case 'w':
      anglex = 1.0f;
      break;
    case 's':
      anglex = -1.0f;
      break;
    case 27:
      exit(0);
      break;
   }
}

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
  glutKeyboardFunc(keyboard);
  glutMainLoop(); 
  return 0;
}