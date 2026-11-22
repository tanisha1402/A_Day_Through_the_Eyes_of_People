// Environment.h - reusable scenery objects built from the Utils primitives.
// Objects that stand on the ground use (x, y) = bottom-CENTRE.
#pragma once
#include "Utils.h"

void drawCloud(float x, float y, float scale, Color c);            // (x,y) = centre
void drawTree(float x, float y, float scale, Color leaf);
void drawWindow(float x, float y, float w, float h, bool lit);     // (x,y) = bottom-left
void drawHouse(float x, float y, float w, float h, Color wall, Color roof, bool lit);
// Wavy hill silhouette across the whole screen. scrollX shifts it sideways, so
// layers scrolled at different speeds give parallax depth.
void drawHillLayer(float baseY, float amplitude, float wavelength, float scrollX,
                   Color top, Color bottom);
// Twinkling stars (GL_POINTS). alpha scales overall brightness (0 = invisible).
void drawStarField(float x, float y, float w, float h, int count, float time, float alpha);
// Streak with a fading tail. tail = head - (tailDX, tailDY).
void drawShootingStar(float headX, float headY, float tailDX, float tailDY, float alpha);

// World-space hill silhouette from x0 to x1, filled down to bottomY (used with the camera).
void drawHillLayerRange(float x0, float x1, float baseY, float amplitude, float wavelength,
                        Color top, Color bottom, float bottomY);
// Sun shafts: soft additive triangles fanning out from (sx, sy) toward the lower right.
void drawLightRays(float sx, float sy, float time, float strength, Color c);
// Street lamp; the bulb glows when lit. (x, y) = bottom-centre.
void drawLamp(float x, float y, float scale, bool lit);
