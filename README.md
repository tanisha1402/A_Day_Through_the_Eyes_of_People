# A Day Through the Eyes of People

An OpenGL-based 2D animated story of five lives under one sky.
C++ / OpenGL / freeGLUT. Course: Computer Graphics Lab. Repository: https://github.com/tanisha1402/A_Day_Through_the_Eyes_of_People

One day (4:30 AM to 11:30 PM) in a hillside town, told in ten chapters of about 15 seconds each (about 2.5 minutes per loop). The sun, the train, the rain, a cloud and the moon touch five people differently: Ren (fisherman), Aoi (mansion girl), Hana (schoolgirl), Kenji (office worker) and Granny Sato (farmer). A stray cat connects them. Everything is drawn from OpenGL primitives, with no textures or models.

## Technologies
C++11, OpenGL 1.x (fixed pipeline), freeGLUT, Code::Blocks, MinGW, Windows (optional audio via winmm).

## Environment verification checklist (Windows)
Tick each item before building.

1. **Code::Blocks with MinGW installed.** Use the `codeblocks-XX.XXmingw-setup.exe` installer from codeblocks.org (it bundles the compiler).
2. **Compiler works.** Open Command Prompt, add `...\CodeBlocks\MinGW\bin` to PATH if needed, then run:
   - `g++ --version` (prints a version)
   - `g++ -dumpmachine` (prints `x86_64-w64-mingw32` for 64-bit or `i686-w64-mingw32` for 32-bit). **Write this down.**
3. **freeGLUT for MinGW downloaded** (for example "freeglut 3.0.0 MinGW package" from Martin Payne, or from the freeglut site mirrors).
4. **Headers copied.** `freeglut` package `include\GL\*.h` goes to `<MinGW>\include\GL\`. Check that `<MinGW>\include\GL\freeglut.h` exists.
5. **Import library copied, matching your compiler's bitness.** 32-bit: `lib\libfreeglut.a` to `<MinGW>\lib\`. 64-bit: `lib\x64\libfreeglut.a` to `<MinGW>\lib\`. A mismatch gives "skipping incompatible" linker errors.
6. **DLL placed.** 32-bit: `bin\freeglut.dll`. 64-bit: `bin\x64\freeglut.dll`. Copy it next to the built `.exe` (in `bin\Debug` and `bin\Release`).
7. **Open `ADayThroughTheEyesOfPeople.cbp` in Code::Blocks** and press Build and Run (F9).
8. **Expected result (Phase 2):** a 1280x720 window showing a 4x2 grid of test cells (rectangles and gradients, circles and a rainbow arc, polygons and lines, glow and windows, a day/night sky cycle, translucent circles, a swinging arm with a pulsing house, stars with a shooting star). Black bars at the top and bottom, FPS near 60. A console window prints your GL vendor, renderer and version.
9. **Behaviour check:** T (or Tab) switches to page 2, a composed mini-scene with parallax hills, drifting clouds, houses and trees and a day/night cycle (24 s). Space pauses, Left/Right change speed, resizing keeps a centred 16:9 picture, Esc closes the program.

Common errors: `cannot find -lfreeglut` (steps 4/5), `freeglut.dll was not found` (step 6), `GL/freeglut.h: No such file` (step 4), `skipping incompatible` (step 5 bitness).

## Build and run
Code::Blocks: open the `.cbp`, choose Debug or Release, press F9.
Command line (MinGW): `g++ -std=c++11 -Wall src\*.cpp -o bin\ADay.exe -lfreeglut -lopengl32 -lglu32 -lwinmm`

## Controls (final)
Space pause/resume, N/P next/previous chapter, Left/Right arrow speed down/up, R toggle rain, left click on a character shows a thought bubble, right click launches a firework or pauses clouds, M mute, Esc exit.
Phase 2 implements Space, Left/Right, Esc and T (test page switch, removed in Phase 3).

## Headless tests (optional)
`tests/` holds a fake OpenGL layer and checks that run the real drawing code without a window (colour/viewport/mouse maths, the clock, and 14,400 simulated frames checked for GL misuse). Run from the project folder:
`g++ -std=c++11 -Itests/stub src/Utils.cpp src/Environment.cpp src/Animation.cpp src/TestScreen.cpp tests/mock_gl.cpp tests/test_all.cpp -o test_all.exe && test_all.exe`
These do not replace looking at the real window.

## Status
See `docs/PROGRESS.md`.
