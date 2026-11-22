// Renders frames to PPM using the software rasterizer.  Usage: render out.ppm test <page> <seconds>
#include "../src/Utils.h"
#include "../src/Animation.h"
#include "TestScreen.h"
#include "mock_gl.h"
#include <GL/freeglut.h>
#include <cstdlib>
#include <cstring>
int main(int argc, char** argv) {
    if (argc < 5) return 1;
    const int K = 2;                                   // render 2x, downscale for anti-aliasing
    mockInitRaster(1280 * K, 720 * K);
    utilsOnResize(1280 * K, 720 * K);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clockStart(0);
    int page = std::atoi(argv[3]); float target = (float)std::atof(argv[4]);
    for (int i = 0; i < page; ++i) testScreenNextPage();
    for (int ms = 16; ms <= (int)(target * 1000); ms += 16) clockTick(ms);
    glClear(GL_COLOR_BUFFER_BIT);
    testScreenDraw();
    mockWritePPM(argv[1], K);
    return 0;
}
