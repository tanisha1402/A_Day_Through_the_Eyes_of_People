# Progress

**Current phase:** 2 (Core graphics utilities), awaiting user verification on Windows. Phase 1 environment check also still unconfirmed by the user.
**Date:** 21/11/2026

## Completed (headless-tested, not yet seen in a real window)
- Modules: `Config.h`, `Animation` (central clock), `Utils` (colour maths, primitives, glow, vignette, letterbox, fade, text, scissor clip, viewport/mouse maths), `Environment` (cloud, tree, window, house, hill layer, star field, shooting star), `TestScreen` (2 pages), `main`.
- Headless tests (`tests/`): 25 checks pass (maths, viewport, mouse conversion, scissor, clock) plus 14,400 simulated frames across 4 window sizes and both pages with no GL misuse: balanced Begin/End, matrix stack, scissor and blend state restored, no NaN vertices, colours in range. Max about 4,260 vertices per frame.
- Compiled with `-Wall -Wextra` against stub headers: no warnings.

## Bug found and fixed by tests
- Zero-size (minimised) window gave viewport height 0, which would divide by zero in mouse conversion.

## Not verified
- Build/link with real freeGLUT in Code::Blocks.
- How it looks (colours, proportions, glow strength), and real FPS. The assistant cannot open a window.

## Known limitations
- `drawPolygon` uses GL_POLYGON, so only convex shapes are correct.
- Overlapping translucent cloud puffs would show seams if cloud alpha < 1; use opaque clouds.
- Test screens are temporary; Phase 3 replaces them.

## Decisions
- Logical 1280x720 coordinates; 50 px letterbox; houses, trees and clouds are placed by bottom-centre or centre.
- Clock functions take the time in ms so they can be tested without OpenGL.
- Additive blending for glow, restored to normal alpha blending after each glow.

## Next task
Phase 3, task 1: `TimeOfDay` module (hour keyframes, sky and light colours) and `Camera` module (pan, zoom, smooth moves).
