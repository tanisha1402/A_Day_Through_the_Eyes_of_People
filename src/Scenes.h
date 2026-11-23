// Scenes.h - scene manager: chapter timing, transitions, captions, per-scene scripts.
#pragma once

void scenesInit();
void scenesUpdate(float dt);      // advances the chapter clock (dt already includes speed/pause)
void scenesDraw();
void scenesNext();                // jump to the next implemented chapter
void scenesPrev();
void scenesGoto(int index, float time);   // used by tests/tools
float sceneHourAt(int index, float time);   // story hour at a moment of a chapter
int   sceneIndex();
float sceneTime();
