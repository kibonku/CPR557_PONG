# Computer Project #2: Pong (Vulkan + GLFW)

------------------------------------------------------------
Overview
------------------------------------------------------------
This project implements a Pong-style game using Vulkan and GLFW.
It includes:
- Ball movement and wall bounce
- Paddle movement via keyboard input
- Paddle–ball collision
- Game reset
- Two-player mode (two paddles)
- Creativity feature: on-screen HUD showing Rally Score and Speed Level

The paddles move horizontally (left/right), similar to the assignment example.

------------------------------------------------------------
Controls
------------------------------------------------------------
Top paddle (Red):        Left Arrow / Right Arrow
Bottom paddle (Blue):    A / D
Reset game:              SPACE
Quit:                    ESC

------------------------------------------------------------
Phases / Requirements Mapping
------------------------------------------------------------
Phase 1 (Move the ball):
- The ball moves continuously and bounces off the side walls.

Phase 2 (Move the paddle):
- Two paddles can be moved using the keyboard inputs above.

Phase 3 (Collision between paddle and ball):
- Ball reflects when it collides with a paddle (AABB collision).

Phase 4 (Reset game):
- SPACE resets the game (ball + paddles; HUD resets).

Phase 5 (Two players):
- Two paddles are included (top and bottom) with separate controls.

Phase 6 (Creativity):
- On-screen HUD (drawn inside the Vulkan view using pixel-quads + 7-seg digits):
  * Score (Rally): counts the number of paddle hits in the current rally (00–99).
  * Speed: shows the current speed level (01–99), increasing every rally hit.
- Speed increases slightly after each paddle hit (rally hit). When a point is lost,
  the rally score and speed level reset.

------------------------------------------------------------
Build and Run (Windows / MSYS2 MinGW64)
------------------------------------------------------------
1) Prerequisites
   - Install the Vulkan SDK.
   - Make sure MSYS2 MinGW64/UCRT64 build tools are available.

2) Compile shaders (generates .spv files)
   ./compile-win.bat

   After this step, the folder "shaders/" should contain:
   - simple_shader.vert.spv
   - simple_shader.frag.spv

3) Build
   mingw32-make

4) Run
   ./Assignment2.exe

(Optional) If supported in your Makefile:
   make run

------------------------------------------------------------
Notes / Troubleshooting
------------------------------------------------------------
1) Shader file not found error:
   If you see:
     failed to open file: shaders/simple_shader.vert.spv
   run compile-win.bat again and make sure you launch the program from the
   project root folder (where "shaders/" exists).

2) VS Code:
   - Running with "Run and Debug (F5)" is recommended.
   - If a debug run fails to find shaders, ensure the working directory is
     the project root folder.
     
------------------------------------------------------------
GitHub Repository
------------------------------------------------------------
For reference, the source code is also available on GitHub:

`[https://github.com/kibonku/CPR557_PONG](https://github.com/kibonku/CPR557_PONG.git)`
