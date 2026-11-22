#include "Camera.h"
#include "Utils.h"
#include <GL/freeglut.h>

Camera gCamera = { 640.0f, 360.0f, 1.0f };

Camera makeCamera(float x, float y, float zoom) { Camera c; c.x = x; c.y = y; c.zoom = zoom; return c; }

Camera cameraBlend(const Camera& a, const Camera& b, float t) {
    float e = smoothstep01(t);
    return makeCamera(lerp(a.x, b.x, e), lerp(a.y, b.y, e), lerp(a.zoom, b.zoom, e));
}

void cameraBegin(float parallax) {
    glPushMatrix();
    glTranslatef(Config::WINDOW_W / 2.0f, Config::WINDOW_H / 2.0f, 0);   // 3. put the camera point mid-screen
    glScalef(gCamera.zoom, gCamera.zoom, 1);                               // 2. zoom
    glTranslatef(-gCamera.x * parallax, -gCamera.y * parallax, 0);         // 1. move the world so the camera point is at the origin
}
void cameraEnd() { glPopMatrix(); }
