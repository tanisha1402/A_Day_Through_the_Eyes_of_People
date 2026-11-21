// Minimal stand-in for <GL/freeglut.h>, used ONLY by the headless tests in tests/.
// Not used by Code::Blocks (which uses the real freeGLUT headers).
#pragma once
typedef float GLfloat; typedef unsigned int GLenum; typedef unsigned int GLbitfield;
typedef int GLint; typedef int GLsizei; typedef unsigned char GLubyte; typedef double GLdouble;
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_POINTS 0
#define GL_LINES 1
#define GL_LINE_LOOP 2
#define GL_TRIANGLES 4
#define GL_TRIANGLE_FAN 6
#define GL_QUADS 7
#define GL_POLYGON 9
#define GL_PROJECTION 0x1701
#define GL_MODELVIEW 0x1700
#define GL_BLEND 0x0BE2
#define GL_SCISSOR_TEST 0x0C11
#define GL_LINE_SMOOTH 0x0B20
#define GL_POINT_SMOOTH 0x0B10
#define GL_ONE 1
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GLUT_DOUBLE 2
#define GLUT_RGB 0
#define GLUT_MULTISAMPLE 128
#define GLUT_ELAPSED_TIME 700
#define GLUT_KEY_LEFT 100
#define GLUT_KEY_RIGHT 102
#define GLUT_BITMAP_HELVETICA_12 ((void*)7)
#define GLUT_BITMAP_HELVETICA_18 ((void*)8)
extern "C" {
void glClear(GLbitfield); void glBegin(GLenum); void glEnd();
void glColor3f(GLfloat,GLfloat,GLfloat); void glColor4f(GLfloat,GLfloat,GLfloat,GLfloat);
void glVertex2f(GLfloat,GLfloat); void glRasterPos2f(GLfloat,GLfloat);
void glViewport(GLint,GLint,GLsizei,GLsizei); void glScissor(GLint,GLint,GLsizei,GLsizei);
void glMatrixMode(GLenum); void glLoadIdentity(); void glPushMatrix(); void glPopMatrix();
void glTranslatef(GLfloat,GLfloat,GLfloat); void glRotatef(GLfloat,GLfloat,GLfloat,GLfloat); void glScalef(GLfloat,GLfloat,GLfloat);
void glOrtho(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
void glEnable(GLenum); void glDisable(GLenum); void glBlendFunc(GLenum,GLenum);
void glClearColor(GLfloat,GLfloat,GLfloat,GLfloat); void glLineWidth(GLfloat); void glPointSize(GLfloat);
const GLubyte* glGetString(GLenum);
void glutInit(int*,char**); void glutSetOption(GLenum,int); void glutInitDisplayMode(unsigned);
void glutInitWindowSize(int,int); void glutInitWindowPosition(int,int); int glutCreateWindow(const char*);
void glutDisplayFunc(void(*)()); void glutReshapeFunc(void(*)(int,int)); void glutKeyboardFunc(void(*)(unsigned char,int,int));
void glutSpecialFunc(void(*)(int,int,int)); void glutTimerFunc(unsigned,void(*)(int),int);
void glutMainLoop(); void glutSwapBuffers(); void glutPostRedisplay(); int glutGet(GLenum);
void glutLeaveMainLoop(); void glutBitmapString(void*,const unsigned char*);
}
