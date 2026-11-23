#include "Viewports.h"
#include <GL/freeglut.h>

static PanelRect   gPanel = { 0, 0, 1280, 720 };
static ScissorRect gPanelPx = { 0, 0, 1, 1 };

void beginPanel(const PanelRect& r) {
    gPanel = r;
    gPanelPx = logicalToScissor(r.x, r.y, r.w, r.h, utilsGetViewport());   // logical -> window pixels
    glViewport(gPanelPx.x, gPanelPx.y, gPanelPx.w, gPanelPx.h);            // the panel gets its own view...
    glEnable(GL_SCISSOR_TEST);                                              // ...and nothing may draw outside it
    glScissor(gPanelPx.x, gPanelPx.y, gPanelPx.w, gPanelPx.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, r.w, 0, r.h, -1, 1);                                         // panel-local coordinates
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void endPanel() {
    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);
    Viewport v = utilsGetViewport();
    glViewport(v.x, v.y, v.w, v.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Config::WINDOW_W, 0, Config::WINDOW_H, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void panelClipBegin(float lx, float ly, float w, float h) {
    ScissorRect s = logicalToScissor(gPanel.x + lx, gPanel.y + ly, w, h, utilsGetViewport());
    glScissor(s.x, s.y, s.w, s.h);
}
void panelClipEnd() { glScissor(gPanelPx.x, gPanelPx.y, gPanelPx.w, gPanelPx.h); }

void drawPanelFrames(const PanelLayout& L) {
    for (int i = 0; i < L.count; ++i) {
        const PanelRect& r = L.r[i];
        drawRectOutline(r.x + 0.5f, r.y + 0.5f, r.w - 1, r.h - 1, rgba(0.05f, 0.05f, 0.07f, 1.0f), 3.0f);
        drawRectOutline(r.x + 2.0f, r.y + 2.0f, r.w - 4, r.h - 4, rgba(1, 1, 1, 0.22f), 1.0f);
    }
}
