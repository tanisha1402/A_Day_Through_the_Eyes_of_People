// ============================================================================
//  A Day Through the Eyes of People  -  Phase 1 skeleton
//  Purpose: verify that C++ / OpenGL / freeGLUT build and run on your machine
//  and establish the three things every later phase depends on:
//    1. a 1280x720 logical coordinate system (origin bottom-left)
//    2. a single centralized timer that computes a frame time (dt)
//    3. a 16:9 viewport that letterboxes when the window is resized
//  This file will be split into modules in Phase 2 (see docs/PROJECT_PLAN.md).
// ============================================================================
#include <GL/freeglut.h>
#include <cstdio>
#include <cmath>

// MinGW's gl.h only knows OpenGL 1.1, so GL_MULTISAMPLE may be missing.
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

namespace Config {
    const int   WINDOW_W   = 1280;   // logical and initial window width
    const int   WINDOW_H   = 720;    // logical and initial window height
    const int   TIMER_MS   = 16;     // requested timer period (~60 fps)
    const float LETTERBOX  = 50.0f;  // cinematic bar height in logical pixels
    const float MAX_DT     = 0.05f;  // clamp so a hiccup never causes a jump
}

// ---- Global animation state (kept deliberately small) ----------------------
static float gTime     = 0.0f;   // animation seconds (scaled by speed, frozen when paused)
static float gSpeed    = 1.0f;   // animation speed multiplier
static bool  gPaused   = false;
static int   gLastMs   = 0;      // time of the previous tick
static float gFps      = 0.0f;
static int   gFrames   = 0;
static int   gFpsStart = 0;

// ---- Tiny drawing helpers (the real versions arrive in Phase 2) -------------
// Vertical gradient: a different colour at the bottom and top vertices.
static void drawGradientRect(float x0, float y0, float x1, float y1,
                             float br, float bg, float bb,
                             float tr, float tg, float tb) {
    glBegin(GL_QUADS);
        glColor3f(br, bg, bb); glVertex2f(x0, y0); glVertex2f(x1, y0);
        glColor3f(tr, tg, tb); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

// Filled circle with an alpha value. Several stacked circles make a soft glow.
static void drawCircle(float cx, float cy, float r, float R, float G, float B, float A) {
    const int SEGMENTS = 48;
    glColor4f(R, G, B, A);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= SEGMENTS; ++i) {
            float a = 2.0f * 3.14159265f * i / SEGMENTS;
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
    glEnd();
}

static void drawText(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)s);
}

// ---- Display ----------------------------------------------------------------
static void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Dawn sky gradient
    drawGradientRect(0, 0, 1280, 720, 1.00f, 0.62f, 0.38f,  0.10f, 0.15f, 0.40f);

    // Sun rising and falling slowly, with a glow made from stacked translucent circles
    float sunY = 230.0f + 40.0f * std::sin(gTime * 0.8f);
    for (int i = 6; i >= 1; --i)
        drawCircle(640, sunY, 40.0f + i * 28.0f, 1.0f, 0.85f, 0.55f, 0.07f);
    drawCircle(640, sunY, 40.0f, 1.0f, 0.95f, 0.75f, 1.0f);

    // Two hills (GL_TRIANGLES) in front of the sun
    glBegin(GL_TRIANGLES);
        glColor3f(0.16f, 0.20f, 0.30f); glVertex2f(-100, 0); glVertex2f(520, 0);  glVertex2f(200, 260);
        glColor3f(0.10f, 0.14f, 0.24f); glVertex2f(380, 0);  glVertex2f(1380, 0); glVertex2f(900, 200);
    glEnd();

    // A thin outline loop, to confirm GL_LINE_LOOP works (used for outlines later)
    glColor3f(0.05f, 0.07f, 0.12f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-100, 0); glVertex2f(520, 0); glVertex2f(200, 260);
    glEnd();

    // Cinematic letterbox bars
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);   glVertex2f(1280, 0);   glVertex2f(1280, Config::LETTERBOX);   glVertex2f(0, Config::LETTERBOX);
        glVertex2f(0, 720); glVertex2f(1280, 720); glVertex2f(1280, 720 - Config::LETTERBOX); glVertex2f(0, 720 - Config::LETTERBOX);
    glEnd();

    // Status text
    char line[128];
    glColor3f(1, 1, 1);
    drawText(24, 690, "A Day Through the Eyes of People  -  Phase 1 environment check");
    std::snprintf(line, sizeof line, "FPS: %.0f   time: %.1f s   speed: %.2fx   %s",
                  gFps, gTime, gSpeed, gPaused ? "[PAUSED]" : "");
    drawText(24, 20, line);
    drawText(760, 20, "Space: pause   Left/Right: speed   Esc: exit");

    glutSwapBuffers();
}

// ---- Resize: keep a centred 16:9 viewport, black bars elsewhere -------------
static void onReshape(int w, int h) {
    const float aspect = 16.0f / 9.0f;
    int vw = w, vh = (int)(w / aspect);
    if (vh > h) { vh = h; vw = (int)(h * aspect); }
    glViewport((w - vw) / 2, (h - vh) / 2, vw, vh);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Config::WINDOW_W, 0, Config::WINDOW_H, -1, 1);   // logical 1280x720
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ---- The one and only timer: computes dt and requests a redraw --------------
static void onTimer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - gLastMs) / 1000.0f;       // real seconds since last tick
    gLastMs = now;
    if (dt > Config::MAX_DT) dt = Config::MAX_DT;

    if (!gPaused) gTime += dt * gSpeed;

    ++gFrames;
    if (now - gFpsStart >= 500) {               // refresh FPS twice per second
        gFps = gFrames * 1000.0f / (now - gFpsStart);
        gFrames = 0;
        gFpsStart = now;
    }

    glutPostRedisplay();
    glutTimerFunc(Config::TIMER_MS, onTimer, 0);
}

// ---- Input -------------------------------------------------------------------
static void onKeyboard(unsigned char key, int, int) {
    if (key == 27)  glutLeaveMainLoop();        // Esc
    if (key == ' ') gPaused = !gPaused;
}

static void onSpecial(int key, int, int) {
    if (key == GLUT_KEY_RIGHT && gSpeed < 4.0f)  gSpeed *= 1.25f;
    if (key == GLUT_KEY_LEFT  && gSpeed > 0.25f) gSpeed /= 1.25f;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutSetOption(GLUT_MULTISAMPLE, 4);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
    glutInitWindowSize(Config::WINDOW_W, Config::WINDOW_H);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("A Day Through the Eyes of People");

    // Environment report in the console window
    std::printf("GL vendor  : %s\n", glGetString(GL_VENDOR));
    std::printf("GL renderer: %s\n", glGetString(GL_RENDERER));
    std::printf("GL version : %s\n", glGetString(GL_VERSION));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0, 0, 0, 1);

    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
    glutKeyboardFunc(onKeyboard);
    glutSpecialFunc(onSpecial);

    gLastMs = gFpsStart = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(Config::TIMER_MS, onTimer, 0);
    glutMainLoop();
    return 0;
}
