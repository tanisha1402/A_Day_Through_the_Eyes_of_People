# Progress

**Current phase:** 5 (Scenes 7-10, story completion), awaiting user verification on Windows. Phase 1-4 checks on the user's machine are also still unconfirmed.
**Date:** 21/11/2026

## Completed (headless-tested and reviewed in software-rendered previews; not seen in a real window)
- All ten chapters implemented and looping (150 s). New: `Fireworks` module, scenes 7, 8, 9, 10, festival/rainbow/puddles/open-window/home-light/gate-lantern in `World`, three Scene 8 panels in `Panels`, `makeMother`, stroke-font text (`drawTextStroke`), growing moon.
- 63 automated checks pass, including: ten-chapter timing and wrap (P from chapter 1 -> chapter 10), fireworks caps and flash range under a launch flood, sky continuity at every chapter boundary, and 18,720 simulated frames (full film at 1280x720, every chapter at 3 awkward window sizes, pause, max speed) with no GL misuse and the viewport restored after every frame.
- Preview images for every chapter in `docs/previews/`.

## Not verified
- Build and link with real freeGLUT. Real FPS. Real text (stroke-font titles and the closing line are drawn as boxes in previews). Whether GLUT_STROKE_ROMAN looks good at the chosen sizes.

## Known limitations
- Performance: the festival is the heaviest frame, about 56,000 vertices (down from 74,000 after cheaper glows). If real FPS is low, Phase 7 options: display lists for static scenery, fewer lantern glow layers, smaller crowd.
- Crowd people in the festival overlap a little; acceptable for a busy crowd.
- Kenji's and Granny's reunion ends at the meeting at the gate (no embrace animation).
- The thought bubbles, R, M, right-click firework and left-click controls are Phase 6.

## Decisions
- Scene 8 uses three golden-hour panels (like Scene 4) so the three homecomings sit side by side; the crow flock has one screen position, drawn in each panel at (x - panelX).
- Scene 10 story hour is 23:30-24:30 (not -28:30) so the moon stays high for the final rise; the fade-out then loops to 4:30 AM.
- Scene 9 fireworks are scripted (same show every loop) and burst in the open sky beside the office towers; `fireworksLaunch` is ready for the right-click control.
- Kenji arrives at Granny's gate from the town side (left); Granny comes out of the farmhouse (right); the cat waits just inside the gate.
- Scene 4's story hour stays 9:00-10:00 (clocks readable); chapter 4 -> 5 is a deliberate time skip with near-identical day sky.

## Next task
Phase 6, task 1: input handling module (keyboard, mouse), then thought bubbles with picking, rain toggle (R), speed controls review, right-click firework / cloud pause, optional audio (M).
