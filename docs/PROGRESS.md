# Progress

**Current phase:** 4 (Split-screen Scenes 4-6), awaiting user verification on Windows. Phase 1-3 checks on the user's machine are also still unconfirmed.
**Date:** 21/11/2026

## Completed (headless-tested and reviewed in software-rendered previews; not seen in a real window)
- `Viewports` (layouts, beginPanel/endPanel with viewport + scissor + projection, nested clip, picking), `Particles` (pool, RainField), `Panels` (nine panel scenes, lightning), scene manager extended to six chapters, characters gained a seated pose and a typing/playing arm pose.
- 59 automated checks pass, including layout geometry (inside the bars, no overlaps), picking, scissor scaling, pool caps under a 5000/s rain flood, rain draining, lightning range, six-chapter timing and wrap, and about 16,000 simulated frames over 3 window sizes with the GL viewport and scissor verified restored after every frame.
- Test found and fixed a real bug: an office light colour reached 1.05 (outside 0..1).
- Preview images for all six chapters in `docs/previews/`.

## Not verified
- Build and link with real freeGLUT. Real FPS. Real text rendering.

## Known limitations
- Peak about 34,000 vertices per frame (town chapters); the panel chapters are lighter. Display lists are the Phase 7 option if FPS is low.
- Granny's seated pose is simple (skirt flat on the mat); acceptable but could be refined in polish.
- Chapters 7-10 not implemented; the loop wraps after chapter 6.
- R (rain toggle) is not wired yet: RainField already has an `intensity` value for it (Phase 6).
- Thought-bubble hit areas will use `panelAt` in Phase 6.

## Decisions
- Scene 4 story hour is 9:00-10:00 (not 9:00-12:00) so the on-screen clocks stay readable; the sky is nearly identical across 9-12, so there is no visible jump into chapter 5 (12:00).
- Scene 6 uses a 2x2 grid (four lives), scenes 4 and 5 use columns.
- Panels use their own backgrounds, not the world camera; panel content is in panel-local coordinates.
- Scene 5: the cloud has one screen-space position; each panel draws it at (cloudX - panelX), so the viewport clipping alone produces the crossing.
- Both phones in scene 5 ring at the same moments (4.2-10.0 s): the first visual hint that Kenji and Granny are connected.

## Next task
Phase 5, task 1: Scene 7 (After the Rain): rainbow, wet ground and puddles, the cat's journey up to the mansion, Aoi opening her window.
