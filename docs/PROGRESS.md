# Progress

**Current phase:** 1 (Planning and technical foundation), awaiting user verification.
**Date:** 21/11/2026

## Completed
- Proposal analysed; architecture, world layout, module list and phase criteria written (docs/PROJECT_PLAN.md).
- Skeleton `src/main.cpp`: 1280x720 window, centralized 16 ms timer with `dt`, 16:9 letterboxed viewport, gradient, glow, GL_TRIANGLES, GL_LINE_LOOP, bitmap text, pause and speed keys.
- Code::Blocks project file and `.gitignore`.
- Syntax check against stub headers passed.

## Not verified yet
- Build and run on the user's Windows machine (real freeGLUT, real window). Not tested by the assistant.

## Incomplete
Phases 2-7.

## Known bugs
None known; nothing has been run with a real OpenGL window.

## Decisions
- Logical 1280x720 coordinates; 50 px letterbox bars.
- Districts left to right: Hilltop, Downtown, Riverside, Countryside.
- Interiors drawn as cutaway panels in screen space.
- `dt` from `GLUT_ELAPSED_TIME`, clamped.
- Audio limited to optional music (PlaySound plays one WAV at a time).

## Next task
Phase 2, task 1: split `main.cpp` into modules and build the utility test screen.
