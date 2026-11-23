#pragma once
struct MockStats {
    bool inBegin = false;
    int  matrixDepth = 0;
    bool scissorOn = false;
    unsigned blendS = 0x0302, blendD = 0x0303;
    int  errors = 0;
    long vertices = 0, maxVertices = 0, textCalls = 0;
    char firstError[256] = "";
};
extern MockStats gMock;
void mockFrameCheck(const char* where);   // verifies state is clean at the end of a frame

// Optional software rasterizer so frames can be saved as images (tests/render.cpp).
void mockInitRaster(int width, int height);                  // enables rasterizing
void mockGetViewport(int& x, int& y, int& w, int& h);      // current GL viewport
void mockWritePPM(const char* path, int downscale);          // box-filter downscale by k
