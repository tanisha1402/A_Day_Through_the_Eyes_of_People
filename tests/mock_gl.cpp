// Headless fake OpenGL: records calls and flags invalid usage. Lets us run the
// real drawing code without a window or GPU.
#include <GL/freeglut.h>
#include "mock_gl.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

MockStats gMock;

// ------------------------------------------------------------- rasterizer ----
// Tiny 2D software renderer: affine modelview, ortho projection, viewport,
// scissor, alpha + additive blending, per-vertex colour interpolation.
struct Mat { float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0; };
struct Vtx { float x, y, r, g, b, a; };
static bool gRasterOn = false;
static int fbW = 0, fbH = 0;
static std::vector<float> fb;
static Mat mv; static std::vector<Mat> mvStack; static GLenum curMode = GL_MODELVIEW;
static float curCol[4] = { 1, 1, 1, 1 };
static float projL = 0, projR = 1, projB = 0, projT = 1;
static int vpX = 0, vpY = 0, vpW = 1, vpH = 1, scX = 0, scY = 0, scW = 1, scH = 1;
static bool blendOn = false;
static float lineW = 1, pointS = 1, rasterX = 0, rasterY = 0;
static std::vector<Vtx> verts; static GLenum beginMode = 0;

void mockInitRaster(int w, int h) { gRasterOn = true; fbW = w; fbH = h; fb.assign((size_t)w * h * 3, 0.0f); }

static void putPixel(int x, int y, float r, float g, float b, float a) {
    float* p = &fb[((size_t)y * fbW + x) * 3];
    if (!blendOn) { p[0] = r; p[1] = g; p[2] = b; return; }
    if (gMock.blendD == GL_ONE) {                       // additive light
        p[0] = std::min(1.0f, p[0] + r * a); p[1] = std::min(1.0f, p[1] + g * a); p[2] = std::min(1.0f, p[2] + b * a);
    } else {                                            // normal alpha blending
        p[0] = p[0] * (1 - a) + r * a; p[1] = p[1] * (1 - a) + g * a; p[2] = p[2] * (1 - a) + b * a;
    }
}
static Vtx project(float x, float y, const float* col) {
    float tx = mv.a * x + mv.c * y + mv.e, ty = mv.b * x + mv.d * y + mv.f;
    float nx = (tx - projL) / (projR - projL) * 2 - 1, ny = (ty - projB) / (projT - projB) * 2 - 1;
    Vtx v; v.x = vpX + (nx + 1) * 0.5f * vpW; v.y = vpY + (ny + 1) * 0.5f * vpH;
    v.r = col[0]; v.g = col[1]; v.b = col[2]; v.a = col[3]; return v;
}
static void fillTri(const Vtx& A, const Vtx& B, const Vtx& C) {
    int cx0 = std::max(0, vpX), cy0 = std::max(0, vpY), cx1 = std::min(fbW, vpX + vpW), cy1 = std::min(fbH, vpY + vpH);
    if (gMock.scissorOn) { cx0 = std::max(cx0, scX); cy0 = std::max(cy0, scY); cx1 = std::min(cx1, scX + scW); cy1 = std::min(cy1, scY + scH); }
    int x0 = std::max(cx0, (int)std::floor(std::min(A.x, std::min(B.x, C.x))));
    int x1 = std::min(cx1, (int)std::ceil(std::max(A.x, std::max(B.x, C.x))));
    int y0 = std::max(cy0, (int)std::floor(std::min(A.y, std::min(B.y, C.y))));
    int y1 = std::min(cy1, (int)std::ceil(std::max(A.y, std::max(B.y, C.y))));
    float area = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);
    if (std::fabs(area) < 1e-9f) return;
    for (int y = y0; y < y1; ++y) for (int x = x0; x < x1; ++x) {
        float px = x + 0.5f, py = y + 0.5f;
        float w0 = ((B.x - px) * (C.y - py) - (B.y - py) * (C.x - px)) / area;
        float w1 = ((C.x - px) * (A.y - py) - (C.y - py) * (A.x - px)) / area;
        float w2 = 1 - w0 - w1;
        if (w0 < 0 || w1 < 0 || w2 < 0) continue;
        putPixel(x, y, w0 * A.r + w1 * B.r + w2 * C.r, w0 * A.g + w1 * B.g + w2 * C.g,
                       w0 * A.b + w1 * B.b + w2 * C.b, w0 * A.a + w1 * B.a + w2 * C.a);
    }
}
static void thickLine(const Vtx& A, const Vtx& B) {
    float sc = fbW / 1280.0f, w = std::max(1.0f, lineW * sc) * 0.5f;
    float dx = B.x - A.x, dy = B.y - A.y, len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-6f) return;
    float nx = -dy / len * w, ny = dx / len * w;
    Vtx a1 = A, a2 = A, b1 = B, b2 = B; a1.x += nx; a1.y += ny; a2.x -= nx; a2.y -= ny; b1.x += nx; b1.y += ny; b2.x -= nx; b2.y -= ny;
    fillTri(a1, a2, b2); fillTri(a1, b2, b1);
}
static void flushPrimitive() {
    size_t n = verts.size();
    switch (beginMode) {
    case GL_POINTS: { float h = std::max(1.0f, pointS * fbW / 1280.0f) * 0.5f;
        for (size_t i = 0; i < n; ++i) { Vtx p = verts[i], a = p, b = p, c = p, d = p; a.x -= h; a.y -= h; b.x += h; b.y -= h; c.x += h; c.y += h; d.x -= h; d.y += h; fillTri(a, b, c); fillTri(a, c, d); } break; }
    case GL_LINES: for (size_t i = 0; i + 1 < n; i += 2) thickLine(verts[i], verts[i + 1]); break;
    case GL_LINE_LOOP: for (size_t i = 0; i < n; ++i) thickLine(verts[i], verts[(i + 1) % n]); break;
    case GL_TRIANGLES: for (size_t i = 0; i + 2 < n; i += 3) fillTri(verts[i], verts[i + 1], verts[i + 2]); break;
    case GL_TRIANGLE_FAN: case GL_POLYGON: for (size_t i = 1; i + 1 < n; ++i) fillTri(verts[0], verts[i], verts[i + 1]); break;
    case GL_QUADS: for (size_t i = 0; i + 3 < n; i += 4) { fillTri(verts[i], verts[i + 1], verts[i + 2]); fillTri(verts[i], verts[i + 2], verts[i + 3]); } break;
    }
}
void mockGetViewport(int& x, int& y, int& w, int& h) { x = vpX; y = vpY; w = vpW; h = vpH; }
void mockWritePPM(const char* path, int k) {
    FILE* f = std::fopen(path, "wb"); if (!f) return;
    int w = fbW / k, h = fbH / k; std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
        float acc[3] = { 0, 0, 0 };
        for (int j = 0; j < k; ++j) for (int i = 0; i < k; ++i) {
            const float* p = &fb[((size_t)(fbH - 1 - (y * k + j)) * fbW + (x * k + i)) * 3];
            acc[0] += p[0]; acc[1] += p[1]; acc[2] += p[2];
        }
        for (int c = 0; c < 3; ++c) std::fputc((int)(std::min(1.0f, acc[c] / (k * k)) * 255 + 0.5f), f);
    }
    std::fclose(f);
}

static void err(const char* msg) {
    if (gMock.errors == 0) std::snprintf(gMock.firstError, sizeof gMock.firstError, "%s", msg);
    ++gMock.errors;
}
static void notInBegin(const char* fn) { if (gMock.inBegin) { char b[128]; std::snprintf(b, sizeof b, "%s called inside glBegin/glEnd", fn); err(b); } }
static void checkColor(float v) { if (!std::isfinite(v) || v < -0.001f || v > 1.001f) { char b[96]; std::snprintf(b, sizeof b, "colour component %.4f out of 0..1", v); err(b); } }

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
void glClear(GLbitfield) { notInBegin("glClear"); if (gMock.scissorOn) err("glClear while scissor enabled"); if (gRasterOn) std::fill(fb.begin(), fb.end(), 0.0f); }
void glBegin(GLenum m) { if (gMock.inBegin) err("nested glBegin"); gMock.inBegin = true; beginMode = m; verts.clear(); }
void glEnd() { if (!gMock.inBegin) err("glEnd without glBegin"); gMock.inBegin = false; if (gRasterOn) flushPrimitive(); }
void glColor3f(GLfloat r, GLfloat g, GLfloat b) { checkColor(r); checkColor(g); checkColor(b); curCol[0] = r; curCol[1] = g; curCol[2] = b; curCol[3] = 1; }
void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { checkColor(r); checkColor(g); checkColor(b); checkColor(a); curCol[0] = r; curCol[1] = g; curCol[2] = b; curCol[3] = a; }
void glVertex2f(GLfloat x, GLfloat y) {
    if (!gMock.inBegin) err("glVertex2f outside glBegin");
    if (!std::isfinite(x) || !std::isfinite(y)) err("non-finite vertex");
    ++gMock.vertices;
    if (gRasterOn) verts.push_back(project(x, y, curCol));
}
void glRasterPos2f(GLfloat x, GLfloat y) { notInBegin("glRasterPos2f"); if (gRasterOn) { Vtx v = project(x, y, curCol); rasterX = v.x; rasterY = v.y; } }
void glViewport(GLint x, GLint y, GLsizei w, GLsizei h) { notInBegin("glViewport"); vpX = x; vpY = y; vpW = w; vpH = h; }
void glScissor(GLint x, GLint y, GLsizei w, GLsizei h) { notInBegin("glScissor"); if (w < 0 || h < 0 || x < -2 || y < -2) err("bad scissor rectangle"); scX = x; scY = y; scW = w; scH = h; }
void glMatrixMode(GLenum m) { notInBegin("glMatrixMode"); curMode = m; }
void glLoadIdentity() { notInBegin("glLoadIdentity"); if (curMode == GL_MODELVIEW) mv = Mat(); }
void glPushMatrix() { notInBegin("glPushMatrix"); ++gMock.matrixDepth; mvStack.push_back(mv); }
void glPopMatrix()  { notInBegin("glPopMatrix"); if (--gMock.matrixDepth < 0) err("glPopMatrix underflow"); else { mv = mvStack.back(); mvStack.pop_back(); } }
void glTranslatef(GLfloat x, GLfloat y, GLfloat) { notInBegin("glTranslatef"); mv.e += mv.a * x + mv.c * y; mv.f += mv.b * x + mv.d * y; }
void glRotatef(GLfloat deg, GLfloat, GLfloat, GLfloat) { notInBegin("glRotatef"); float r = deg * 3.14159265f / 180, cs = std::cos(r), sn = std::sin(r); Mat o = mv; mv.a = o.a * cs + o.c * sn; mv.b = o.b * cs + o.d * sn; mv.c = -o.a * sn + o.c * cs; mv.d = -o.b * sn + o.d * cs; }
void glScalef(GLfloat x, GLfloat y, GLfloat) { notInBegin("glScalef"); mv.a *= x; mv.b *= x; mv.c *= y; mv.d *= y; }
void glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble, GLdouble) { notInBegin("glOrtho"); projL = (float)l; projR = (float)r; projB = (float)b; projT = (float)t; }
void glEnable(GLenum cap)  { notInBegin("glEnable");  if (cap == GL_SCISSOR_TEST) gMock.scissorOn = true; if (cap == GL_BLEND) blendOn = true; }
void glDisable(GLenum cap) { notInBegin("glDisable"); if (cap == GL_SCISSOR_TEST) gMock.scissorOn = false; }
void glBlendFunc(GLenum s, GLenum d) { notInBegin("glBlendFunc"); gMock.blendS = s; gMock.blendD = d; }
void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat) {}
void glLineWidth(GLfloat w) { notInBegin("glLineWidth"); if (w <= 0) err("line width <= 0"); lineW = w; }
void glPointSize(GLfloat w) { notInBegin("glPointSize"); if (w <= 0) err("point size <= 0"); pointS = w; }
const GLubyte* glGetString(GLenum) { return (const GLubyte*)"mock"; }
void glutInit(int*, char**) {} void glutSetOption(GLenum, int) {} void glutInitDisplayMode(unsigned) {}
void glutInitWindowSize(int, int) {} void glutInitWindowPosition(int, int) {} int glutCreateWindow(const char*) { return 1; }
void glutDisplayFunc(void (*)()) {} void glutReshapeFunc(void (*)(int, int)) {}
void glutKeyboardFunc(void (*)(unsigned char, int, int)) {} void glutSpecialFunc(void (*)(int, int, int)) {}
void glutTimerFunc(unsigned, void (*)(int), int) {} void glutMainLoop() {} void glutSwapBuffers() {}
void glutPostRedisplay() {} int glutGet(GLenum) { return 0; } void glutLeaveMainLoop() {}
void glutBitmapString(void*, const unsigned char* s) {
    notInBegin("glutBitmapString"); ++gMock.textCalls;
    if (!gRasterOn) return;                      // crude text: one small box per character, to show placement
    float sc = fbW / 1280.0f, x = rasterX;
    for (; *s; ++s) { if (*s != ' ') { Vtx a = {x, rasterY, curCol[0], curCol[1], curCol[2], 0.55f}, b = a, c = a, d = a; b.x += 6 * sc; c.x += 6 * sc; c.y += 9 * sc; d.y += 9 * sc; fillTri(a, b, c); fillTri(a, c, d); } x += 9 * sc; }
}
int glutBitmapLength(void*, const unsigned char* s) { int n = 0; for (; *s; ++s) n += 9; return n; }
void glutStrokeString(void*, const unsigned char* s) {      // like freeGLUT: advances the current matrix after each character
    notInBegin("glutStrokeString"); ++gMock.textCalls;
    for (; *s; ++s) {
        if (gRasterOn && *s != ' ') { Vtx a = project(8, 0, curCol), b = project(78, 0, curCol), c = project(78, 90, curCol), d = project(8, 90, curCol); a.a = b.a = c.a = d.a = 0.7f; fillTri(a, b, c); fillTri(a, c, d); }
        mv.e += mv.a * 104.76f; mv.f += mv.b * 104.76f;
    }
}
float glutStrokeLength(void*, const unsigned char* s) { float n = 0; for (; *s; ++s) n += 104.76f; return n; }
}
