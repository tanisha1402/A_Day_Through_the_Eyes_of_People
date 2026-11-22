// Camera.h - a 2D camera: pan (x, y = world point at screen centre) and zoom.
#pragma once

struct Camera { float x, y, zoom; };
extern Camera gCamera;

Camera makeCamera(float x, float y, float zoom);
Camera cameraBlend(const Camera& a, const Camera& b, float t);   // eased (smoothstep) move from a to b
// Starts drawing in world space: screen = centre + (world - camera) * zoom.
// parallax < 1 makes a layer follow the camera more slowly (distant things).
void cameraBegin(float parallax = 1.0f);
void cameraEnd();
