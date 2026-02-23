# Computer Project #2: Pong (Vulkan + GLFW)

------------------------------------------------------------
Overview
------------------------------------------------------------
This project reproduces a simple Pong-style game using Vulkan and GLFW.
It includes:
- Ball movement and wall bounce
- Paddle movement via keyboard input
- Paddle–ball collision
- Game reset
- Two-player mode (two paddles)
- Simple creativity features (see "Creativity" section)

The paddles move horizontally (left/right), similar to the assignment example.

------------------------------------------------------------
Controls
------------------------------------------------------------
Bottom paddle (Blue):   A / D
Top paddle (Red):       Left Arrow / Right Arrow
Reset game:             SPACE
Quit:                   ESC

------------------------------------------------------------
Phases / Requirements Mapping
------------------------------------------------------------
Phase 1 (Move the ball):
- The ball moves continuously and bounces off the left/right walls.

Phase 2 (Move the paddle):
- Paddles can be moved using the keyboard inputs listed above.

Phase 3 (Collision between paddle and ball):
- When the ball collides with a paddle, the ball reflects and continues moving.

Phase 4 (Reset game):
- Press SPACE to reset ball position/velocity and paddle positions.

Phase 5 (Two players):
- Two paddles are included (top and bottom) with separate controls.

Phase 6 (Creativity):
- Ball speed increases slightly after paddle hits (if enabled in code).
- (Optional) Console messages print reset/score events for clarity.

------------------------------------------------------------
Build and Run (Windows / MSYS2 MinGW64)
------------------------------------------------------------
1) Make sure Vulkan SDK is installed and your compiler environment is ready.

2) Compile shaders (generates .spv files):
   ./compile-win.bat

   After this, the folder "shaders/" should contain:
   - simple_shader.vert.spv
   - simple_shader.frag.spv

3) Build:
   mingw32-make

4) Run:
   ./main.exe

Optional (recommended): If your Makefile supports it, you can also use:
   make run
This should compile shaders, build the program, and run it.

------------------------------------------------------------
Build and Run (Linux / macOS)
------------------------------------------------------------
1) Compile shaders:
   ./compile-unx.bat  (or the provided unix compile script in your template)

2) Build:
   make

3) Run:
   ./main

(Exact command names may vary depending on your template setup.)

------------------------------------------------------------
Notes / Troubleshooting
------------------------------------------------------------
1) Shader file not found error:
   If you see:
     failed to open file: shaders/simple_shader.vert.spv
   it means the SPIR-V shader files were not generated or the program was
   launched from a different working directory.

   Fix:
   - Run compile-win.bat again to generate .spv files.
   - Run the executable from the project root folder (where shaders/ exists).

2) VS Code:
   - Running with "Run and Debug (F5)" is recommended.
   - Running directly from the "Run" button in main.cpp may use a different
     working directory and fail to find shaders.

------------------------------------------------------------
Files / Submission
------------------------------------------------------------
- All source code included in the project folder
- Screenshot of the view window (in action)
- This README.txt
- Compressed as a zip for Canvas submission
