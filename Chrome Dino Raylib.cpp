#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;

const int BASE_WIDTH = 800;
const int BASE_HEIGHT = 400;
const float GROUND_Y_RATIO = 0.875f;

float scaleX = 1.0f;
float scaleY = 1.0f;
float globalScale = 1.0f;
int currentWidth = BASE_WIDTH;
int currentHeight = BASE_HEIGHT;
bool fullscreen = false;

class Dinosaur
{
public:
	Rectangle rect;
	float velocityY;
	bool isJumping;
	bool isDucking;
	bool isAlive;
	float groundY;
	float normalHeight;
	float duckHeight;
	float normalWidth;
	float duckWidth;
	
	Dinosaur(float groundY) : groundY(groundY)
	{
		normalWidth = 44.0f * 1.5f;
		normalHeight = 47.0f * 1.5f;
		duckWidth = 59.0f * 1.5f;
		duckHeight = 20.0f * 1.5f;  // 缩小下蹲碰撞体积，匹配原版低飞翼龙躲避
		rect = {80.0f * 1.5f, groundY - normalHeight, normalWidth, normalHeight};
		velocityY = 0;
		isJumping = false;
		isDucking = false;
		isAlive = true;
	}
	
	void Update(float deltaTime)
	{
		float timeFactor = 60.0f * deltaTime;
		if (isDucking)
		{
			rect.height = duckHeight;
			rect.y = groundY - duckHeight;
		}
		else
		{
			rect.height = normalHeight;
			if (!isJumping)
				rect.y = groundY - normalHeight;
		}
		
		if (isJumping)
		{
			velocityY += 1.5f * scaleY * timeFactor;
			if (velocityY > 18.0f * scaleY)
				velocityY = 18.0f * scaleY;
			rect.y += velocityY * timeFactor;
			if (rect.y >= groundY - rect.height)
			{
				rect.y = groundY - rect.height;
				velocityY = 0;
				isJumping = false;
			}
		}
	}
	
	void Jump()
	{
		if (!isJumping && !isDucking)
		{
			velocityY = -18.0f * scaleY; // 降低跳跃初速度，手感更贴近原版
			isJumping = true;
		}
	}
	
	void Duck(bool duck)
	{
		if (duck && !isJumping)
			isDucking = true;
		else
			isDucking = false;
	}
	
	void Draw()
	{
		if (isDucking)
			DrawDuckingDino();
		else if (isJumping)
			DrawJumpingDino();
		else
			DrawRunningDino();
	}
	
private:
	void DrawRunningDino()
	{
		int frame = ((int)(GetTime() * 10) % 2);
		float x = rect.x;
		float y = rect.y;
		
		DrawRectangle(x + 8 * globalScale, y + 4 * globalScale, 30 * globalScale, 30 * globalScale, DARKGRAY);
		DrawRectangle(x + 4 * globalScale, y + 8 * globalScale, 36 * globalScale, 26 * globalScale, DARKGRAY);
		DrawRectangle(x + 28 * globalScale, y - 4 * globalScale, 16 * globalScale, 16 * globalScale, DARKGRAY);
		DrawRectangle(x + 32 * globalScale, y - 8 * globalScale, 12 * globalScale, 12 * globalScale, DARKGRAY);
		DrawCircle(x + 40 * globalScale, y - 2 * globalScale, 3 * globalScale, WHITE);
		DrawCircle(x + 41 * globalScale, y - 2 * globalScale, 1.5f * globalScale, BLACK);
		DrawCircle(x + 43 * globalScale, y + 2 * globalScale, 1 * globalScale, BLACK);
		
		if (frame == 0)
		{
			DrawTriangle({x + 4 * globalScale, y + 12 * globalScale}, {x - 6 * globalScale, y + 4 * globalScale}, {x - 2 * globalScale, y + 16 * globalScale}, DARKGRAY);
		}
		else
		{
			DrawTriangle({x + 4 * globalScale, y + 10 * globalScale}, {x - 8 * globalScale, y + 6 * globalScale}, {x - 4 * globalScale, y + 18 * globalScale}, DARKGRAY);
		}
		DrawRectangle(x + 20 * globalScale, y + 20 * globalScale, 4 * globalScale, 12 * globalScale, DARKGRAY);
		
		if (frame == 0)
		{
			DrawRectangle(x + 10 * globalScale, y + 34 * globalScale, 8 * globalScale, 13 * globalScale, DARKGRAY);
			DrawRectangle(x + 26 * globalScale, y + 34 * globalScale, 8 * globalScale, 8 * globalScale, DARKGRAY);
		}
		else
		{
			DrawRectangle(x + 10 * globalScale, y + 34 * globalScale, 8 * globalScale, 8 * globalScale, DARKGRAY);
			DrawRectangle(x + 26 * globalScale, y + 34 * globalScale, 8 * globalScale, 13 * globalScale, DARKGRAY);
		}
		DrawRectangle(x + 8 * globalScale, y + 44 * globalScale, 28 * globalScale, 3 * globalScale, DARKGRAY);
	}
	
	void DrawJumpingDino()
	{
		float x = rect.x;
		float y = rect.y;
		DrawRectangle(x + 8 * globalScale, y + 4 * globalScale, 30 * globalScale, 30 * globalScale, DARKGRAY);
		DrawRectangle(x + 4 * globalScale, y + 8 * globalScale, 36 * globalScale, 26 * globalScale, DARKGRAY);
		DrawRectangle(x + 28 * globalScale, y - 4 * globalScale, 16 * globalScale, 16 * globalScale, DARKGRAY);
		DrawRectangle(x + 32 * globalScale, y - 8 * globalScale, 12 * globalScale, 12 * globalScale, DARKGRAY);
		DrawCircle(x + 40 * globalScale, y - 2 * globalScale, 3 * globalScale, WHITE);
		DrawCircle(x + 41 * globalScale, y - 2 * globalScale, 1.5f * globalScale, BLACK);
		DrawCircle(x + 43 * globalScale, y + 2 * globalScale, 1 * globalScale, BLACK);
		DrawTriangle({x + 4 * globalScale, y + 12 * globalScale}, {x - 6 * globalScale, y + 6 * globalScale}, {x - 2 * globalScale, y + 16 * globalScale}, DARKGRAY);
		DrawRectangle(x + 20 * globalScale, y + 20 * globalScale, 4 * globalScale, 12 * globalScale, DARKGRAY);
		DrawRectangle(x + 10 * globalScale, y + 34 * globalScale, 8 * globalScale, 6 * globalScale, DARKGRAY);
		DrawRectangle(x + 26 * globalScale, y + 34 * globalScale, 8 * globalScale, 6 * globalScale, DARKGRAY);
		DrawRectangle(x + 8 * globalScale, y + 40 * globalScale, 28 * globalScale, 3 * globalScale, DARKGRAY);
	}
	
	void DrawDuckingDino()
	{
		int frame = ((int)(GetTime() * 10) % 2);
		float x = rect.x;
		float y = rect.y;
		DrawRectangle(x + 4 * globalScale, y + 2 * globalScale, 50 * globalScale, 18 * globalScale, DARKGRAY);
		DrawRectangle(x + 44 * globalScale, y - 2 * globalScale, 14 * globalScale, 14 * globalScale, DARKGRAY);
		DrawCircle(x + 54 * globalScale, y + 2 * globalScale, 3 * globalScale, WHITE);
		DrawCircle(x + 55 * globalScale, y + 2 * globalScale, 1.5f * globalScale, BLACK);
		DrawCircle(x + 57 * globalScale, y + 6 * globalScale, 1 * globalScale, BLACK);
		DrawTriangle({x + 4 * globalScale, y + 6 * globalScale}, {x - 4 * globalScale, y + 2 * globalScale}, {x, y + 14 * globalScale}, DARKGRAY);
		
		if (frame == 0)
		{
			DrawRectangle(x + 10 * globalScale, y + 20 * globalScale, 6 * globalScale, 5 * globalScale, DARKGRAY);
			DrawRectangle(x + 40 * globalScale, y + 20 * globalScale, 6 * globalScale, 3 * globalScale, DARKGRAY);
		}
		else
		{
			DrawRectangle(x + 10 * globalScale, y + 20 * globalScale, 6 * globalScale, 3 * globalScale, DARKGRAY);
			DrawRectangle(x + 40 * globalScale, y + 20 * globalScale, 6 * globalScale, 5 * globalScale, DARKGRAY);
		}
	}
};

class Obstacle
{
public:
	float speed;
	float groundY;
	Rectangle rect;
	bool isBird;
	float birdHeight;
	
	Obstacle(float x, float groundY, float speed, int score) : speed(speed), groundY(groundY)
	{
		isBird = (score > 300) && (rand() % 5 == 0);  // 翼龙出现更晚、概率更低
		if (isBird)
		{
			float width = 30.0f * globalScale;
			float height = 20.0f * globalScale;
			int heightLevel = rand() % 3;
			switch(heightLevel)
			{
				case 0: birdHeight = 40.0f * globalScale; break;
				case 1: birdHeight = 20.0f * globalScale; break;
				case 2: birdHeight = 0.0f; break;
			}
			rect = {x, groundY - height - birdHeight, width, height};
		}
		else
		{
			float minHeight = 30.0f * globalScale;
			float maxHeight = 44.0f * globalScale;  // 降低仙人掌最大高度
			float height = minHeight + (rand() % (int)(maxHeight - minHeight));
			float width = 16.0f * globalScale;
			rect = {x, groundY - height, width, height};
		}
	}
	
	void Update(float deltaTime)
	{
		rect.x -= speed * 60.0f * deltaTime;
	}
	
	void Draw()
	{
		if (isBird)
			DrawBird();
		else
			DrawCactus();
	}
	
	bool IsOffScreen()
	{
		return rect.x + rect.width < 0;
	}
	
private:
	void DrawCactus()
	{
		Color cactusColor = {46, 125, 50, 255};
		Color darkCactus = {27, 94, 32, 255};
		float g = globalScale;
		DrawRectangle(rect.x + 4 * g, rect.y, rect.width - 8 * g, rect.height, cactusColor);
		DrawRectangle(rect.x + 4 * g, rect.y, 3 * g, rect.height, darkCactus);
		float armY = rect.y + rect.height * 0.3f;
		DrawRectangle(rect.x - 4 * g, armY, 8 * g, 6 * g, cactusColor);
		DrawRectangle(rect.x - 4 * g, armY - 8 * g, 6 * g, 8 * g, cactusColor);
		float arm2Y = rect.y + rect.height * 0.5f;
		DrawRectangle(rect.x + rect.width - 4 * g, arm2Y, 8 * g, 6 * g, cactusColor);
		DrawRectangle(rect.x + rect.width - 2 * g, arm2Y - 10 * g, 6 * g, 10 * g, cactusColor);
		for (int i = 0; i < 3; i++)
		{
			float spikeY = rect.y + rect.height * 0.2f + i * rect.height * 0.25f;
			DrawLineEx({rect.x + 2 * g, spikeY}, {rect.x - 2 * g, spikeY - 2 * g}, g, darkCactus);
			DrawLineEx({rect.x + rect.width - 2 * g, spikeY + 2 * g}, {rect.x + rect.width + 2 * g, spikeY}, g, darkCactus);
		}
	}
	
	void DrawBird()
	{
		int wingFrame = ((int)(GetTime() * 15) % 2);
		Color birdColor = {66, 66, 66, 255};
		float g = globalScale;
		float x = rect.x;
		float y = rect.y;
		float w = rect.width;
		float h = rect.height;
		DrawEllipse(x + w/2, y + h/2, w/2, h/2, birdColor);
		DrawCircle(x + w - 5 * g, y + 3 * g, 5 * g, birdColor);
		DrawCircle(x + w - 3 * g, y + 1 * g, 2 * g, WHITE);
		DrawCircle(x + w - 2.5f * g, y + 1 * g, 1 * g, BLACK);
		DrawTriangle({x + w + 2 * g, y + 3 * g}, {x + w + 8 * g, y + 4 * g}, {x + w + 2 * g, y + 5 * g}, {255, 165, 0, 255});
		if (wingFrame == 0)
		{
			DrawTriangle({x + w/2, y}, {x + w/2 - 10 * g, y - 12 * g}, {x + w/2 + 10 * g, y - 12 * g}, birdColor);
		}
		else
		{
			DrawTriangle({x + w/2, y + h}, {x + w/2 - 10 * g, y + h + 12 * g}, {x + w/2 + 10 * g, y + h + 12 * g}, birdColor);
		}
	}
};

class Cloud
{
public:
	Rectangle rect;
	float speed;
	
	Cloud(float screenWidth, float screenHeight)
	{
		float cloudWidth = 60.0f * globalScale;
		float cloudHeight = 30.0f * globalScale;
		rect = {screenWidth + (float)(rand() % (int)(screenWidth * 0.5f)), 50.0f * globalScale + (float)(rand() % (int)(screenHeight * 0.25f)), cloudWidth, cloudHeight};
		speed = 1.0f + (float)(rand() % 200) / 100.0f;
	}
	
	void Update(float deltaTime, float screenWidth, float screenHeight)
	{
		rect.x -= speed * globalScale * 60.0f * deltaTime;
		if (rect.x + rect.width < 0)
		{
			rect.x = screenWidth + (float)(rand() % (int)(screenWidth * 0.25f));
			rect.y = 50.0f * globalScale + (float)(rand() % (int)(screenHeight * 0.25f));
		}
	}
	
	void Draw()
	{
		float radius1 = rect.height * 0.5f;
		float radius2 = rect.height * 0.67f;
		DrawCircle(rect.x + rect.width * 0.33f, rect.y + rect.height * 0.5f, radius1, LIGHTGRAY);
		DrawCircle(rect.x + rect.width * 0.58f, rect.y + rect.height * 0.33f, radius2, LIGHTGRAY);
		DrawCircle(rect.x + rect.width * 0.83f, rect.y + rect.height * 0.5f, radius1, LIGHTGRAY);
	}
};

void SwitchFullscreen()
{
	if (IsWindowFullscreen())
	{
		ToggleFullscreen();
		fullscreen = false;
		currentWidth = BASE_WIDTH * 2;
		currentHeight = BASE_HEIGHT * 2;
		SetWindowSize(currentWidth, currentHeight);
	}
	else
	{
		int monitor = GetCurrentMonitor();
		currentWidth = GetMonitorWidth(monitor);
		currentHeight = GetMonitorHeight(monitor);
		ToggleFullscreen();
		fullscreen = true;
	}
	scaleX = (float)currentWidth / BASE_WIDTH;
	scaleY = (float)currentHeight / BASE_HEIGHT;
	globalScale = std::max(scaleX, scaleY);
}

void DrawGround(float groundY, float deltaTime)
{
	float g = globalScale;
	DrawLineEx({0, groundY}, {(float)currentWidth, groundY}, 3.0f * g, DARKGRAY);
	DrawRectangle(0, groundY, currentWidth, currentHeight - groundY, {247, 247, 247, 255});
	static float offset = 0;
	offset -= 3.0f * g * 60.0f * deltaTime;
	if (offset < -40.0f * g) offset += 40.0f * g;
	for (float i = offset; i < currentWidth; i += 40.0f * g)
	{
		float lineY = groundY + 12.0f * g;
		DrawLineEx({i, lineY}, {i + 20.0f * g, lineY}, 2.0f * g, {200, 200, 200, 255});
	}
}

int main()
{
	int n;
	cout << "FPS:";
	cin >> n;
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	currentWidth = BASE_WIDTH * 2;
	currentHeight = BASE_HEIGHT * 2;
	InitWindow(currentWidth, currentHeight, "Chrome Dino Game");
	SetTargetFPS(n);
	srand((unsigned int)time(NULL));
	
	scaleX = (float)currentWidth / BASE_WIDTH;
	scaleY = (float)currentHeight / BASE_HEIGHT;
	globalScale = std::max(scaleX, scaleY);
	float groundY = currentHeight * GROUND_Y_RATIO;
	
	Dinosaur dino(groundY);
	std::vector<Obstacle> obstacles;
	std::vector<Cloud> clouds;
	for (int i = 0; i < 6; i++)
	{
		clouds.push_back(Cloud((float)currentWidth, (float)currentHeight));
	}
	
	int score = 0;
	int highScore = 0;
	std::ifstream infile("highscore.dat");
	if (infile.is_open())
	{
		infile >> highScore;
		infile.close();
	}
	
	bool gameOver = false;
	bool gameStarted = false;
	float obstacleTimer = 0;
	float baseObstacleInterval = 1.8f;    // 延长初始间隔
	float obstacleInterval = baseObstacleInterval;
	float baseSpeed = 8.0f;
	
	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		
		if (IsWindowResized() && !IsWindowFullscreen())
		{
			currentWidth = GetScreenWidth();
			currentHeight = GetScreenHeight();
			scaleX = (float)currentWidth / BASE_WIDTH;
			scaleY = (float)currentHeight / BASE_HEIGHT;
			globalScale = std::max(scaleX, scaleY);
			groundY = currentHeight * GROUND_Y_RATIO;
			dino.groundY = groundY;
			if (!dino.isJumping)
			{
				dino.rect.y = groundY - dino.rect.height;
			}
			clouds.clear();
			for (int i = 0; i < 6; i++)
			{
				clouds.push_back(Cloud((float)currentWidth, (float)currentHeight));
			}
		}
		
		if (IsKeyPressed(KEY_F11))
		{
			SwitchFullscreen();
			groundY = currentHeight * GROUND_Y_RATIO;
			dino.groundY = groundY;
			if (!dino.isJumping)
			{
				dino.rect.y = groundY - dino.rect.height;
			}
			clouds.clear();
			for (int i = 0; i < 6; i++)
			{
				clouds.push_back(Cloud((float)currentWidth, (float)currentHeight));
			}
		}
		
		bool jumpPressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
		bool duckHeld = IsKeyDown(KEY_DOWN);
		
		if (!gameStarted && !gameOver)
		{
			if (jumpPressed)
			{
				gameStarted = true;
				dino.Jump();
			}
		}
		
		if (gameStarted && !gameOver)
		{
			if (jumpPressed)
			{
				dino.Jump();
			}
			dino.Duck(duckHeld);
		}
		
		if (gameOver)
		{
			if (jumpPressed)
			{
				dino = Dinosaur(groundY);
				obstacles.clear();
				score = 0;
				gameOver = false;
				gameStarted = true;
				obstacleTimer = 0;
				dino.Jump();
			}
		}
		
		if (gameStarted && !gameOver)
		{
			dino.Update(dt);
			
			obstacleTimer += dt;
			float currentSpeed = (baseSpeed + score * 0.01f) * globalScale; // 缓慢加速
			if (obstacleTimer >= obstacleInterval)
			{
				obstacles.push_back(Obstacle((float)currentWidth, groundY, currentSpeed, score));
				obstacleTimer = 0;
				float minInterval = 0.6f;   // 最小间隔不小于0.6秒
				obstacleInterval = std::max(minInterval, 1.8f - score * 0.0015f);
			}
			
			for (auto& obs : obstacles)
			{
				obs.Update(dt);
			}
			
			obstacles.erase(
							std::remove_if(obstacles.begin(), obstacles.end(),
										   [&score](Obstacle& obs)
										   {
											   if (obs.IsOffScreen())
											   {
												   score += 10;
												   return true;
											   }
											   return false;
										   }),
										   obstacles.end()
										   );
			
			Rectangle dinoCollision = dino.rect;
			for (const auto& obs : obstacles)
			{
				Rectangle obsCollision = obs.rect;
				if (CheckCollisionRecs(dinoCollision, obsCollision))
				{
					gameOver = true;
					if (score > highScore)
					{
						highScore = score;
						std::ofstream outfile("highscore.dat");
						if (outfile.is_open())
						{
							outfile << highScore;
							outfile.close();
						}
					}
				}
			}
			
			for (auto& cloud : clouds)
			{
				cloud.Update(dt, (float)currentWidth, (float)currentHeight);
			}
		}
		
		BeginDrawing();
		ClearBackground(RAYWHITE);
		
		// 网格背景
		for (int i = 0; i < currentWidth; i += (int)(40 * globalScale))
		{
			for (int j = 0; j < (int)groundY; j += (int)(40 * globalScale))
			{
				DrawPixel(i, j, {245, 245, 245, 255});
			}
		}
		
		for (auto& cloud : clouds)
		{
			cloud.Draw();
		}
		
		DrawGround(groundY, dt);
		
		for (auto& obs : obstacles)
		{
			obs.Draw();
		}
		
		dino.Draw();
		
		int fontSize = (int)(24.0f * globalScale);
		int largeFontSize = (int)(36.0f * globalScale);
		
		if (!gameStarted)
		{
			const char* startText = "Press SPACE or UP to Start";
			int textWidth = MeasureText(startText, fontSize);
			DrawText(startText, currentWidth/2 - textWidth/2, currentHeight/2 - fontSize * 2, fontSize, DARKGRAY);
		}
		
		const char* scoreText = TextFormat("%05d", score);
		const char* hiText = TextFormat("HI %05d", highScore);
		int hiWidth = MeasureText(hiText, fontSize);
		int scoreWidth = MeasureText(scoreText, fontSize);
		int maxWidth = std::max(hiWidth, scoreWidth);
		DrawText(hiText, currentWidth - maxWidth - (int)(30 * globalScale), (int)(30 * globalScale), fontSize, {128, 128, 128, 255});
		DrawText(scoreText, currentWidth - maxWidth - (int)(30 * globalScale), (int)(30 * globalScale) + fontSize + (int)(8 * globalScale), fontSize, DARKGRAY);
		
		if (gameOver)
		{
			const char* gameOverText = "G A M E  O V E R";
			int goWidth = MeasureText(gameOverText, largeFontSize);
			DrawText(gameOverText, currentWidth/2 - goWidth/2, currentHeight/2 - largeFontSize, largeFontSize, {83, 83, 83, 255});
			const char* restartText = "Press SPACE to Restart";
			int restartWidth = MeasureText(restartText, fontSize);
			DrawText(restartText, currentWidth/2 - restartWidth/2, currentHeight/2 + (int)(10 * globalScale), fontSize, {128, 128, 128, 255});
		}
		
		EndDrawing();
	}
	
	std::ofstream outfile("highscore.dat");
	if (outfile.is_open())
	{
		outfile << highScore;
		outfile.close();
	}
	
	CloseWindow();
	return 0;
}
