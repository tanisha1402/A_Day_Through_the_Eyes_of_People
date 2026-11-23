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
8. **Expected result (Phase 4):** a 1280x720 window that plays chapters 1-6 on a loop (about 90 s). Chapters 1-3 are the town scenes (pre-dawn, sunrise and the pan down the hill, the railway crossing). Chapter 4 is three side-by-side panels (warm classroom, grey office, cold piano room) with the same clock ticking in each. Chapter 5 is two panels (Kenji on a rooftop, Granny Sato in a field) with one cloud drifting across the border and both phones ringing together. Chapter 6 is a 2x2 grid in a thunderstorm with lightning flashing across all four panels. Black bars top and bottom, a clock top-right, FPS bottom-left.
9. **Behaviour check:** N / P switch chapter, Space pauses, Left/Right change speed, resizing keeps a centred 16:9 picture (panels must stay inside it, with no bleeding into the black bars), Esc closes. Each chapter fades in from black; in chapters 4-6 the panels appear one by one.
10. **Compare with** `docs/previews/*.png` (frames from a software renderer; your GPU output should look very similar, text will be real text).

Common errors: `cannot find -lfreeglut` (steps 4/5), `freeglut.dll was not found` (step 6), `GL/freeglut.h: No such file` (step 4), `skipping incompatible` (step 5 bitness).

## Build and run
Code::Blocks: open the `.cbp`, choose Debug or Release, press F9.
Command line (MinGW): `g++ -std=c++11 -Wall src\*.cpp -o bin\ADay.exe -lfreeglut -lopengl32 -lglu32 -lwinmm`

## Controls (final)
Space pause/resume, N/P next/previous chapter, Left/Right arrow speed down/up, R toggle rain, left click on a character shows a thought bubble, right click launches a firework or pauses clouds, M mute, Esc exit.
Implemented so far: Space, N/P, Left/Right, Esc. The rest arrive in Phase 6.

## Headless tests (optional)
`tests/` holds a fake OpenGL layer (with a small software rasterizer) and checks that run the real drawing code without a window: colour, viewport and mouse maths, the clock, sky continuity, camera, chapter timing, and thousands of simulated frames checked for GL misuse. Run `tests\run_tests.bat` from the project folder. They do not replace looking at the real window.

## Status
See `docs/PROGRESS.md`.
