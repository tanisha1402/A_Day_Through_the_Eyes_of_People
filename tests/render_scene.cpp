// Renders a chapter at a given time to PPM. Usage: render_scene out.ppm <chapter 0-2> <seconds>
#include "../src/Utils.h"
#include "../src/Animation.h"
#include "../src/Scenes.h"
#include "mock_gl.h"
#include <GL/freeglut.h>
#include <cstdlib>
int main(int argc, char** argv) {
    if (argc < 4) return 1;
    const int K = 2; mockInitRaster(1280 * K, 720 * K); utilsOnResize(1280 * K, 720 * K);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    int ch = std::atoi(argv[2]); float target = (float)std::atof(argv[3]);
    clockStart(0); scenesInit(); scenesGoto(ch, 0);
    for (int ms = 16; ms <= (int)(target * 1000); ms += 16) { clockTick(ms); scenesUpdate(gClock.dt); }
    glClear(GL_COLOR_BUFFER_BIT);
    scenesDraw();
    mockWritePPM(argv[1], K);
    return 0;
}
