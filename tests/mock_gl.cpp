// Headless fake OpenGL: records calls and flags invalid usage. Lets us run the
// real drawing code without a window or GPU.
#include <GL/freeglut.h>
#include "mock_gl.h"
#include <cmath>
#include <cstdio>
#include <cstring>

MockStats gMock;

static void err(const char* msg) {
    if (gMock.errors == 0) std::snprintf(gMock.firstError, sizeof gMock.firstError, "%s", msg);
    ++gMock.errors;
}
static void notInBegin(const char* fn) { if (gMock.inBegin) { char b[128]; std::snprintf(b, sizeof b, "%s called inside glBegin/glEnd", fn); err(b); } }
static void checkColor(float v) { if (!std::isfinite(v) || v < -0.001f || v > 1.001f) err("colour component out of 0..1"); }

void mockFrameCheck(const char* where) {
    char b[160];
    if (gMock.inBegin)         { std::snprintf(b, sizeof b, "%s: glBegin without glEnd", where); err(b); }
    if (gMock.matrixDepth != 0){ std::snprintf(b, sizeof b, "%s: matrix stack depth %d", where, gMock.matrixDepth); err(b); }
    if (gMock.scissorOn)       { std::snprintf(b, sizeof b, "%s: scissor left enabled", where); err(b); }
    if (gMock.blendS != GL_SRC_ALPHA || gMock.blendD != GL_ONE_MINUS_SRC_ALPHA) { std::snprintf(b, sizeof b, "%s: blend mode not restored", where); err(b); }
    if (gMock.vertices > gMock.maxVertices) gMock.maxVertices = gMock.vertices;
    gMock.vertices = 0;
}

extern "C" {
void glClear(GLbitfield) { notInBegin("glClear"); if (gMock.scissorOn) err("glClear while scissor enabled"); }
void glBegin(GLenum) { if (gMock.inBegin) err("nested glBegin"); gMock.inBegin = true; }
void glEnd() { if (!gMock.inBegin) err("glEnd without glBegin"); gMock.inBegin = false; }
void glColor3f(GLfloat r, GLfloat g, GLfloat b) { checkColor(r); checkColor(g); checkColor(b); }
void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { checkColor(r); checkColor(g); checkColor(b); checkColor(a); }
void glVertex2f(GLfloat x, GLfloat y) {
    if (!gMock.inBegin) err("glVertex2f outside glBegin");
    if (!std::isfinite(x) || !std::isfinite(y)) err("non-finite vertex");
    ++gMock.vertices;
}
void glRasterPos2f(GLfloat, GLfloat) { notInBegin("glRasterPos2f"); }
void glViewport(GLint, GLint, GLsizei, GLsizei) { notInBegin("glViewport"); }
void glScissor(GLint x, GLint y, GLsizei w, GLsizei h) { notInBegin("glScissor"); if (w < 0 || h < 0 || x < -2 || y < -2) err("bad scissor rectangle"); }
void glMatrixMode(GLenum) { notInBegin("glMatrixMode"); }
void glLoadIdentity() { notInBegin("glLoadIdentity"); }
void glPushMatrix() { notInBegin("glPushMatrix"); ++gMock.matrixDepth; }
void glPopMatrix()  { notInBegin("glPopMatrix"); if (--gMock.matrixDepth < 0) err("glPopMatrix underflow"); }
void glTranslatef(GLfloat, GLfloat, GLfloat) { notInBegin("glTranslatef"); }
void glRotatef(GLfloat, GLfloat, GLfloat, GLfloat) { notInBegin("glRotatef"); }
void glScalef(GLfloat, GLfloat, GLfloat) { notInBegin("glScalef"); }
void glOrtho(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble) { notInBegin("glOrtho"); }
void glEnable(GLenum cap)  { notInBegin("glEnable");  if (cap == GL_SCISSOR_TEST) gMock.scissorOn = true; }
void glDisable(GLenum cap) { notInBegin("glDisable"); if (cap == GL_SCISSOR_TEST) gMock.scissorOn = false; }
void glBlendFunc(GLenum s, GLenum d) { notInBegin("glBlendFunc"); gMock.blendS = s; gMock.blendD = d; }
void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat) {}
void glLineWidth(GLfloat w) { notInBegin("glLineWidth"); if (w <= 0) err("line width <= 0"); }
void glPointSize(GLfloat w) { notInBegin("glPointSize"); if (w <= 0) err("point size <= 0"); }
const GLubyte* glGetString(GLenum) { return (const GLubyte*)"mock"; }
void glutInit(int*, char**) {} void glutSetOption(GLenum, int) {} void glutInitDisplayMode(unsigned) {}
void glutInitWindowSize(int, int) {} void glutInitWindowPosition(int, int) {} int glutCreateWindow(const char*) { return 1; }
void glutDisplayFunc(void (*)()) {} void glutReshapeFunc(void (*)(int, int)) {}
void glutKeyboardFunc(void (*)(unsigned char, int, int)) {} void glutSpecialFunc(void (*)(int, int, int)) {}
void glutTimerFunc(unsigned, void (*)(int), int) {} void glutMainLoop() {} void glutSwapBuffers() {}
void glutPostRedisplay() {} int glutGet(GLenum) { return 0; } void glutLeaveMainLoop() {}
void glutBitmapString(void*, const unsigned char*) { notInBegin("glutBitmapString"); ++gMock.textCalls; }
}
