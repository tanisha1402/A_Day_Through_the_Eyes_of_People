# Project Plan and Technical Architecture

Legend: [ ] not started, [x] completed and verified, [!] blocked or at risk.
Source of truth: the submitted project proposal.

## 1. Fixed decisions
- Window 1280x720 (16:9). Logical coordinates 1280x720, origin bottom-left, set with `glOrtho(0,1280,0,720,-1,1)`. On resize a centred 16:9 viewport is kept.
- Letterbox bars 50 px top and bottom, drawn last. Keep important action between y = 50 and y = 670.
- One timer (16 ms) computes `dt` from `glutGet(GLUT_ELAPSED_TIME)`, clamped to 0.05 s. All motion uses `dt * speed`.
- Fixed-function OpenGL, immediate mode, `GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE`.
- No textures, no models, no extra libraries.

## 2. World layout
One panorama, 5200 x 1100 world units (1 unit = 1 pixel at zoom 1). Camera shows 1280x720 at zoom 1, and zoom 0.56 shows about 2290 units wide.

| District | x range | Contents |
|---|---|---|
| Hilltop | 0 - 1300 | Aoi's mansion on the high ground, gate, path down the hill |
| Riverside | 1300 - 2500 | River at the foot of the hill, dock, boat, Ren's hut, small houses |
| Downtown | 2500 - 4000 | Bakery, offices, school with clock, railway track and crossing |
| Countryside | 4000 - 5200 | Rice paddies, Granny Sato's farmhouse and gate |

(Changed in Phase 3 from Hilltop / Downtown / Riverside / Countryside: the river now sits at the foot of the hill so Scene 2's pan down the hill reaches Ren's hut, and Scene 7's cat walk from the riverside up to the mansion is short and natural.)

Layout is adjustable in Phase 3 if composition requires it. Interiors: Aoi's room and Ren's hut are in-world cutaways (open front walls) so the camera can pan and zoom into them. The classroom, office and piano room of Scene 4 are separate split-screen panels (Phase 4).

## 3. Modules (files under `src/`)
| File | Responsibility |
|---|---|
| `main.cpp` | window setup, callback registration, main timer |
| `Config.h` | all constants (sizes, durations, world bounds, colours of key lights) |
| `Utils.h/.cpp` | `Color`, `lerpColor`, `drawCircle`, `drawRect`, `drawGradientRect`, `drawGlow`, outlines, text |
| `Environment.h/.cpp` | `drawCloud`, `drawTree`, `drawHouse`, hills, river, rails, train, lanterns, rainbow, stars |
| `Characters.h/.cpp` | `drawPerson(x,y,scale,colors,walkPhase)`, `drawCat`, thought bubbles |
| `Particles.h/.cpp` | `Particle {x,y,vx,vy,life,color}`, capped pool, rain, petals, fireflies, fireworks |
| `TimeOfDay.h/.cpp` | hour keyframes, sky top/bottom, ambient light, window-light thresholds |
| `Camera.h/.cpp` | position, zoom, smooth move between keyframes, `applyCamera()` using `glTranslatef`/`glScalef` |
| `Scenes.h/.cpp` | `SceneManager`, 10 `updateSceneN(dt)` and `drawSceneN()` functions, transitions, fade overlay |
| `Viewports.h/.cpp` | panel layout for scenes 4-6, `glViewport` + `glScissor`, screen-to-panel-to-world conversion |
| `Input.h/.cpp` | keyboard, special keys, mouse, picking |
| `Audio.h/.cpp` | optional `PlaySound` wrapper, mute flag |

Beginner-friendly rule: plain functions and structs, no inheritance or templates. Scenes are a table of `SceneInfo` plus one `switch` on `sceneIndex`.

## 4. Core data
- `SceneInfo { title, caption, startHour, endHour, duration }`
- Global story clock: `sceneIndex`, `sceneTime` (seconds), `timeOfDay` (hours).
- `timeOfDay = lerp(hour[i], hour[i+1], sceneTime / duration)`; hours are 4.5, 6, 7.5, 9, 12, 15, 16.5, 18, 20, 23.5, then 28.5 (4:30 AM next day) so the loop blends back to Scene 1.
- Sky colours are keyframed per hour and blended with `lerpColor`.
- `Particle` pool capped (rain 400, petals 80, fireflies 60, fireworks 300) and reused.

## 5. Special systems
- **Shared cloud (Scene 5):** one world-space x position, drawn in both panels with each panel's offset; scissor clips at the border.
- **Picking:** window pixel to logical coordinates (undo letterbox viewport and flip y), then panel (scenes 4-6), then world (undo camera).
- **Frame-rate independence:** all updates take `dt`.
- **Performance:** capped particles, static scenery optionally in display lists, no per-frame allocation.
- **Audio:** `PlaySound` supports one WAV at a time, so music only, optional, never blocks graphics.

## 6. Phases
### Phase 1: Planning and technical foundation
- [x] Analyse proposal, scenes and challenges
- [x] Define architecture and world layout (this document)
- [x] README, PROGRESS, CLAUDE.md created
- [x] Skeleton `main.cpp` and Code::Blocks project written; syntax-checked against stub headers
- [ ] User verifies environment checklist and runs skeleton on Windows
- [ ] User approves plan
Acceptance: plan approved; skeleton builds and runs on the user's machine with the expected window.

### Phase 2: Environment and core graphics utilities
- [x] Modules split: Config, Animation (central clock), Utils, Environment, TestScreen, main (headless-tested)
- [x] Primitives: rect, outline, gradient V/H, triangle, polygon, circle, ellipse, arc band, line (headless-tested)
- [x] Glow (additive), translucency, vignette, letterbox, fade overlay (headless-tested; look unverified)
- [x] drawCloud, drawTree, drawWindow, drawHouse, drawHillLayer, drawStarField, drawShootingStar (headless-tested; look unverified)
- [x] Coordinate system: logical 1280x720, letterboxed viewport, window-to-logical mouse maths, scissor clipping helpers (unit-tested)
- [x] Central clock with dt, speed, pause, FPS (unit-tested)
- [ ] User builds in Code::Blocks and checks both test pages visually on Windows
- [ ] Visual polish pass on object look (colours, proportions) after user feedback
Acceptance: both test pages display correctly; ~60 fps; clean build with no warnings in Code::Blocks.
Depends on: Phase 1.

### Phase 3: World, characters, Scenes 1-3
- [x] TimeOfDay: 11 hour keyframes, sky/light blending, sun and moon arcs, window schedule (unit-tested)
- [x] Camera: pan, zoom, eased moves, parallax layers (unit-tested)
- [x] Town panorama with four districts, terrain, river, hut, mansion with room cutaway, bakery, offices, school, farmhouse, track, train, crossing (rendered and reviewed in preview images)
- [x] drawPerson: six character styles, walk cycle, idle breathing, blinking, watch-check arm; drawCat: walking and sitting (reviewed in preview images)
- [x] Scene manager: chapter clock, fade transitions, captions, corner clock; Scenes 1, 2, 3 scripted (headless-tested for timing and GL correctness)
- [ ] User builds and watches chapters 1-3 on Windows and confirms visuals, timing and FPS
- [ ] Particle pool: deferred to Phase 4 (arrives with rain). Chapters 1-3 smoke, steam and mist use simple formulas.
Acceptance: chapters 1-3 play and transition smoothly; characters distinct; walk cycle correct; train and crossing lights in sync.
Depends on: Phase 2.

### Phase 4: Split-screen Scenes 4-6
- [x] Viewports module: 2/3-column and 2x2 layouts, per-panel glViewport + glScissor + local projection, nested panel clip, picking maths (unit-tested)
- [x] Particles module: capped pool, deterministic random, RainField (drops, ripples) (unit-tested, caps stress-tested)
- [x] Scene 4 Same Clock: classroom (warm), office (grey), piano room (cold blue); same ticking clock in all three; dust motes and music-note particles (reviewed in preview)
- [x] Scene 5 Same Cloud: rooftop + field; one cloud drawn in both panels across the border; both phones ring in sync (reviewed in preview)
- [x] Scene 6 Same Rain: 2x2 grid with Aoi, Hana, Ren (leak into a bucket), Kenji; rain, ripples, shared lightning flash and bolts (reviewed in preview)
- [x] Characters: seated pose, both-arms-forward pose, briefcase-overhead option
- [ ] User builds and watches chapters 4-6 on Windows; checks that panels clip cleanly at any window size
Acceptance: panels render independently with no bleeding; cloud visibly crosses the border; rain and lightning on all panels.
Depends on: Phase 3.

### Phase 5: Scenes 7-10 and story completion
- [x] Scene 7 After the Rain: rainbow (parallax arc bands), wet ground, puddles with ripples under the cat, camera follows the cat up the hill, Aoi walks to her window and opens it (sashes swing, curtains lift, light floods in)
- [x] Scene 8 Going Home: three golden-hour panels (Ren returns to the empty hut and the lamp comes on, Hana waits on the doorstep and the cat joins her, Kenji on the train looks at his missed calls and calls home); one crow flock flies across all panel borders
- [x] Scene 9 The Festival: lantern strings, three food stalls, crowd, rising moon, scripted particle fireworks with trails and sky flash, Aoi and Hana meet, wave, and watch together with the cat
- [x] Scene 10 Same Moon: Ren pours a second cup, Hana's mother comes home and the doorway lights, Kenji reaches Granny's gate (lanterns and farmhouse light), camera rises to a growing moon, closing line, fade out and loop
- [x] Fireworks module (capped rockets/sparks), stroke-font titles, loop from chapter 10 back to chapter 1 through the fade
- [ ] User builds and watches the whole film on Windows (one full loop) and confirms visuals, timing, FPS
Acceptance: ten scenes play in order, about 15 s each, loop to Scene 1 without a visible jump.
Depends on: Phase 4.

### Phase 6: Interaction and integration
Tasks: all controls, thought bubbles, picking in split screens, speed, rain toggle, firework click, optional audio.
Acceptance: every control in the proposal table works.
Depends on: Phase 5.

### Phase 7: Testing, optimisation, presentation
Tasks: full test pass, timing check, particle/FPS check, docs update, final ZIP, presentation notes.
Acceptance: all tests recorded with real results; builds in Code::Blocks; final ZIP verified.
Depends on: Phase 6.

## 7. Risks
| Risk | Mitigation |
|---|---|
| Time: 15 days, heavy scenes 4-6 and 9 | Build reusable systems first; benchmark scene early |
| Cannot visually test in the assistant's sandbox | User runs each phase and reports; features are marked complete only after that |
| 32/64-bit freeGLUT mismatch | Checklist step 2 and 5 |
| MinGW `gl.h` is OpenGL 1.1 only | Define missing constants by hand (e.g. `GL_MULTISAMPLE`) |
| Multisample unsupported on some drivers | Falls back to no antialiasing; not required for correctness |

## 8. Schedule (agreed)
Phases 1-2 by 23/11, Phase 3 by 26/11, Phase 4 by 29/11, Phase 5 by 02/12, Phase 6 by 04/12, Phase 7 05-06/12. Final presentation 06/12/2026.
