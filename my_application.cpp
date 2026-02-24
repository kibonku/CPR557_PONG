#include "my_application.h"

// Render factory
#include "my_simple_render_factory.h"

// use radian rather degree for angle
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Std
#include <stdexcept>
#include <array>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>

// GLFW time helper
#include <GLFW/glfw3.h>

namespace {

	// ---- Minimal 5x7 bitmap font (only needed characters) ----
	static const int FONT_W = 5;
	static const int FONT_H = 7;

	static const char* glyph(char ch) {
		// each glyph is 7 strings of length 5, '1' means filled pixel
		switch (ch) {
		case 'S': return
			"11111"
			"10000"
			"11110"
			"00001"
			"00001"
			"10001"
			"01110";
		case 'C': return
			"01111"
			"10000"
			"10000"
			"10000"
			"10000"
			"10000"
			"01111";
		case 'O': return
			"01110"
			"10001"
			"10001"
			"10001"
			"10001"
			"10001"
			"01110";
		case 'R': return
			"11110"
			"10001"
			"10001"
			"11110"
			"10100"
			"10010"
			"10001";
		case 'E': return
			"11111"
			"10000"
			"11110"
			"10000"
			"10000"
			"10000"
			"11111";
		case 'A': return
			"01110"
			"10001"
			"10001"
			"11111"
			"10001"
			"10001"
			"10001";
		case 'L': return
			"10000"
			"10000"
			"10000"
			"10000"
			"10000"
			"10000"
			"11111";
		case 'Y': return
			"10001"
			"10001"
			"01010"
			"00100"
			"00100"
			"00100"
			"00100";
		case 'P': return
			"11110"
			"10001"
			"10001"
			"11110"
			"10000"
			"10000"
			"10000";
		case 'D': return
			"11110"
			"10001"
			"10001"
			"10001"
			"10001"
			"10001"
			"11110";
		case ':': return
			"00000"
			"00100"
			"00100"
			"00000"
			"00100"
			"00100"
			"00000";
		case '(': return
			"00010"
			"00100"
			"01000"
			"01000"
			"01000"
			"00100"
			"00010";
		case ')': return
			"01000"
			"00100"
			"00010"
			"00010"
			"00010"
			"00100"
			"01000";
		case ' ': return
			"00000"
			"00000"
			"00000"
			"00000"
			"00000"
			"00000"
			"00000";
		default:
			return nullptr;
		}
	}

	std::vector<MyModel::Vertex> makeUnitQuadVertices(const glm::vec3& c0,
		const glm::vec3& c1,
		const glm::vec3& c2,
		const glm::vec3& c3)
	{
		// Unit quad centered at origin in NDC-ish model space: [-0.5, 0.5]^2
		// Colors per corner so the ball can be "rainbow" if desired.
		return {
			// first tri
			{{-0.5f, -0.5f}, c0},
			{{ 0.5f, -0.5f}, c1},
			{{ 0.5f,  0.5f}, c2},
			// second tri
			{{-0.5f, -0.5f}, c0},
			{{ 0.5f,  0.5f}, c2},
			{{-0.5f,  0.5f}, c3},
		};
	}

	float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }

	glm::vec2 normalizedOr(const glm::vec2& v, const glm::vec2& fallback)
	{
		float len = std::sqrt(v.x * v.x + v.y * v.y);
		if (len < 1e-6f) return fallback;
		return v / len;
	}

	// 7-seg lookup: seg index: 0=top,1=tl,2=tr,3=mid,4=bl,5=br,6=bottom
	static const bool DIGIT_SEG[10][7] = {
	  {1,1,1,0,1,1,1}, //0
	  {0,0,1,0,0,1,0}, //1
	  {1,0,1,1,1,0,1}, //2
	  {1,0,1,1,0,1,1}, //3
	  {0,1,1,1,0,1,0}, //4
	  {1,1,0,1,0,1,1}, //5
	  {1,1,0,1,1,1,1}, //6
	  {1,0,1,0,0,1,0}, //7
	  {1,1,1,1,1,1,1}, //8
	  {1,1,1,1,0,1,1}  //9
	};
}

MyApplication::MyApplication()
{
	// initialize HUD segment indices to "unused"
	for (int d = 0; d < 2; d++) {
		for (int s = 0; s < 7; s++) {
			m_pong.hudRallySeg[d][s] = -1;
			m_pong.hudSpeedSeg[d][s] = -1;
			m_pong.hudSegOnScale[d][s] = { 0.0f, 0.0f };
			m_pong.hudSegOnScale2[d][s] = { 0.0f, 0.0f };
		}
	}

	_loadGameObjects();
}

// --- HUD helpers ---
void MyApplication::setDigitSegments(int segIdx[7], const glm::vec2 onScale[7], int value)
{
	value = std::clamp(value, 0, 9);

	for (int s = 0; s < 7; s++) {
		int objIndex = segIdx[s];
		if (objIndex < 0) continue;

		auto& obj = m_vMyGameObjects[objIndex];
		if (DIGIT_SEG[value][s]) {
			obj.transform2d.scale = onScale[s];
		}
		else {
			obj.transform2d.scale = { 0.0f, 0.0f }; // hide
		}
	}
}

int MyApplication::addHudPixelQuad(const std::string& name, glm::vec3 color, glm::vec2 pos, glm::vec2 scale)
{
	auto obj = MyGameObject::createGameObject(name);
	auto verts = makeUnitQuadVertices(color, color, color, color);
	obj.model = std::make_shared<MyModel>(m_myDevice, verts);
	obj.transform2d.translation = pos;
	obj.transform2d.scale = scale;
	obj.transform2d.rotation = 0.0f;

	int idx = static_cast<int>(m_vMyGameObjects.size());
	m_vMyGameObjects.push_back(std::move(obj));
	return idx;
}

void MyApplication::updateHud()
{
	// rally: 0..99
	int rallyShown = std::clamp(m_pong.rallyCount, 0, 99);
	int speedShown = std::clamp(m_pong.speedLevel, 0, 99);

	int rT = rallyShown / 10, rO = rallyShown % 10;
	int sT = speedShown / 10, sO = speedShown % 10;

	setDigitSegments(m_pong.hudRallySeg[0], m_pong.hudSegOnScale[0], rT);
	setDigitSegments(m_pong.hudRallySeg[1], m_pong.hudSegOnScale[1], rO);

	setDigitSegments(m_pong.hudSpeedSeg[0], m_pong.hudSegOnScale2[0], sT);
	setDigitSegments(m_pong.hudSpeedSeg[1], m_pong.hudSegOnScale2[1], sO);
}

void MyApplication::run()
{
	MySimpleRenderFactory simpleRenderFactory{ m_myDevice, m_myRenderer.swapChainRenderPass() };

	m_myWindow.bindMyApplication(this);

	while (!m_myWindow.shouldClose())
	{
		m_myWindow.pollEvents();

		_updateGameLogic();

		if (auto commandBuffer = m_myRenderer.beginFrame())
		{
			m_myRenderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderFactory.renderGameObjects(commandBuffer, m_vMyGameObjects);
			m_myRenderer.endSwapChainRenderPass(commandBuffer);

			m_myRenderer.endFrame();
		}
	}

	vkDeviceWaitIdle(m_myDevice.device());
}

void MyApplication::_loadGameObjects()
{
	// --- Paddle TOP (blue) ---
	{
		auto obj = MyGameObject::createGameObject("paddle_top");
		auto verts = makeUnitQuadVertices(
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f }
		);
		obj.model = std::make_shared<MyModel>(m_myDevice, verts);
		obj.transform2d.translation = { 0.0f, 0.80f };
		obj.transform2d.scale = { m_pong.paddleHalf.x * 2.0f, m_pong.paddleHalf.y * 2.0f };
		obj.transform2d.rotation = 0.0f;
		m_pong.paddleTop = static_cast<int>(m_vMyGameObjects.size());
		m_vMyGameObjects.push_back(std::move(obj));
	}

	// --- Paddle BOTTOM (red) ---
	{
		auto obj = MyGameObject::createGameObject("paddle_bottom");
		auto verts = makeUnitQuadVertices(
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f }
		);
		obj.model = std::make_shared<MyModel>(m_myDevice, verts);
		obj.transform2d.translation = { 0.0f, -0.80f };
		obj.transform2d.scale = { m_pong.paddleHalf.x * 2.0f, m_pong.paddleHalf.y * 2.0f };
		obj.transform2d.rotation = 0.0f;
		m_pong.paddleBottom = static_cast<int>(m_vMyGameObjects.size());
		m_vMyGameObjects.push_back(std::move(obj));
	}

	// --- Ball (rainbow quad) ---
	{
		auto obj = MyGameObject::createGameObject("ball");
		auto verts = makeUnitQuadVertices(
			{ 1.0f, 0.2f, 0.2f },
			{ 0.2f, 1.0f, 0.2f },
			{ 0.2f, 0.2f, 1.0f },
			{ 1.0f, 1.0f, 0.2f }
		);
		obj.model = std::make_shared<MyModel>(m_myDevice, verts);
		obj.transform2d.translation = { 0.0f, 0.0f };
		obj.transform2d.scale = { m_pong.ballHalf.x * 2.0f, m_pong.ballHalf.y * 2.0f };
		obj.transform2d.rotation = 0.25f;
		m_pong.ball = static_cast<int>(m_vMyGameObjects.size());
		m_vMyGameObjects.push_back(std::move(obj));
	}

	// --- HUD: status bar + 7-seg digits + legend boxes ---
	{
		const float HUD_Y = -0.92f;
		const float HUD_SCALE = 0.70f;   

		auto makeHudSeg = [&](const std::string& name, glm::vec3 color, glm::vec2 pos, glm::vec2 scale) {
			auto obj = MyGameObject::createGameObject(name);
			auto verts = makeUnitQuadVertices(color, color, color, color);
			obj.model = std::make_shared<MyModel>(m_myDevice, verts);
			obj.transform2d.translation = pos;
			obj.transform2d.scale = scale;
			obj.transform2d.rotation = 0.0f;

			int idx = static_cast<int>(m_vMyGameObjects.size());
			m_vMyGameObjects.push_back(std::move(obj));
			return idx;
		};

		// --- Text labels (pixel font) ---
		// We don't have font rendering, so we draw letters using tiny quads (5x7 glyphs).
		{
			auto drawText = [&](const std::string& text, glm::vec2 origin, float pixel, float spacing) {
				glm::vec3 white{ 1.f, 1.f, 1.f };
				float x = origin.x;
				float y = origin.y;

				for (size_t i = 0; i < text.size(); i++) {
					const char* g = glyph(text[i]);
					if (!g) {
						x += (FONT_W + 1) * (pixel + spacing);
						continue;
					}

					for (int row = 0; row < FONT_H; row++) {
						for (int col = 0; col < FONT_W; col++) {
							char bit = g[row * FONT_W + col];
							if (bit == '1') {
								glm::vec2 p{
									x + col * (pixel + spacing),
									y + row * (pixel + spacing)
								};
								addHudPixelQuad(
									"hud_txt_" + std::to_string(i) + "_" + std::to_string(row) + "_" + std::to_string(col),
									white,
									p,
									{ pixel, pixel }
								);
							}
						}
					}

					x += (FONT_W + 1) * (pixel + spacing);
				}
			};

			// Labels 
			const float px = 0.010f * HUD_SCALE;
			const float sp = 0.0025f * HUD_SCALE;
			drawText("SCORE:", { -0.95f, HUD_Y - 0.05f }, px, sp);
			drawText("SPEED:",        {  0.25f, HUD_Y - 0.05f }, px, sp);
		}

		// digit geometry (bigger so it is clearly visible)
		const float th  = 0.016f * HUD_SCALE;
		const float dw  = 0.085f * HUD_SCALE;
		const float dh  = 0.140f * HUD_SCALE;
		const float gap = 0.025f * HUD_SCALE;

		auto buildDigit = [&](glm::vec2 origin, int segOut[7], glm::vec2 onScaleOut[7], const std::string& prefix) {
			glm::vec2 hScale{ dw, th };
			glm::vec2 vScale{ th, dh * 0.50f };

			glm::vec2 top = origin + glm::vec2(0.0f, -dh * 0.50f);
			glm::vec2 mid = origin + glm::vec2(0.0f, 0.0f);
			glm::vec2 bottom = origin + glm::vec2(0.0f, dh * 0.50f);

			glm::vec2 tl = origin + glm::vec2(-dw * 0.50f, -dh * 0.25f);
			glm::vec2 tr = origin + glm::vec2(dw * 0.50f, -dh * 0.25f);
			glm::vec2 bl = origin + glm::vec2(-dw * 0.50f, dh * 0.25f);
			glm::vec2 br = origin + glm::vec2(dw * 0.50f, dh * 0.25f);

			// digits are white
			glm::vec3 white{ 1.0f, 1.0f, 1.0f };

			segOut[0] = makeHudSeg(prefix + "_top", white, top, hScale);        onScaleOut[0] = hScale;
			segOut[1] = makeHudSeg(prefix + "_tl", white, tl, vScale);         onScaleOut[1] = vScale;
			segOut[2] = makeHudSeg(prefix + "_tr", white, tr, vScale);         onScaleOut[2] = vScale;
			segOut[3] = makeHudSeg(prefix + "_mid", white, mid, hScale);       onScaleOut[3] = hScale;
			segOut[4] = makeHudSeg(prefix + "_bl", white, bl, vScale);         onScaleOut[4] = vScale;
			segOut[5] = makeHudSeg(prefix + "_br", white, br, vScale);         onScaleOut[5] = vScale;
			segOut[6] = makeHudSeg(prefix + "_bot", white, bottom, hScale);    onScaleOut[6] = hScale;
		};

		// Digits positions (two digits each)
		// Place them to the right of the labels above.
		glm::vec2 rallyBase{ -0.5f, HUD_Y };
		glm::vec2 speedBase{  0.7f, HUD_Y };

		// Rally (2 digits)
		buildDigit(rallyBase + glm::vec2(0.0f, 0.0f), m_pong.hudRallySeg[0], m_pong.hudSegOnScale[0], "hud_r0");
		buildDigit(rallyBase + glm::vec2(dw + gap, 0.0f), m_pong.hudRallySeg[1], m_pong.hudSegOnScale[1], "hud_r1");

		// Speed (2 digits)
		buildDigit(speedBase + glm::vec2(0.0f, 0.0f), m_pong.hudSpeedSeg[0], m_pong.hudSegOnScale2[0], "hud_s0");
		buildDigit(speedBase + glm::vec2(dw + gap, 0.0f), m_pong.hudSpeedSeg[1], m_pong.hudSegOnScale2[1], "hud_s1");
	}

	resetGame(); // also updates HUD
}

void MyApplication::_updateGameLogic()
{
	if (m_pong.paddleTop < 0 || m_pong.paddleBottom < 0 || m_pong.ball < 0) return;

	// --- dt ---
	double now = glfwGetTime();
	if (m_pong.lastTimeSec <= 0.0) m_pong.lastTimeSec = now;
	float dt = static_cast<float>(now - m_pong.lastTimeSec);
	m_pong.lastTimeSec = now;
	// avoid huge jumps after window drag / breakpoints
	dt = clampf(dt, 0.0f, 0.033f);

	auto& top = m_vMyGameObjects[m_pong.paddleTop];
	auto& bot = m_vMyGameObjects[m_pong.paddleBottom];
	auto& ball = m_vMyGameObjects[m_pong.ball];

	// --- Apply paddle input (horizontal movement) ---
	{
		float dxTop = static_cast<float>(m_pong.paddleTopDir) * m_pong.paddleSpeed * dt;
		float dxBot = static_cast<float>(m_pong.paddleBottomDir) * m_pong.paddleSpeed * dt;
		top.transform2d.translation.x += dxTop;
		bot.transform2d.translation.x += dxBot;

		// clamp paddles within screen
		float minX = -1.0f + m_pong.paddleHalf.x;
		float maxX = 1.0f - m_pong.paddleHalf.x;
		top.transform2d.translation.x = clampf(top.transform2d.translation.x, minX, maxX);
		bot.transform2d.translation.x = clampf(bot.transform2d.translation.x, minX, maxX);

		// reset intents (so holding key keeps sending events each pollEvents)
		m_pong.paddleTopDir = 0;
		m_pong.paddleBottomDir = 0;
	}

	// --- Move ball ---
	ball.transform2d.translation += m_pong.ballVel * dt;
	ball.transform2d.rotation += 0.8f * dt;

	// --- Wall collisions (left/right) ---
	{
		float minX = -1.0f + m_pong.ballHalf.x;
		float maxX = 1.0f - m_pong.ballHalf.x;
		if (ball.transform2d.translation.x < minX)
		{
			ball.transform2d.translation.x = minX;
			m_pong.ballVel.x *= -1.0f;
		}
		else if (ball.transform2d.translation.x > maxX)
		{
			ball.transform2d.translation.x = maxX;
			m_pong.ballVel.x *= -1.0f;
		}
	}

	// --- Paddle collision helper (AABB vs AABB in NDC) ---
	auto collideWithPaddle = [&](const MyGameObject& paddle, bool isTop) {
		const glm::vec2 p = paddle.transform2d.translation;
		const glm::vec2 b = ball.transform2d.translation;

		float dx = std::abs(b.x - p.x);
		float dy = std::abs(b.y - p.y);
		if (dx > (m_pong.paddleHalf.x + m_pong.ballHalf.x)) return false;
		if (dy > (m_pong.paddleHalf.y + m_pong.ballHalf.y)) return false;

		// Only bounce if ball is moving towards the paddle
		if (isTop && m_pong.ballVel.y <= 0.0f) return false;
		if (!isTop && m_pong.ballVel.y >= 0.0f) return false;

		// Place ball just outside the paddle and flip Y velocity
		float sign = isTop ? -1.0f : 1.0f;
		ball.transform2d.translation.y = p.y + sign * (m_pong.paddleHalf.y + m_pong.ballHalf.y);
		m_pong.ballVel.y *= -1.0f;

		// Add a little "angle" depending on where it hit
		float t = (b.x - p.x) / m_pong.paddleHalf.x; // ~[-1,1]
		t = clampf(t, -1.0f, 1.0f);
		m_pong.ballVel.x += 0.35f * t;

		// ---- Phase 6: rally score + speed ramp on each paddle hit ----
		m_pong.rallyCount += 1;

		// HUD speed level increases every rally
		m_pong.speedLevel = std::min(m_pong.speedLevel + 1, m_pong.maxSpeedLevel);

		// actual ball speed increases too
		m_pong.speed *= m_pong.speedUpFactor;

		glm::vec2 dir = normalizedOr(m_pong.ballVel, { 0.0f, isTop ? -1.0f : 1.0f });
		m_pong.ballVel = dir * m_pong.speed;

		updateHud();
		return true;
	};

	(void)collideWithPaddle(top, true);
	(void)collideWithPaddle(bot, false);

	// --- Out of bounds: score + reset (classic Pong) ---
	{
		if (ball.transform2d.translation.y > 1.0f + m_pong.ballHalf.y)
		{
			// bottom player scores
			m_pong.scoreBottom++;
			std::cout << "Score  Top:" << m_pong.scoreTop << "  Bottom:" << m_pong.scoreBottom << std::endl;
			resetGame();
		}
		else if (ball.transform2d.translation.y < -1.0f - m_pong.ballHalf.y)
		{
			// top player scores
			m_pong.scoreTop++;
			std::cout << "Score  Top:" << m_pong.scoreTop << "  Bottom:" << m_pong.scoreBottom << std::endl;
			resetGame();
		}
	}
}

void MyApplication::movePaddle(bool moveLeftOrRight, int index)
{
	// index: 0 (arrow keys) -> TOP paddle, 1 (A/D) -> BOTTOM paddle
	int dir = moveLeftOrRight ? -1 : 1;
	if (index == 0) m_pong.paddleTopDir = dir;
	else if (index == 1) m_pong.paddleBottomDir = dir;
}

void MyApplication::resetGame()
{
	if (m_pong.paddleTop < 0 || m_pong.paddleBottom < 0 || m_pong.ball < 0) return;

	auto& top = m_vMyGameObjects[m_pong.paddleTop];
	auto& bot = m_vMyGameObjects[m_pong.paddleBottom];
	auto& ball = m_vMyGameObjects[m_pong.ball];

	// reset positions
	top.transform2d.translation = { 0.0f, 0.80f };
	bot.transform2d.translation = { 0.0f, -0.80f };
	ball.transform2d.translation = { 0.0f, 0.0f };
	ball.transform2d.rotation = 0.25f;

	// reset Phase 6 values
	m_pong.rallyCount = 0;
	m_pong.speedLevel = 1;
	m_pong.speed = m_pong.baseSpeed;

	// reset velocity: alternate serve direction based on score parity
	float serveUp = ((m_pong.scoreTop + m_pong.scoreBottom) % 2 == 0) ? 1.0f : -1.0f;
	glm::vec2 dir = normalizedOr({ 0.45f, 0.90f * serveUp }, { 0.0f, serveUp });
	m_pong.ballVel = dir * m_pong.speed;

	updateHud();
	std::cout << "Reset game (SPACE)." << std::endl;
}
