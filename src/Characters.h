// Characters.h - reusable people and cat. Every limb is drawn inside its parent's
// glPushMatrix/glRotatef, so arms and legs swing from the shoulder / hip.
#pragma once
#include "Utils.h"

enum HairStyle { HAIR_SHORT, HAIR_LONG, HAIR_TAILS, HAIR_BUN, HAIR_BOB, HAIR_OLD_MAN };
enum PropType  { PROP_NONE, PROP_BRIEFCASE, PROP_BACKPACK, PROP_BAG };

struct PersonStyle {
    Color skin, hair, top, bottom, accent, shoe;
    HairStyle hairStyle;
    bool  skirt;          // draws a skirt/dress over the legs
    bool  suit;           // jacket + shirt + tie
    bool  uniform;        // sailor collar + ribbon
    bool  apron;
    bool  hat;            // 0 none; Ren: knit cap, Granny: straw hat (see isGranny)
    bool  isGranny;
    PropType prop;
    float height;         // 1.0 = normal adult
    float hunch;          // degrees the upper body leans forward
    float bodyWidth;
};

struct PersonPose {
    float walkPhase;      // radians; advances with distance walked
    float walkAmount;     // 0 = standing (idle breathing), 1 = full walking swing
    float facing;         // +1 faces right, -1 faces left
    float time;           // animation seconds (breathing, blinking)
    float armUp;          // 0..1 raises the front arm (checking a watch, waving)
};
PersonPose makePose(float facing, float time);

PersonStyle makeRen();       // old fisherman
PersonStyle makeAoi();       // girl in the mansion
PersonStyle makeHana();      // schoolgirl
PersonStyle makeKenji();     // office worker
PersonStyle makeGranny();    // Granny Sato
PersonStyle makeNeighbor();  // woman at the crossing

// (x, y) = point between the feet on the ground. scale 1.0 is about 118 units tall.
void drawPerson(float x, float y, float scale, const PersonStyle& st, const PersonPose& pose);

// Stray cat. sitting = true draws it seated, otherwise it walks/trots.
void drawCat(float x, float y, float scale, float facing, float walkPhase, float walkAmount, float time, bool sitting);
