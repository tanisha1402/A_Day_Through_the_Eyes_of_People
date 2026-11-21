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
| Hilltop | 0 - 1300 | Aoi's mansion on the high ground, garden, gate |
| Downtown | 1300 - 2900 | Bakery, offices, school, railway crossing and track, station |
| Riverside | 2900 - 4000 | River, fishing dock, Ren's hut, small wooden houses |
| Countryside | 4000 - 5200 | Rice fields, Granny Sato's farmhouse and gate |

Layout is adjustable in Phase 3 if composition requires it. Interiors (Aoi's room, Ren's hut, classroom, office, piano room) are drawn as separate cutaway panels in local screen space, not inside the panorama.

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
Tasks: split into modules; shapes, gradients, glow, outlines; cloud, tree, house; coordinate and letterbox handling; test screen showing every utility.
Acceptance: test screen displays every utility correctly; stable ~60 fps; clean build in Code::Blocks with no warnings.
Depends on: Phase 1.

### Phase 3: World, characters, Scenes 1-3
Tasks: town panorama with four districts; `drawPerson` with walk and idle; `drawCat`; `TimeOfDay`; `Camera`; Scenes 1, 2, 3 (Scene 2 first as the visual benchmark); sunrise, river mist, crossing, train.
Acceptance: scenes 1-3 play and transition smoothly; characters look distinct; walk cycle works; train and crossing lights sync.
Depends on: Phase 2.

### Phase 4: Split-screen Scenes 4-6
Tasks: viewport manager and scissor; three-panel clock scene; shared cloud; rain and lightning; panel palettes (warm, grey, cold blue).
Acceptance: panels render independently with no bleeding; cloud visibly crosses the border; rain and lightning on all panels.
Depends on: Phase 3.

### Phase 5: Scenes 7-10 and story completion
Tasks: rainbow, puddles, cat journey, sunset and crows, festival, lanterns, fireworks, Aoi and Hana meeting, final pan and moon, loop.
Acceptance: ten scenes play in order, each about 15 s, loop to Scene 1 without a visible jump.
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
