# CLAUDE.md - project context

Project: "A Day Through the Eyes of People", a 2D OpenGL animated short for a Computer Graphics Lab. The submitted proposal is the source of truth: ten scenes in fixed order, five characters plus a cat, Kenji is Granny Sato's grandson, Aoi meets Hana at the festival. Do not change story or relationships without asking.

## Technical restrictions
C++11, OpenGL fixed pipeline, freeGLUT, Code::Blocks, MinGW, Windows. No textures or models for primary visuals, no extra libraries or languages. Primitives: GL_QUADS, GL_TRIANGLES, GL_TRIANGLE_FAN, GL_POLYGON, GL_LINES, GL_LINE_LOOP, GL_POINTS, with GL_BLEND for glow, mist and fades.

## Conventions
- Logical coordinates 1280x720, origin bottom-left; letterbox 50 px.
- One centralized timer; all updates use `dt * speed`.
- Plain functions and structs, beginner-friendly, comments on transformations and transitions.
- Named constants in `Config.h`, no unexplained magic numbers.
- Declare GL constants missing from MinGW's gl.h.

## Workflow
Seven phases, one at a time (see docs/PROJECT_PLAN.md). Each phase must compile. Report tasks, files, tests with real results, bugs, next task. Mark tasks complete only after the user has run them. ZIP per phase: Project_Phase_01.zip ... Project_Phase_07_Final.zip, excluding bin/, obj/ and secrets. Update docs/PROGRESS.md after significant work.

## Limitations
The assistant cannot open a Windows OpenGL window; visual verification is done by the user.
