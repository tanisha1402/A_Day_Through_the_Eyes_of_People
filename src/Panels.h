// Panels.h - the content of every split-screen panel (scenes 4, 5 and 6).
// Each draw function works in panel-local coordinates: (0,0) = bottom-left of the panel, w x h = its size.
#pragma once
#include "Utils.h"

void  panelsReset(int sceneIndex);                          // clear/prepare particles when a chapter starts
void  panelsUpdate(float dt, int sceneIndex, float t);      // advance rain, dust, notes, drips
float lightningLevel(float t);                              // 0..1 flash brightness at chapter time t (scene 6)

// Scene 4 - Same Clock (three columns)
void drawClassroomPanel(float w, float h, float t);         // Hana, warm
void drawOfficePanel(float w, float h, float t);            // Kenji, grey
void drawPianoPanel(float w, float h, float t);             // Aoi, cold blue
// Scene 5 - Same Cloud (two columns). cloudX/cloudY are in 1280x720 screen coordinates,
// panelX is this panel's left edge, so the SAME cloud is drawn at (cloudX - panelX) in each panel.
void drawRooftopPanel(float w, float h, float t, float panelX, float cloudX, float cloudY);   // Kenji
void drawFieldPanel(float w, float h, float t, float panelX, float cloudX, float cloudY);     // Granny Sato
// Scene 6 - Same Rain (2x2 grid)
void drawAoiWindowPanel(float w, float h, float t, float flash);
void drawHanaStreetPanel(float w, float h, float t, float flash);
void drawRenHutPanel(float w, float h, float t, float flash);
void drawKenjiRunPanel(float w, float h, float t, float flash);

void drawWallClock(float cx, float cy, float r, float hour, float seconds, Color face, Color rim, Color ink);
