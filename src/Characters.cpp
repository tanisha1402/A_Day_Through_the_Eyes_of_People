#include "Characters.h"
#include <GL/freeglut.h>

static const float D2R = Config::PI / 180.0f;

PersonPose makePose(float facing, float time) {
    PersonPose p; p.walkPhase = 0; p.walkAmount = 0; p.facing = facing; p.time = time; p.armUp = 0; p.sit = 0; p.armsFwd = 0; p.propAlways = false; return p;
}

// ------------------------------------------------------------------ styles ----
static PersonStyle baseStyle() {
    PersonStyle s;
    s.skin = rgb(0.98f, 0.82f, 0.70f); s.hair = rgb(0.12f, 0.10f, 0.12f);
    s.top = rgb(0.5f, 0.5f, 0.5f); s.bottom = rgb(0.3f, 0.3f, 0.35f); s.accent = rgb(0.8f, 0.2f, 0.2f);
    s.shoe = rgb(0.15f, 0.12f, 0.12f); s.hairStyle = HAIR_SHORT;
    s.skirt = s.suit = s.uniform = s.apron = s.hat = s.isGranny = false;
    s.prop = PROP_NONE; s.height = 1.0f; s.hunch = 0; s.bodyWidth = 24;
    return s;
}
PersonStyle makeRen() {
    PersonStyle s = baseStyle();
    s.skin = rgb(0.82f, 0.62f, 0.48f); s.hair = rgb(0.88f, 0.88f, 0.9f);
    s.top = rgb(0.18f, 0.36f, 0.42f); s.bottom = rgb(0.30f, 0.26f, 0.22f); s.accent = rgb(0.85f, 0.7f, 0.3f);
    s.shoe = rgb(0.22f, 0.16f, 0.1f); s.hairStyle = HAIR_OLD_MAN; s.hat = true; s.height = 0.93f; s.hunch = 11; s.bodyWidth = 26;
    return s;
}
PersonStyle makeAoi() {
    PersonStyle s = baseStyle();
    s.hair = rgb(0.10f, 0.08f, 0.16f); s.top = rgb(0.80f, 0.80f, 0.92f); s.bottom = rgb(0.72f, 0.72f, 0.88f);
    s.accent = rgb(0.55f, 0.55f, 0.80f); s.shoe = rgb(0.85f, 0.85f, 0.9f);
    s.hairStyle = HAIR_LONG; s.skirt = true; s.height = 0.97f; s.bodyWidth = 21;
    return s;
}
PersonStyle makeHana() {
    PersonStyle s = baseStyle();
    s.hair = rgb(0.30f, 0.17f, 0.10f); s.top = rgb(0.97f, 0.97f, 1.0f); s.bottom = rgb(0.16f, 0.22f, 0.45f);
    s.accent = rgb(0.90f, 0.22f, 0.28f); s.shoe = rgb(0.35f, 0.22f, 0.15f);
    s.hairStyle = HAIR_TAILS; s.skirt = true; s.uniform = true; s.prop = PROP_BACKPACK; s.height = 0.88f; s.bodyWidth = 21;
    return s;
}
PersonStyle makeKenji() {
    PersonStyle s = baseStyle();
    s.hair = rgb(0.10f, 0.09f, 0.10f); s.top = rgb(0.20f, 0.22f, 0.32f); s.bottom = rgb(0.17f, 0.18f, 0.26f);
    s.accent = rgb(0.75f, 0.15f, 0.2f); s.hairStyle = HAIR_SHORT; s.suit = true; s.prop = PROP_BRIEFCASE; s.height = 1.06f; s.bodyWidth = 27;
    return s;
}
PersonStyle makeGranny() {
    PersonStyle s = baseStyle();
    s.skin = rgb(0.86f, 0.66f, 0.52f); s.hair = rgb(0.85f, 0.85f, 0.88f);
    s.top = rgb(0.58f, 0.40f, 0.30f); s.bottom = rgb(0.40f, 0.30f, 0.34f); s.accent = rgb(0.95f, 0.92f, 0.80f);
    s.shoe = rgb(0.25f, 0.2f, 0.15f); s.hairStyle = HAIR_BUN; s.skirt = true; s.apron = true; s.hat = true; s.isGranny = true;
    s.height = 0.86f; s.hunch = 14; s.bodyWidth = 26;
    return s;
}
PersonStyle makeNeighbor() {
    PersonStyle s = baseStyle();
    s.hair = rgb(0.35f, 0.2f, 0.12f); s.top = rgb(0.58f, 0.40f, 0.62f); s.bottom = rgb(0.34f, 0.40f, 0.45f);
    s.accent = rgb(0.95f, 0.85f, 0.6f); s.hairStyle = HAIR_BOB; s.skirt = true; s.prop = PROP_BAG; s.height = 0.95f; s.bodyWidth = 25;
    return s;
}

PersonStyle makeMother() {
    PersonStyle s = baseStyle();
    s.hair = rgb(0.10f, 0.08f, 0.09f); s.top = rgb(0.22f, 0.30f, 0.40f); s.bottom = rgb(0.26f, 0.26f, 0.32f);
    s.accent = rgb(0.9f, 0.9f, 0.9f); s.hairStyle = HAIR_BUN; s.skirt = true; s.prop = PROP_BAG; s.height = 0.98f; s.hunch = 7; s.bodyWidth = 25;
    return s;
}

// ---------------------------------------------------------------- helpers ----
static Color ol(Color c) { return darken(c, 0.38f); }            // outline colour from a fill colour

// Limb with its own pivot: draws (via the callback-free pattern) rect hanging down from the pivot.
static void limb(float w, float len, Color c) { drawRect(-w / 2, -len, w, len, c); }

static void drawHeadAndHair(float hy, const PersonStyle& st, const PersonPose& pose) {
    const float R = 14.5f;
    Color hair = st.hair, hairShade = darken(hair, 0.7f);

    // ---- hair behind the head ----
    switch (st.hairStyle) {
    case HAIR_LONG: {
        float p[] = { -13,hy+6, -17,hy-10, -19,hy-34, -16,hy-52, -3,hy-50, 0,hy-20, 6,hy+4 };
        drawPolygon(p, 7, hairShade);
        drawCircle(-3, hy + 2, R + 1.5f, hair);
    } break;
    case HAIR_TAILS:
        drawCircle(-2, hy + 2, R + 1.5f, hair);
        { float p[] = { -12,hy+1, -22,hy-8, -26,hy-28, -18,hy-30, -12,hy-10 };
          glPushMatrix(); glTranslatef(0, 0, 0); drawPolygon(p, 5, hairShade); glPopMatrix(); }
        drawCircle(-14, hy + 1, 3.0f, st.accent);                              // ribbon
        break;
    case HAIR_BUN:
        drawCircle(-9, hy + 15, 7.5f, hair); drawCircle(-9, hy + 15, 7.5f, hair);
        drawCircle(-2, hy + 2, R + 1.2f, hair);
        break;
    case HAIR_BOB:
        { float p[] = { -15,hy+8, -17,hy-8, -9,hy-12, 4,hy-6, 6,hy+8 }; drawPolygon(p, 5, hairShade); }
        drawCircle(-2, hy + 2, R + 1.4f, hair);
        break;
    case HAIR_SHORT:  drawCircle(-2, hy + 2, R + 1.0f, hair); break;
    case HAIR_OLD_MAN: drawCircle(-3, hy + 1, R + 0.5f, hair); break;
    }

    // ---- head ----
    drawCircle(0, hy, R, st.skin);
    drawCircleOutline(0, hy, R, ol(st.skin), 1.2f);
    drawCircle(-2.5f, hy - 1, 3.0f, darken(st.skin, 0.93f));                   // ear

    // ---- face (looking toward +x) ----
    bool blink = std::fmod(pose.time * 0.9f + hy, 4.3f) > 4.15f;
    if (blink) drawLine(3.5f, hy + 1, 8.5f, hy + 1, rgb(0.15f, 0.1f, 0.1f), 1.5f);
    else {
        drawEllipse(6.2f, hy + 1.0f, 2.4f, 3.6f, rgb(0.14f, 0.10f, 0.12f));
        drawCircle(6.9f, hy + 2.3f, 1.0f, rgb(1, 1, 1));                      // eye highlight
    }
    drawEllipse(9.0f, hy - 4.2f, 3.0f, 1.8f, rgba(1.0f, 0.45f, 0.45f, 0.35f)); // blush
    drawLine(7.0f, hy - 7.0f, 10.0f, hy - 6.4f, rgb(0.55f, 0.25f, 0.25f), 1.2f); // mouth
    if (st.hairStyle == HAIR_OLD_MAN || st.isGranny)                            // brow lines: age
        drawLine(3.5f, hy + 6.5f, 9.0f, hy + 5.5f, darken(st.hair, 0.8f), 1.3f);

    // ---- fringe / front hair ----
    switch (st.hairStyle) {
    case HAIR_SHORT: case HAIR_OLD_MAN: {
        float f[] = { -14,hy+3, -9,hy+14, -1,hy+16.5f, 8,hy+14, 13.5f,hy+5, 9,hy+9, 4,hy+5, -1,hy+10, -7,hy+5 };
        drawPolygon(f, 9, hair); } break;
    case HAIR_LONG: case HAIR_TAILS: case HAIR_BOB: case HAIR_BUN: {
        float f[] = { -15,hy+2, -11,hy+13, -2,hy+16.5f, 8,hy+14, 14,hy+4, 10,hy+8, 6,hy+3.5f, 2,hy+9, -4,hy+4, -9,hy+8 };
        drawPolygon(f, 10, hair); } break;
    }
    if (st.hat) {
        if (st.isGranny) {                                                     // straw hat
            drawEllipse(0, hy + 11, 24, 5.5f, rgb(0.86f, 0.74f, 0.44f));
            drawEllipse(0, hy + 14, 14, 8, rgb(0.82f, 0.70f, 0.40f));
            drawRect(-14, hy + 12, 28, 3, st.apron ? rgb(0.7f, 0.25f, 0.25f) : st.accent);
        } else {                                                               // fisherman's knit cap
            drawEllipse(-1.5f, hy + 11, 15.0f, 7.5f, rgb(0.15f, 0.18f, 0.25f));   // above the eyes
            drawRect(-14.5f, hy + 8.5f, 27, 3.2f, rgb(0.35f, 0.38f, 0.5f));
        }
    }
}

// ------------------------------------------------------------------ person ----
void drawPerson(float x, float y, float scale, const PersonStyle& st, const PersonPose& pose) {
    const float legLen = 36.0f * st.height, torsoH = 33.0f * st.height;
    const bool sit = pose.sit > 0.5f;
    const float hipY = sit ? legLen * 0.5f + 1.0f : legLen + 1.0f, bw = st.bodyWidth;
    float swing = std::sin(pose.walkPhase);
    float legAng = swing * 30.0f * pose.walkAmount;
    float armAng = -swing * 28.0f * pose.walkAmount;
    float bounce = std::fabs(std::sin(pose.walkPhase)) * 2.2f * pose.walkAmount          // step bounce
                 + std::sin(pose.time * 2.0f) * 0.7f * (1.0f - pose.walkAmount);          // breathing

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(pose.facing * scale, scale, 1);                                              // flip to face left
    glTranslatef(0, bounce, 0);

    // ground shadow
    drawEllipse(0, -bounce + 1, 20, 4, rgba(0, 0, 0, 0.22f), 20);

    // ---- legs (pivot at the hip) ----
    for (int side = 0; side < 2; ++side) {
        float ang = side == 0 ? -legAng : legAng;              // back leg first, front leg last
        Color legCol = st.skirt ? (st.uniform ? rgb(0.97f, 0.9f, 0.85f) : st.skin) : st.bottom;
        if (side == 0) legCol = darken(legCol, 0.82f);
        glPushMatrix();
        glTranslatef(0, hipY, 0);
        if (sit) {                                   // thigh points forward, shin hangs down from the knee
            float th = legLen * 0.5f * (side == 0 ? 0.92f : 1.0f);
            glRotatef(90, 0, 0, 1);
            limb(8.5f, th, legCol);
            glTranslatef(0, -th, 0);
            glRotatef(-90, 0, 0, 1);
            limb(8.0f, legLen * 0.5f, legCol);
            drawEllipse(3.5f, -legLen * 0.5f + 2.0f, 7.5f, 3.5f, side == 0 ? darken(st.shoe, 0.8f) : st.shoe);
        } else {
            glRotatef(ang, 0, 0, 1);
            limb(8.5f, legLen, legCol);
            drawEllipse(3.5f, -legLen + 2.0f, 7.5f, 3.5f, side == 0 ? darken(st.shoe, 0.8f) : st.shoe);
        }
        glPopMatrix();
    }

    // ---- upper body, leaning forward by 'hunch' degrees around the hip ----
    glPushMatrix();
    glTranslatef(0, hipY, 0);
    glRotatef(-st.hunch, 0, 0, 1);

    if (st.prop == PROP_BACKPACK) {
        drawRect(-bw * 0.95f, 2, bw * 0.55f, torsoH - 4, rgb(0.85f, 0.45f, 0.30f));
        drawRectOutline(-bw * 0.95f, 2, bw * 0.55f, torsoH - 4, ol(rgb(0.85f, 0.45f, 0.30f)), 1.2f);
    }
    // back arm
    glPushMatrix();
    glTranslatef(-1, torsoH - 4, 0);
    glRotatef(-armAng * 0.9f + 4 + pose.armsFwd * 50.0f, 0, 0, 1);
    limb(7, 15, darken(st.top, 0.8f));
    glTranslatef(0, -15, 0); glRotatef(6 + pose.armsFwd * 55.0f, 0, 0, 1);
    limb(6, 14, darken(st.skin, 0.9f));
    glPopMatrix();

    // torso
    float torso[] = { -bw * 0.42f, 0, bw * 0.42f, 0, bw * 0.52f, torsoH, -bw * 0.52f, torsoH };
    Color topCol = st.top;
    drawPolygon(torso, 4, topCol);
    drawPolygonOutline(torso, 4, ol(topCol), 1.3f);
    if (st.apron) { float a[] = { -bw * 0.3f, 0, bw * 0.38f, 0, bw * 0.38f, torsoH * 0.8f, -bw * 0.3f, torsoH * 0.8f }; drawPolygon(a, 4, st.accent); }
    if (st.suit) {
        float shirt[] = { -3, torsoH, 8, torsoH, 4, torsoH * 0.62f }; drawPolygon(shirt, 3, rgb(0.96f, 0.96f, 0.98f));
        float tie[] = { 3, torsoH - 1, 6, torsoH - 2, 7.5f, torsoH * 0.35f, 4.5f, torsoH * 0.28f }; drawPolygon(tie, 4, st.accent);
    }
    if (st.uniform) {
        float col[] = { -bw * 0.5f, torsoH, bw * 0.5f, torsoH, 2, torsoH * 0.62f }; drawPolygon(col, 3, st.bottom);   // sailor collar
        float rib[] = { 0, torsoH * 0.66f, 7, torsoH * 0.75f, 7, torsoH * 0.5f }; drawPolygon(rib, 3, st.accent);
    }
    // skirt / dress
    if (st.skirt) {
        float flare = st.isGranny ? 1.55f : (st.uniform ? 1.5f : 1.7f);
        float drop  = st.isGranny ? -22.0f : (st.uniform ? -17.0f : -26.0f);
        float sk[] = { -bw * 0.45f, 2, bw * 0.45f, 2, bw * 0.45f * flare, drop, -bw * 0.45f * flare, drop };
        drawPolygon(sk, 4, st.bottom);
        drawPolygonOutline(sk, 4, ol(st.bottom), 1.2f);
        if (st.uniform) drawLine(-bw * 0.45f * flare, drop + 3, bw * 0.45f * flare, drop + 3, rgb(0.9f, 0.9f, 0.95f), 1.3f);
    }
    // neck + head
    drawRect(-3, torsoH - 2, 6.5f, 7, darken(st.skin, 0.95f));
    drawHeadAndHair(torsoH + 18.0f * 1.0f, st, pose);

    // front arm (with elbow) and prop in hand
    glPushMatrix();
    glTranslatef(1, torsoH - 4, 0);
    float upper = armAng * 0.9f - 4 + pose.armUp * 70.0f + pose.armsFwd * 55.0f;
    glRotatef(upper, 0, 0, 1);
    limb(7.5f, 15, st.top);
    glTranslatef(0, -15, 0);
    glRotatef(10 + pose.armUp * 85.0f + pose.armsFwd * 50.0f, 0, 0, 1);
    limb(6, 14, st.skin);
    glTranslatef(0, -14, 0);
    drawCircle(0, 0, 3.6f, st.skin);
    if (st.prop == PROP_BRIEFCASE && (pose.armUp < 0.3f || pose.propAlways)) {
        drawRect(-9, -22, 18, 15, rgb(0.28f, 0.2f, 0.14f));
        drawRectOutline(-9, -22, 18, 15, rgb(0.1f, 0.07f, 0.05f), 1.2f);
        drawRect(-3, -8, 6, 3, rgb(0.7f, 0.6f, 0.2f));
    }
    if (st.prop == PROP_BAG && pose.armUp < 0.3f) {
        drawRect(-8, -26, 16, 20, rgb(0.9f, 0.85f, 0.7f));
        drawRectOutline(-8, -26, 16, 20, rgb(0.5f, 0.4f, 0.3f), 1.2f);
        drawCircle(0, -6, 4, rgb(0.5f, 0.75f, 0.35f));
    }
    glPopMatrix();

    glPopMatrix();   // upper body
    glPopMatrix();   // person
}

// --------------------------------------------------------------------- cat ----
void drawCat(float x, float y, float scale, float facing, float walkPhase, float walkAmount, float time, bool sitting) {
    Color fur = rgb(0.86f, 0.58f, 0.32f), stripe = rgb(0.62f, 0.36f, 0.18f), belly = rgb(0.98f, 0.90f, 0.78f);
    Color line = rgb(0.30f, 0.16f, 0.08f);
    float tailSway = std::sin(time * 2.2f) * 12.0f;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(facing * scale, scale, 1);
    drawEllipse(0, 0.5f, sitting ? 13 : 24, 3.5f, rgba(0, 0, 0, 0.22f), 20);

    if (sitting) {
        // tail curled around the front
        glPushMatrix(); glTranslatef(-8, 3, 0); glRotatef(tailSway * 0.4f, 0, 0, 1);
        drawEllipse(10, 2.5f, 15, 3.5f, stripe, 16); glPopMatrix();
        drawEllipse(0, 15, 11.5f, 16.5f, fur);                      // upright body
        drawEllipse(5.5f, 12, 5.5f, 11, belly);
        drawEllipse(5, 2.5f, 5, 2.4f, belly); drawEllipse(-4, 2.5f, 5, 2.4f, belly);   // paws
        for (int i = 0; i < 3; ++i) drawLine(-10.5f, 12 + i * 5.0f, -4.5f, 12 + i * 5.0f, stripe, 1.5f);
        glTranslatef(5, 34, 0);
    } else {
        float s = std::sin(walkPhase) * 22.0f * walkAmount;
        glPushMatrix(); glTranslatef(-20, 20, 0); glRotatef(35 + tailSway, 0, 0, 1);
        float tail[] = { -2,0, 4,0, 5,22, 0,34, -5,32, -3,18 }; drawPolygon(tail, 6, stripe); glPopMatrix();
        for (int i = 0; i < 4; ++i) {                                // four legs, diagonal pairs swing together
            float lx = (i < 2 ? 13.0f : -13.0f) + (i % 2) * 3.0f;
            float ang = ((i == 0 || i == 3) ? s : -s);
            glPushMatrix(); glTranslatef(lx, 14, 0); glRotatef(ang, 0, 0, 1);
            drawRect(-2.5f, -14, 5, 14, i % 2 ? darken(fur, 0.85f) : fur);
            drawEllipse(1, -14, 4.5f, 2.2f, belly); glPopMatrix();
        }
        float bob = std::fabs(std::sin(walkPhase)) * 1.2f * walkAmount;
        drawEllipse(0, 20 + bob, 23, 10.5f, fur);                    // body
        drawEllipse(2, 15 + bob, 17, 5, belly);
        for (int i = 0; i < 4; ++i) drawLine(-12.0f + i * 6.0f, 25 + bob, -10.0f + i * 6.0f, 18 + bob, stripe, 1.6f);
        glTranslatef(21, 27 + bob, 0);
    }
    // head (origin is now at the head centre)
    float ears1[] = { -8, 5, -7, 15, 0, 8 }, ears2[] = { 1, 8, 8, 15, 8, 4 };
    drawPolygon(ears1, 3, fur); drawPolygon(ears2, 3, fur);
    float in1[] = { -6.5f, 6, -6.3f, 12, -2, 8 }, in2[] = { 2.5f, 8, 6.5f, 12, 6.5f, 5 };
    drawPolygon(in1, 3, rgb(0.95f, 0.65f, 0.6f)); drawPolygon(in2, 3, rgb(0.95f, 0.65f, 0.6f));
    drawCircle(0, 0, 9.5f, fur);
    drawCircleOutline(0, 0, 9.5f, line, 1.0f);
    drawEllipse(5.5f, -3, 4.5f, 3.3f, belly);                        // muzzle
    drawLine(-3, 8, -3, 5, stripe, 1.4f); drawLine(1, 9, 1, 6, stripe, 1.4f);
    bool blink = std::fmod(time * 0.7f + 1.0f, 5.0f) > 4.85f;
    if (blink) drawLine(2.5f, 1.5f, 7.0f, 1.5f, line, 1.4f);
    else {
        drawEllipse(4.8f, 1.8f, 2.7f, 3.0f, rgb(0.75f, 0.9f, 0.3f));
        drawEllipse(5.2f, 1.8f, 0.9f, 2.4f, rgb(0.08f, 0.08f, 0.08f));
    }
    drawCircle(9.6f, -1.5f, 1.2f, rgb(0.9f, 0.45f, 0.5f));          // nose
    drawLine(7, -3.5f, 14, -2.0f, rgba(1, 1, 1, 0.8f), 1.0f); drawLine(7, -4.5f, 14, -5.5f, rgba(1, 1, 1, 0.8f), 1.0f);   // whiskers
    glPopMatrix();
}
