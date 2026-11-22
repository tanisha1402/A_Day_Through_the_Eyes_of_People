// World.h - the hillside town: terrain, four districts, landmarks, train and crossing.
// World coordinates: x runs left to right over about 5200 units, y is up.
//   Hilltop 0-1300 | Riverside 1300-2500 | Downtown 2500-4000 | Countryside 4000-5200
#pragma once
#include "Utils.h"

namespace World {
    const float MANSION_X   = 620.0f;
    const float HUT_X       = 1780.0f;
    const float DOCK_X0     = 1880.0f, DOCK_X1 = 2110.0f;
    const float RIVER_X0    = 1330.0f, RIVER_X1 = 2480.0f;
    const float WATER_TOP   = 132.0f;
    const float BAKERY_X    = 2640.0f;
    const float CROSSING_X  = 3300.0f;
    const float SCHOOL_X    = 3800.0f;
    const float GATE_X      = 4560.0f;
    const float FARMHOUSE_X = 4800.0f;
    const float TRACK_Y     = 186.0f;       // top of the rails
    const float BUILDING_Y  = 205.0f;       // downtown buildings stand behind the track
}

float groundY(float x);      // height of the ground surface at x
float walkY(float x);        // where characters' feet should be at x

// Everything in the town that changes from scene to scene.
struct TownState {
    float time;              // animation seconds (water, smoke, mist)
    float mist;              // 0..1 river mist
    float arms;              // crossing barrier: 0 = open (up), 1 = closed (down)
    bool  signalOn;          // crossing red lights blinking
    bool  trainOn;
    float trainHeadX;        // x of the front of the train (it travels left to right)
    int   hutCups;           // tea cups on Ren's table
    float boatX;
    bool  lantern;           // Ren's boat lantern
    float sunlight;          // 0..1 sunlight entering Aoi's room
    bool  grannyLight;       // light in Granny Sato's farmhouse window
    TownState() : time(0), mist(0), arms(0), signalOn(false), trainOn(false), trainHeadX(0),
                  hutCups(1), boatX(2250), lantern(false), sunlight(0), grannyLight(false) {}
};

void  drawSky(float time);                     // screen-space: gradient, stars, sun, moon
void  drawFarLayers(float time);               // clouds, mountains, hills (parallax)
void  drawTownBack(const TownState& ts);       // everything behind the characters
void  drawTownFront(const TownState& ts);      // crossing gates, light mist: in front of the characters
float boatBobY(float time);                    // vertical bob of the boat (so Ren can ride it)
float boatDeckY();                             // y of the boat deck at rest
void  drawBoatFront(const TownState& ts);      // boat rim, drawn after the rider so he stands inside
