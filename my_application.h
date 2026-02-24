#ifndef __MY_APPLICATION_H__
#define __MY_APPLICATION_H__

#include "my_window.h"
#include "my_device.h"
#include "my_renderer.h"
#include "my_game_object.h"

#include <memory>
#include <vector>
#include <string>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class MyApplication 
{
public:
	static constexpr int WIDTH = 1000;
	static constexpr int HEIGHT = 1000;

	MyApplication();

	void run();
	void movePaddle(bool moveLeftOrRight, int index);
	void resetGame();

private:
	void _loadGameObjects();
	void _updateGameLogic();

	// Phase 6 HUD helpers
	int addHudPixelQuad(const std::string& name, glm::vec3 color, glm::vec2 pos, glm::vec2 scale);
	void setDigitSegments(int segIdx[7], const glm::vec2 onScale[7], int value);
	void updateHud();

	// --- Pong state (NDC coordinates: x,y in [-1, 1]) ---
	struct PongState
	{
		// Object indices inside m_vMyGameObjects
		int paddleTop = -1;
		int paddleBottom = -1;
		int ball = -1;

		// Sizes (half extents in NDC)
		glm::vec2 paddleHalf{ 0.20f, 0.03f }; // width/2, height/2
		glm::vec2 ballHalf{ 0.025f, 0.025f };

		float paddleSpeed = 0.9f; // NDC units per second
		float ballSpeed = 0.7f;   // legacy (kept); we now use speed/baseSpeed for ramp
		glm::vec2 ballVel{ 0.35f, 0.65f }; // will be normalized in reset

		int scoreTop = 0;
		int scoreBottom = 0;

		// Input intents (set by movePaddle, consumed in _updateGameLogic)
		int paddleTopDir = 0;    // -1 left, +1 right
		int paddleBottomDir = 0; // -1 left, +1 right

		double lastTimeSec = 0.0;

		// --------------------
		// Phase 6 (Creativity)
		// --------------------
		// 1 rally hit (ball touches any paddle) => rallyCount++
		int rallyCount = 0;

		// speed level shown on HUD (01,02,...)
		int speedLevel = 1;              // start from 01
		int maxSpeedLevel = 99;

		// physics speed values
		float baseSpeed = 0.70f;         // actual initial ball speed (feel free to tune)
		float speed = 0.70f;             // current ball speed magnitude
		float speedUpFactor = 1.05f;     // multiply actual speed each paddle hit

		// HUD: 2 digits for rally + 2 digits for speed, each digit has 7 segments
		// store indices into m_vMyGameObjects; -1 means unused
		int hudRallySeg[2][7];
		int hudSpeedSeg[2][7];

		// store the "on" scale for each segment; OFF => scale {0,0}
		glm::vec2 hudSegOnScale[2][7];
		glm::vec2 hudSegOnScale2[2][7];
	} m_pong;

	MyWindow                  m_myWindow{ WIDTH, HEIGHT, "Assignment 2" };
	MyDevice                  m_myDevice{ m_myWindow };
	MyRenderer                m_myRenderer{ m_myWindow, m_myDevice };

	std::vector<MyGameObject> m_vMyGameObjects;
};

#endif
