// ============================================================================
//  A Day Through the Eyes of People  -  main.cpp
//  Window setup, callbacks and the single central timer.
// ============================================================================
#include <GL/freeglut.h>
#include <cstdio>
#include "Config.h"
#include "Animation.h"
#include "Utils.h"
#include "Scenes.h"

// MinGW's gl.h only knows OpenGL 1.1, so GL_MULTISAMPLE may be missing.
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

static void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);     // clears the whole window (black bars outside the 16:9 view)
    scenesDraw();
    glutSwapBuffers();
}

static void onReshape(int w, int h) { utilsOnResize(w, h); }

// The ONE timer: advance the clock, update the story, redraw, schedule the next tick.
static void onTimer(int) {
    clockTick(glutGet(GLUT_ELAPSED_TIME));
    scenesUpdate(gClock.dt);
    glutPostRedisplay();
    glutTimerFunc(Config::TIMER_MS, onTimer, 0);
}

static void onKeyboard(unsigned char key, int, int) {
    switch (key) {
        case 27:  glutLeaveMainLoop(); break;          // Esc
        case ' ': clockTogglePause();  break;
        case 'n': case 'N': scenesNext(); break;       // (final polish of N/P comes in Phase 6)
        case 'p': case 'P': scenesPrev(); break;
    }
}

static void onSpecial(int key, int, int) {
    if (key == GLUT_KEY_RIGHT) clockSpeedUp();
    if (key == GLUT_KEY_LEFT)  clockSlowDown();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutSetOption(GLUT_MULTISAMPLE, 4);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
    glutInitWindowSize(Config::WINDOW_W, Config::WINDOW_H);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("A Day Through the Eyes of People");

    std::printf("GL vendor  : %s\n", glGetString(GL_VENDOR));
    std::printf("GL renderer: %s\n", glGetString(GL_RENDERER));
    std::printf("GL version : %s\n", glGetString(GL_VERSION));

    glEnable(GL_BLEND);                                   // normal alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glClearColor(0, 0, 0, 1);

    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
    glutKeyboardFunc(onKeyboard);
    glutSpecialFunc(onSpecial);

    scenesInit();
    clockStart(glutGet(GLUT_ELAPSED_TIME));
    glutTimerFunc(Config::TIMER_MS, onTimer, 0);
    glutMainLoop();
    return 0;
}
