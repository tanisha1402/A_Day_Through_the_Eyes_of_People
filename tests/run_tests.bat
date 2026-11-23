@echo off
REM Headless tests (no window needed). Run from the project folder: tests\run_tests.bat
g++ -std=c++11 -O2 -Wall -Wextra -Itests/stub src/Utils.cpp src/Environment.cpp src/Characters.cpp src/TimeOfDay.cpp src/Camera.cpp src/World.cpp src/Particles.cpp src/Viewports.cpp src/Panels.cpp src/Scenes.cpp src/Animation.cpp tests/TestScreen.cpp tests/mock_gl.cpp tests/test_all.cpp -o tests\test_all.exe && tests\test_all.exe
