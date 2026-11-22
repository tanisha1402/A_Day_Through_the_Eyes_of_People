// Renders a character line-up for visual review.
#include "../src/Utils.h"
#include "../src/Characters.h"
#include "../src/Environment.h"
#include "mock_gl.h"
#include <GL/freeglut.h>
int main(int argc, char** argv) {
    const int K = 2; mockInitRaster(1280 * K, 720 * K); utilsOnResize(1280 * K, 720 * K);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);
    drawGradientRect(0, 0, 1280, 720, rgb(0.85f, 0.9f, 0.95f), rgb(0.55f, 0.75f, 0.95f));
    drawRect(0, 0, 1280, 200, rgb(0.45f, 0.62f, 0.40f));
    PersonStyle st[6] = { makeRen(), makeAoi(), makeHana(), makeKenji(), makeGranny(), makeNeighbor() };
    float sc = 2.6f;
    for (int i = 0; i < 6; ++i) {
        PersonPose p = makePose(1, 3.0f); p.walkAmount = (i % 2) ? 1.0f : 0.0f; p.walkPhase = 1.0f + i;
        if (i == 3) p.armUp = 1.0f;
        drawPerson(110 + i * 205, 170, sc, st[i], p);
    }
    drawCat(300, 80, 2.5f, 1, 1.0f, 1.0f, 2.0f, false);
    drawCat(600, 80, 2.5f, 1, 0, 0, 2.0f, true);
    drawCat(900, 80, 2.5f, -1, 0.5f, 1.0f, 2.0f, false);
    mockWritePPM(argv[1], K);
    return 0;
}
