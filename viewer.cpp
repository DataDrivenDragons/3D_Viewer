#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <pthread.h>
#include <iostream>
#include <regex.h>
#include <string.h>
#include <vector>
#include <math.h>
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
  free(input);
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
    if (abs(*triangle) > 1000){
      std::cout << "null " << std::endl;
      free(triangle);
      break;
    }
    float * face;
    face = (float *) malloc(sizeof(float)*9);
    for (int j = 0; j<9; j++) {
      face[j] = triangle[j];
    }
    // free(triangle);
    // print_triangle(POP, triangle);
    add_triangle(face);

  }
  std::cout << "done " << std::endl;
  // print_global();
  pthread_exit(NULL);
}

void mydisplay() {


  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glColor3f(0,0,1);

  glLoadIdentity ();
  // gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); 
  // gluLookAt (0.25, 0.75, 1.0, 0.0, 0.25, 0.0, 0.0, 1.0, 0.0);
  // glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  // glRotatef(angley, 0.0,0.0,1.0); 
  glScalef(50,50,50);
  // glColor3f(0,0,i%255);
  glBegin(GL_TRIANGLES);
    for(std::vector<int>::size_type i = 0; i != global.size(); i=i+3) {
      
      // fprintf(stdout, "Size of triangles: %f %f %f\n", global[i+2], global[i], global[i+1]);
      glVertex3f(global[i], global[i+1], global[i+2]); 
    }

  glEnd();
  glColor3f(1,1,1);
  for(std::vector<int>::size_type i = 0; i != global.size(); i=i+9) {
      glBegin(GL_LINE_STRIP);
      // fprintf(stdout, "Size of triangles: %f %f %f\n", global[i+2], global[i], global[i+1]);
        glVertex3f(global[i], global[i+1], global[i+2]); 
        glVertex3f(global[i+3], global[i+4], global[i+5]); 
        glVertex3f(global[i+6], global[i+7], global[i+8]); 
      glEnd();
  }

  glFlush();
  glutSwapBuffers();

  glutPostRedisplay();
} 

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
    case 'a':
      angley += 1.0f;
      break;
    case 'd':
      angley += -1.0f;
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