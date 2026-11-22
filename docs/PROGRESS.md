# Progress

**Current phase:** 3 (World, characters, Scenes 1-3), awaiting user verification on Windows. Phase 1 and 2 checks on the user's machine are also still unconfirmed.
**Date:** 21/11/2026

## Completed (headless-tested and reviewed in software-rendered previews; not seen in a real window)
- `TimeOfDay`, `Camera`, `Characters` (6 people + cat), `World` (town, districts, landmarks, train, crossing), `Scenes` (chapter manager, scripts for chapters 1-3, fades, captions, clock), updated `main`.
- 43 automated checks pass: maths, clock, sky continuity over the full looping day, loop closure 4:30 -> 28:30, camera, chapter timing and wrap, N/P, and about 8,000 simulated frames across 3 window sizes, pause and max speed with no GL misuse.
- Preview images in `docs/previews/` show what each chapter should look like.

## Not verified
- Build and link with real freeGLUT in Code::Blocks. Real FPS. Real text rendering (previews draw text as boxes).

## Known limitations
- Up to about 34,000 vertices per frame (immediate mode). Fine on desktop GPUs, but display lists for static scenery are planned for Phase 7 if FPS is low.
- Caption and HUD use GLUT bitmap fonts (small). A larger drawn title style may be added during polish.
- Hunched characters (Ren, Granny) have simple torsos; face detail is minimal by design.
- Chapters 4-10 are not implemented; the loop wraps after chapter 3.
- Particle pool not built yet (Phase 4).

## Decisions
- Districts reordered: Hilltop, Riverside, Downtown, Countryside (see PROJECT_PLAN).
- Aoi's room and Ren's hut are in-world cutaways.
- Character and train positions are pure functions of chapter time (waypoint tracks), so N/P jumps and loops are always consistent.
- At the crossing the train passes behind the waiting people and in front of the buildings; Hana waits left of the gates, Kenji and the neighbour right of them.
- N/P chapter keys were pulled forward from Phase 6 as a testing aid.
- Story hour glides across each chapter (e.g. 4:30 to 6:00 during chapter 1), so each chapter's end sky equals the next chapter's start sky.

## Next task
Phase 4, task 1: `Particles` module (pool with caps) and `Viewports` module (panels, glScissor, picking maths), then the three-panel clock scene.
