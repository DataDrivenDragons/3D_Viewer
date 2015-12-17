#include <stdio.h>
#include <stdlib.h>
// #include <GL/gl.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <regex.h>
#include <string.h>
#include <vector>
#include <math.h>
#include "utils.h"
#include "wrlParser.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

char PUSH[] = "Pushed";
char POP[] = "Popped";

// ACTUAL APPLICATION CODE

static GLfloat anglex = 0.0;
static GLfloat angley = 0.0;
static GLfloat anglez = 0.0;

char *loadshader(char *filename)
{
  std::string strbuf;
  std::string line;
  std::ifstream in(filename);
  while(std::getline(in,line))
    strbuf += line + '\n';

  char *buf = (char *)malloc(strbuf.size()*sizeof(char));
  strcpy(buf,strbuf.c_str());

  return buf;
}

GLuint programObj = 0;
GLint vstime;

void init(char *vsfilename, char *fsfilename) 
{
  GLfloat white[] = {1.0,1.0,1.0,1.0};
  GLfloat redamb[] = {0.792157, 0.819608, 0.933333,0.0};
  GLfloat reddiff[] = {0.792157, 0.819608, 0.933333,0.0};
  GLfloat lpos[] = {2.0,5.0,2.0,1.0};

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  glLightfv(GL_LIGHT0, GL_AMBIENT, white);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redamb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, reddiff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0);

  glClearColor (0.0, 0.0, 0.0, 0.0);
  /* glShadeModel (GL_FLAT); */
  glEnable(GL_DEPTH_TEST);

  GLchar *vertexShaderCodeStr = loadshader(vsfilename);
  const GLchar **vertexShaderCode = (const GLchar **)&vertexShaderCodeStr;

  GLchar *fragmentShaderCodeStr = loadshader(fsfilename);
  const GLchar **fragmentShaderCode = (const GLchar **)&fragmentShaderCodeStr;

  int status;
  GLint infologLength = 0;
  GLint charsWritten = 0;
  GLchar infoLog[10000];

  GLuint vertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShaderObj, 1, vertexShaderCode, 0);
  glCompileShader(vertexShaderObj); /* Converts to GPU code */

  GLuint fragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShaderObj, 1, fragmentShaderCode, 0);
  glCompileShader(fragmentShaderObj); /* Converts to GPU code */

//  GLuint
  programObj = glCreateProgram();
  glAttachObjectARB(programObj,vertexShaderObj);
  glAttachObjectARB(programObj,fragmentShaderObj);
  glLinkProgram(programObj);  /* Connects shaders & variables */
  glUseProgram(programObj); /* OpenGL now uses the shader */

  vstime = glGetUniformLocationARB(programObj,"time");

  glGetProgramivARB(programObj, GL_LINK_STATUS, &status);

  glGetObjectParameterivARB(programObj,GL_OBJECT_INFO_LOG_LENGTH_ARB,&infologLength);

  if(infologLength > 0) {
    glGetInfoLogARB(programObj,infologLength,&charsWritten,infoLog);
    if(charsWritten)
      printf("InfoLog:\n%s\n\n",infoLog);
  }
}

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
  static GLfloat angle = 0.0;
  GLfloat lpos[4];
  lpos[0] = 0.0; lpos[1] = 5.0; lpos[2] = 2.0; lpos[3] = 1.0;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glUniform1fARB(vstime,angle);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(50.0, 1.0, 3.0, 7.0);
  // glColor3f(0,0,1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  glRotatef(angle, 0.0, 1.0, 0.0);
  // gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); 
  // gluLookAt (0.25, 0.75, 1.0, 0.0, 0.25, 0.0, 0.0, 1.0, 0.0);
  // glRotatef(angley, 0.0,0.0,1.0); 
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  glScalef(100,100,100);
  // glColor3f(0,0,i%255);
  glBegin(GL_TRIANGLES);
    for(std::vector<int>::size_type i = 0; i != global.size(); i=i+3) {
      
      // fprintf(stdout, "Size of triangles: %f %f %f\n", global[i+2], global[i], global[i+1]);
      glVertex3f(global[i], global[i+1], global[i+2]); 
    }

  glEnd();
  // glColor3f(1,1,1);
  // for(std::vector<int>::size_type i = 0; i != global.size(); i=i+9) {
  //     glBegin(GL_LINE_STRIP);
  //     // fprintf(stdout, "Size of triangles: %f %f %f\n", global[i+2], global[i], global[i+1]);
  //       glVertex3f(global[i], global[i+1], global[i+2]); 
  //       glVertex3f(global[i+3], global[i+4], global[i+5]); 
  //       glVertex3f(global[i+6], global[i+7], global[i+8]); 
  //     glEnd();
  // }

  glFlush();
  glutSwapBuffers();
  angle += 0.1;
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

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
  glutInitWindowSize(500, 500);
  glutCreateWindow("simple");  
    glewInit();
  //then your rest of code follows 
  init(argv[1],argv[2]);    
  glutDisplayFunc(mydisplay);
  glutKeyboardFunc(keyboard);
  glutMainLoop(); 
  return 0;
}