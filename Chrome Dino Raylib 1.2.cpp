#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <cmath>
#include <fstream>
using namespace std;

// ==================== 基准分辨率与窗口缩放 ====================
const int BASE_WIDTH = 800;
const int BASE_HEIGHT = 400;
const float GROUND_Y_RATIO = 0.875f;

float scaleX = 1.0f, scaleY = 1.0f, globalScale = 1.0f;
int currentWidth = BASE_WIDTH, currentHeight = BASE_HEIGHT;
bool fullscreen = false;

// ==================== 原版参数（Chromium dino_game，600x150 -> 800x400） ====================
// 速度等单位沿用原版 600x150 画布坐标系，渲染时乘 XS/YS/globalScale 转换到屏幕像素
const float XS = 4.0f / 3.0f;            // 横向缩放
const float YS = 8.0f / 3.0f;            // 纵向缩放
const float SPR = 1.5f;                  // 精灵尺寸缩放
const float SPEED_INIT = 6.0f;           // 初始速度（px/帧 @60fps）
const float SPEED_MAX = 13.0f;           // 最大速度
const float ACCELERATION = 0.001f;       // 每帧加速度
const float GRAVITY = 0.6f * YS;         // 重力（px/帧^2）
const float JUMP_VELOCITY = 10.0f * YS;  // 跳跃初速度基数（=10 + speed/10）
const float SPEED_DROP_COEFFICIENT = 3.0f;
const float DROP_VELOCITY = 5.0f * YS;
const float SCORE_COEFFICIENT = 0.025f;  // 像素距离 -> 分数
const int INVERT_DISTANCE = 700;         // 每 700 分触发一次夜间模式
const float INVERT_FADE_DURATION = 12.0f; // 夜间持续 12 秒
const float GAP_COEFFICIENT = 0.6f;      // 障碍物间距系数
const int MAX_OBSTACLE_LENGTH = 3;       // 障碍物最多连续 3 个
const int MAX_OBSTACLE_DUPLICATION = 2;  // 同类型最多连续 2 组
const float PTERO_MIN_SPEED = 8.5f;      // 翼龙最低出现速度
const float PTERO_SPEED_OFFSET = 0.8f;   // 翼龙速度偏移
const float PTERO_FLAP_RATE = 1000.0f / 6.0f; // 翼龙扇翅间隔（毫秒）
const float NIGHT_FADE_SPEED = 0.035f * 60.0f; // 夜间渐入渐出（每秒）

float nightAlpha = 0.0f;   // 夜间混合系数 0..1

struct CollisionBox { float x, y, w, h; };

Color LerpColor(Color a, Color b, float t)
{
	return {
		(unsigned char)(a.r + (b.r - a.r) * t),
		(unsigned char)(a.g + (b.g - a.g) * t),
		(unsigned char)(a.b + (b.b - a.b) * t),
		255 };
}

// 日间/夜间颜色自动混合（原版夜间模式会整体反色）
Color DN(Color day, Color night) { return LerpColor(day, night, nightAlpha); }

// ==================== 恐龙（原版 Trex 物理与碰撞盒） ====================
class Dinosaur
{
public:
	Rectangle rect;
	float velocityY;
	bool isJumping, isDucking, speedDrop, reachedApex, blinking, waiting;
	float groundY;
	float normalWidth, normalHeight, duckWidth, duckHeight;
	float animTimer, blinkTimer, blinkDelay;
	vector<CollisionBox> runBoxes, duckBoxes;

	Dinosaur(float groundY) : groundY(groundY)
	{
		normalWidth = 44.0f * SPR;
		normalHeight = 47.0f * SPR;
		duckWidth = 59.0f * SPR;
		duckHeight = 25.0f * SPR;   // 原版下蹲高度 25px
		rect = {120.0f, groundY - normalHeight, normalWidth, normalHeight};
		velocityY = 0;
		isJumping = false;
		isDucking = false;
		speedDrop = false;
		reachedApex = false;
		blinking = false;
		waiting = true;
		animTimer = 0;
		blinkTimer = 0;
		blinkDelay = 1.0f + (rand() % 7000) / 1000.0f;
		// 原版碰撞盒（相对精灵左上角，x1.5）
		runBoxes = {
			{33, 0, 25.5f, 24},      // 头部
			{1.5f, 27, 45, 13.5f},   // 身体上
			{15, 52.5f, 21, 12},     // 身体下
			{1.5f, 36, 43.5f, 7.5f}, // 腹部
			{7.5f, 45, 31.5f, 6},    // 腿
			{13.5f, 51, 22.5f, 6},   // 脚
		};
		duckBoxes = { {1.5f, 27, 82.5f, 37.5f} };  // 下蹲（可躲开所有翼龙）
	}

	void Update(float dt, float speed, bool duckHeld)
	{
		animTimer += dt;
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
			float frames = 60.0f * dt;
			float g = GRAVITY * scaleY;
			// 按住下键速降：下落速度 x3（原版 speedDrop）
			if (speedDrop)
				rect.y += velocityY * SPEED_DROP_COEFFICIENT * frames;
			else
				rect.y += velocityY * frames;
			velocityY += g * frames;
			if (rect.y < groundY - 30.0f * YS * scaleY)
				reachedApex = true;
			if (rect.y >= groundY - rect.height)
			{
				rect.y = groundY - rect.height;
				velocityY = 0;
				isJumping = false;
				speedDrop = false;
				reachedApex = false;
			}
		}

		if (waiting)
		{
			blinkTimer += dt;
			if (blinking)
			{
				if (blinkTimer >= 0.1f)
				{
					blinking = false;
					blinkTimer = 0;
					blinkDelay = 1.0f + (rand() % 7000) / 1000.0f;
				}
			}
			else if (blinkTimer >= blinkDelay)
			{
				blinking = true;
				blinkTimer = 0;
			}
		}
	}

	void Jump(float speed)
	{
		if (!isJumping && !isDucking)
		{
			waiting = false;
			// 原版：初速度 = -(10 + speed/10)
			velocityY = -(JUMP_VELOCITY + (speed / 10.0f) * YS) * scaleY;
			isJumping = true;
		}
	}

	void EndJump()
	{
		// 原版 endJump：越过最高点且仍快速上升时钳制速度
		if (reachedApex && velocityY < -DROP_VELOCITY * scaleY)
			velocityY = -DROP_VELOCITY * scaleY;
	}

	void Duck(bool duck)
	{
		if (duck && !isJumping)
			isDucking = true;
		else
			isDucking = false;
	}

	bool Hits(const vector<CollisionBox>& obsBoxes, Rectangle obsRect)
	{
		const vector<CollisionBox>& boxes = isDucking ? duckBoxes : runBoxes;
		for (const auto& a : boxes)
		{
			Rectangle ra = {
				rect.x + a.x * globalScale,
				rect.y + a.y * globalScale,
				a.w * globalScale,
				a.h * globalScale };
			for (const auto& b : obsBoxes)
			{
				Rectangle rb = { obsRect.x + b.x, obsRect.y + b.y, b.w, b.h };
				if (CheckCollisionRecs(ra, rb))
					return true;
			}
		}
		return false;
	}

	void Draw()
	{
		if (isDucking)
			DrawDucking();
		else if (isJumping)
			DrawJumping();
		else
			DrawRunning();
	}

private:
	Color Body() { return DN(DARKGRAY, {175, 175, 175, 255}); }

	void DrawEye(float x, float y)
	{
		if (blinking)
		{
			DrawLineEx({x - 3 * globalScale, y}, {x + 3 * globalScale, y}, 1.5f * globalScale, Body());
		}
		else
		{
			DrawCircle(x, y, 3 * globalScale, WHITE);
			DrawCircle(x + 1 * globalScale, y, 1.5f * globalScale, BLACK);
			DrawCircle(x + 3 * globalScale, y + 2 * globalScale, 1 * globalScale, BLACK);
		}
	}

	void DrawBody(float x, float y)
	{
		Color c = Body();
		DrawRectangle(x + 8 * globalScale, y + 4 * globalScale, 30 * globalScale, 30 * globalScale, c);
		DrawRectangle(x + 4 * globalScale, y + 8 * globalScale, 36 * globalScale, 26 * globalScale, c);
		DrawRectangle(x + 28 * globalScale, y - 4 * globalScale, 16 * globalScale, 16 * globalScale, c);
		DrawRectangle(x + 32 * globalScale, y - 8 * globalScale, 12 * globalScale, 12 * globalScale, c);
		DrawRectangle(x + 20 * globalScale, y + 20 * globalScale, 4 * globalScale, 12 * globalScale, c);
		DrawEye(x + 40 * globalScale, y - 2 * globalScale);
	}

	void DrawRunning()
	{
		// 原版奔跑动画 2 帧，1000/12 = 83ms
		int frame = ((int)(animTimer * 12.0f)) % 2;
		float x = rect.x, y = rect.y;
		Color c = Body();
		DrawBody(x, y);
		DrawRectangle(x + 8 * globalScale, y + 44 * globalScale, 28 * globalScale, 3 * globalScale, c); // 尾巴
		if (frame == 0)
		{
			DrawTriangle({x + 4 * globalScale, y + 12 * globalScale}, {x - 6 * globalScale, y + 4 * globalScale}, {x - 2 * globalScale, y + 16 * globalScale}, c);
			DrawRectangle(x + 10 * globalScale, y + 34 * globalScale, 8 * globalScale, 13 * globalScale, c);
			DrawRectangle(x + 26 * globalScale, y + 34 * globalScale, 8 * globalScale, 8 * globalScale, c);
		}
		else
		{
			DrawTriangle({x + 4 * globalScale, y + 10 * globalScale}, {x - 8 * globalScale, y + 6 * globalScale}, {x - 4 * globalScale, y + 18 * globalScale}, c);
			DrawRectangle(x + 10 * globalScale, y + 34 * globalScale, 8 * globalScale, 8 * globalScale, c);
			DrawRectangle(x + 26 * globalScale, y + 34 * globalScale, 8 * globalScale, 13 * globalScale, c);
		}
	}

	void DrawJumping()
	{
		float x = rect.x, y = rect.y;
		Color c = Body();
		DrawBody(x, y);
		DrawTriangle({x + 4 * globalScale, y + 12 * globalScale}, {x - 6 * globalScale, y + 6 * globalScale}, {x - 2 * globalScale, y + 16 * globalScale}, c);
		DrawRectangle(x + 10 * globalScale, y + 34 * globalScale, 8 * globalScale, 6 * globalScale, c);
		DrawRectangle(x + 26 * globalScale, y + 34 * globalScale, 8 * globalScale, 6 * globalScale, c);
		DrawRectangle(x + 8 * globalScale, y + 40 * globalScale, 28 * globalScale, 3 * globalScale, c);
	}

	void DrawDucking()
	{
		// 原版下蹲动画 2 帧，1000/8 = 125ms
		int frame = ((int)(animTimer * 8.0f)) % 2;
		float x = rect.x, y = rect.y;
		Color c = Body();
		DrawRectangle(x + 4 * globalScale, y + 2 * globalScale, 50 * globalScale, 18 * globalScale, c);
		DrawRectangle(x + 44 * globalScale, y - 2 * globalScale, 14 * globalScale, 14 * globalScale, c);
		DrawEye(x + 54 * globalScale, y + 2 * globalScale);
		DrawTriangle({x + 4 * globalScale, y + 6 * globalScale}, {x - 4 * globalScale, y + 2 * globalScale}, {x, y + 14 * globalScale}, c);
		if (frame == 0)
		{
			DrawRectangle(x + 10 * globalScale, y + 20 * globalScale, 6 * globalScale, 5 * globalScale, c);
			DrawRectangle(x + 40 * globalScale, y + 20 * globalScale, 6 * globalScale, 3 * globalScale, c);
		}
		else
		{
			DrawRectangle(x + 10 * globalScale, y + 20 * globalScale, 6 * globalScale, 3 * globalScale, c);
			DrawRectangle(x + 40 * globalScale, y + 20 * globalScale, 6 * globalScale, 5 * globalScale, c);
		}
	}
};

// ==================== 障碍物（原版类型/尺寸/碰撞盒/间距） ====================
enum ObstacleType { CACTUS_SMALL, CACTUS_LARGE, PTERODACTYL };

class Obstacle
{
public:
	ObstacleType type;
	int size;
	float x, y;             // 屏幕坐标（左上角）
	float width, height;    // 屏幕尺寸（整组）
	float speedOffset;
	float gap;
	bool nextSpawned;
	float flapTimer;
	int flapFrame;
	vector<CollisionBox> boxes;

	Obstacle(ObstacleType t, float xStart, float groundY, float currentSpeed, int s, int level)
		: type(t), size(s), speedOffset(0), nextSpawned(false), flapTimer(0), flapFrame(0)
	{
		float uw, uh, minGapConst;
		if (t == CACTUS_SMALL)      { uw = 17 * SPR; uh = 35 * SPR; minGapConst = 120 * SPR; }
		else if (t == CACTUS_LARGE) { uw = 25 * SPR; uh = 50 * SPR; minGapConst = 120 * SPR; }
		else                        { uw = 46 * SPR; uh = 40 * SPR; minGapConst = 150 * SPR; }

		width = uw * size * globalScale;
		height = uh * globalScale;
		x = xStart;
		if (t == PTERODACTYL)
			y = groundY - (float)level * 25.0f * SPR * globalScale - height;  // 三档高度：贴地/中/高
		else
			y = groundY - height;

		if (t == PTERODACTYL)
			speedOffset = (rand() % 2 == 0 ? 1.0f : -1.0f) * PTERO_SPEED_OFFSET;

		// 原版碰撞盒（相对精灵左上角，x1.5）
		vector<CollisionBox> base;
		if (t == CACTUS_SMALL)
			base = { {0, 7, 5, 27}, {4, 0, 6, 34}, {10, 4, 7, 14} };
		else if (t == CACTUS_LARGE)
			base = { {0, 12, 7, 38}, {8, 0, 7, 49}, {13, 10, 10, 38} };
		else
			base = { {15, 15, 16, 5}, {18, 21, 24, 6}, {2, 14, 4, 3}, {6, 10, 4, 7}, {10, 8, 6, 9} };
		for (const auto& b : base)
			boxes.push_back({ b.x * SPR * globalScale, b.y * SPR * globalScale, b.w * SPR * globalScale, b.h * SPR * globalScale });

		// 连续多个时拉长中间碰撞盒（原版逻辑）
		if (size > 1)
		{
			boxes[1].w = width - boxes[0].w - boxes[2].w;
			boxes[2].x = width - boxes[2].w;
		}

		// 原版间距公式：minGap = width*speed + minGap*系数，maxGap = 1.5x，随机取
		float minGap = roundf(width * currentSpeed * XS * globalScale + minGapConst * GAP_COEFFICIENT);
		float maxGap = minGap * 1.5f;
		gap = minGap + (float)(rand() % (int)(maxGap - minGap + 1.0f));
	}

	void Update(float dt, float speed)
	{
		x -= (speed + speedOffset) * XS * globalScale * 60.0f * dt;
		if (type == PTERODACTYL)
		{
			flapTimer += dt * 1000.0f;
			if (flapTimer >= PTERO_FLAP_RATE)
			{
				flapTimer = 0;
				flapFrame = (flapFrame + 1) % 2;
			}
		}
	}

	bool IsOffScreen() { return x + width < 0; }
	bool IsVisible() { return x + width > 0; }

	void Draw()
	{
		if (type == PTERODACTYL)
			DrawBird();
		else
			DrawCactus();
	}

private:
	void DrawCactus()
	{
		Color cactusColor = DN({46, 125, 50, 255}, {209, 130, 205, 255});
		Color darkCactus = DN({27, 94, 32, 255}, {228, 161, 223, 255});
		float g = globalScale;
		DrawRectangle(x + 4 * g, y, width - 8 * g, height, cactusColor);
		DrawRectangle(x + 4 * g, y, 3 * g, height, darkCactus);
		// 小型仙人掌只有左臂，大型左右双臂（原版造型）
		float armY = y + height * 0.3f;
		DrawRectangle(x - 4 * g, armY, 8 * g, 6 * g, cactusColor);
		DrawRectangle(x - 4 * g, armY - 8 * g, 6 * g, 8 * g, cactusColor);
		if (type == CACTUS_LARGE)
		{
			float arm2Y = y + height * 0.5f;
			DrawRectangle(x + width - 4 * g, arm2Y, 8 * g, 6 * g, cactusColor);
			DrawRectangle(x + width - 2 * g, arm2Y - 10 * g, 6 * g, 10 * g, cactusColor);
		}
		for (int i = 0; i < 3; i++)
		{
			float spikeY = y + height * 0.2f + i * height * 0.25f;
			DrawLineEx({x + 2 * g, spikeY}, {x - 2 * g, spikeY - 2 * g}, g, darkCactus);
			DrawLineEx({x + width - 2 * g, spikeY + 2 * g}, {x + width + 2 * g, spikeY}, g, darkCactus);
		}
	}

	void DrawBird()
	{
		Color birdColor = DN({66, 66, 66, 255}, {189, 189, 189, 255});
		float g = globalScale;
		float w = width, h = height;
		DrawEllipse(x + w / 2, y + h / 2, w / 2, h / 2, birdColor);
		DrawCircle(x + w - 5 * g, y + 3 * g, 5 * g, birdColor);
		DrawCircle(x + w - 3 * g, y + 1 * g, 2 * g, WHITE);
		DrawCircle(x + w - 2.5f * g, y + 1 * g, 1 * g, BLACK);
		DrawTriangle({x + w + 2 * g, y + 3 * g}, {x + w + 8 * g, y + 4 * g}, {x + w + 2 * g, y + 5 * g}, birdColor);
		// 原版双帧扇翅动画（1000/6 ms）
		if (flapFrame == 0)
			DrawTriangle({x + w / 2, y}, {x + w / 2 - 10 * g, y - 12 * g}, {x + w / 2 + 10 * g, y - 12 * g}, birdColor);
		else
			DrawTriangle({x + w / 2, y + h}, {x + w / 2 - 10 * g, y + h + 12 * g}, {x + w / 2 + 10 * g, y + h + 12 * g}, birdColor);
	}
};

// ==================== 云（原版位置/间距/速度） ====================
class Cloud
{
public:
	float x, y, width, height, gap;

	Cloud(float screenWidth)
	{
		width = 46.0f * XS * globalScale;
		height = 14.0f * YS * globalScale;
		y = (30.0f + (float)(rand() % 42)) * YS * globalScale;  // 原版天空带 30..71
		x = screenWidth;
		gap = (100.0f + (float)(rand() % 301)) * XS * globalScale;  // 原版 100..400
	}

	void Update(float pxPerFrame, float dt)
	{
		x -= pxPerFrame * 60.0f * dt;
	}

	bool IsOffScreen() { return x + width < 0; }

	void Draw()
	{
		Color c = DN(LIGHTGRAY, {55, 55, 55, 255});
		float radius1 = height * 0.5f;
		float radius2 = height * 0.67f;
		DrawCircle(x + width * 0.33f, y + height * 0.5f, radius1, c);
		DrawCircle(x + width * 0.58f, y + height * 0.33f, radius2, c);
		DrawCircle(x + width * 0.83f, y + height * 0.5f, radius1, c);
	}
};

// ==================== 工具函数 ====================
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

void DrawGround(float groundY, float dt, float speed)
{
	float g = globalScale;
	DrawLineEx({0, groundY}, {(float)currentWidth, groundY}, 3.0f * g, DN(DARKGRAY, {175, 175, 175, 255}));
	static float offset = 0;
	offset -= speed * XS * g * 60.0f * dt;
	if (offset < -40.0f * g) offset += 40.0f * g;
	Color dashC = DN({200, 200, 200, 255}, {55, 55, 55, 255});
	float lineY = groundY + 12.0f * YS * g;
	for (float i = offset; i < currentWidth; i += 40.0f * g)
	{
		DrawLineEx({i, lineY}, {i + 20.0f * g, lineY}, 2.0f * g, dashC);
	}
}

void SpawnObstacle(vector<Obstacle>& obstacles, vector<ObstacleType>& history, float speed, float groundY)
{
	ObstacleType type = CACTUS_SMALL;
	for (int tries = 0; tries < 10; tries++)
	{
		ObstacleType t = (ObstacleType)(rand() % 3);
		if (t == PTERODACTYL && speed < PTERO_MIN_SPEED) continue;             // 翼龙需要速度 >= 8.5
		if (history.size() >= 2 && history[0] == t && history[1] == t) continue; // 同类型最多连续 2 组
		type = t;
		break;
	}
	int size = 1 + rand() % MAX_OBSTACLE_LENGTH;
	float multSpeed = (type == PTERODACTYL) ? 999.0f : (type == CACTUS_LARGE ? 7.0f : 4.0f);
	if (size > 1 && multSpeed > speed) size = 1;
	int level = rand() % 3;
	obstacles.push_back(Obstacle(type, (float)currentWidth, groundY, speed, size, level));
	history.insert(history.begin(), type);
	if (history.size() > MAX_OBSTACLE_DUPLICATION) history.pop_back();
}

void UpdateClouds(vector<Cloud>& clouds, float speed, float dt)
{
	// 原版：ceil(0.2/1000 * dt_ms * speed) px/帧，再换算到屏幕
	float es = ceilf(0.2f / 1000.0f * dt * 1000.0f * speed) * XS * globalScale;
	for (auto& c : clouds)
		c.Update(es, dt);
	clouds.erase(std::remove_if(clouds.begin(), clouds.end(),
					[](Cloud& c) { return c.IsOffScreen(); }),
				clouds.end());
	if (!clouds.empty() && clouds.size() < 6)
	{
		Cloud& last = clouds.back();
		if (currentWidth - last.x > last.gap && (rand() % 100) < 50)
			clouds.push_back(Cloud((float)currentWidth));
	}
}

int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	currentWidth = BASE_WIDTH * 2;
	currentHeight = BASE_HEIGHT * 2;
	InitWindow(currentWidth, currentHeight, "Chrome Dino Game v1.2");

	// 自适应显示器刷新率（v1.1 起）
	int fps = GetMonitorRefreshRate(GetCurrentMonitor());
	if (fps <= 0 || fps > 500) fps = 60;
	SetTargetFPS(fps);

	srand((unsigned int)time(NULL));

	scaleX = (float)currentWidth / BASE_WIDTH;
	scaleY = (float)currentHeight / BASE_HEIGHT;
	globalScale = std::max(scaleX, scaleY);
	float groundY = currentHeight * GROUND_Y_RATIO;

	Dinosaur dino(groundY);
	vector<Obstacle> obstacles;
	vector<ObstacleType> history;
	vector<Cloud> clouds;
	clouds.push_back(Cloud((float)currentWidth));

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
	float speed = SPEED_INIT;
	float distanceRan = 0;
	float restartLock = 0;
	int lastHundred = 0;
	float flashTimer = 0;

	// 夜间模式（原版：每 700 分触发一次，持续 12 秒）
	bool nightMode = false;
	float nightTimer = 0;
	float moonX = 0;
	int moonPhase = 0;
	struct Star { float x, y; };
	Star stars[2];
	float starPhaseOffsets[7] = { 140, 120, 100, 60, 40, 20, 0 }; // 月相
	for (int i = 0; i < 2; i++)
	{
		int seg = currentWidth / 2;
		stars[i].x = (float)(seg * i + rand() % (seg + 1));
		stars[i].y = (float)(rand() % (int)(currentHeight * 0.47f));
	}

	// 开场动画：恐龙从左滑入（原版 introDuration 1.5 秒）
	float introX = -dino.rect.width;
	bool introDone = false;

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
		}

		if (IsKeyPressed(KEY_F11))
		{
			SwitchFullscreen();
			int refresh = GetMonitorRefreshRate(GetCurrentMonitor());
			if (refresh > 0 && refresh <= 500) SetTargetFPS(refresh);
			groundY = currentHeight * GROUND_Y_RATIO;
			dino.groundY = groundY;
			if (!dino.isJumping)
			{
				dino.rect.y = groundY - dino.rect.height;
			}
		}

		bool jumpPressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
		bool jumpReleased = IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_UP);
		bool duckHeld = IsKeyDown(KEY_DOWN);

		// 开场滑入
		if (!introDone)
		{
			introX += ((120.0f + dino.rect.width) / 1.5f) * dt;
			if (introX >= 120.0f)
			{
				introX = 120.0f;
				introDone = true;
			}
			dino.rect.x = introX;
		}

		// 开始 / 重新开始（原版 0.75 秒后允许重开）
		if (!gameStarted && !gameOver && introDone)
		{
			if (jumpPressed)
			{
				gameStarted = true;
				dino.Jump(speed);
			}
		}
		if (gameOver)
		{
			restartLock += dt;
			if (jumpPressed && restartLock >= 0.75f)
			{
				dino = Dinosaur(groundY);
				obstacles.clear();
				history.clear();
				score = 0;
				distanceRan = 0;
				speed = SPEED_INIT;
				gameOver = false;
				gameStarted = true;
				nightMode = false;
				nightTimer = 0;
				lastHundred = 0;
				flashTimer = 0;
				restartLock = 0;
				dino.Jump(speed);
			}
		}

		if (gameStarted && !gameOver)
		{
			dino.Update(dt, speed, duckHeld);
			if (jumpPressed)
				dino.Jump(speed);
			if (jumpReleased)
				dino.EndJump();             // 原版：松开跳跃键后钳制下落速度
			if (dino.isJumping)
				dino.speedDrop = duckHeld;   // 原版：空中按住下键速降（x3）
			else
				dino.Duck(duckHeld);

			// 分数 = 距离 x 0.025（原版 distanceMeter）
			distanceRan += speed * 60.0f * dt;
			int newScore = (int)ceil(distanceRan * SCORE_COEFFICIENT);
			if (newScore / 100 != lastHundred)
			{
				lastHundred = newScore / 100;
				flashTimer = 0.75f;   // 每 100 分闪烁 3 次 x 250ms（原版 achievement）
			}
			score = newScore;

			// 速度曲线：每帧 +0.001，上限 13（原版）
			if (speed < SPEED_MAX)
				speed += ACCELERATION * 60.0f * dt;

			// 障碍物
			for (auto& obs : obstacles)
				obs.Update(dt, speed);
			obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
							[](Obstacle& o) { return o.IsOffScreen(); }),
						obstacles.end());
			if (obstacles.empty())
			{
				SpawnObstacle(obstacles, history, speed, groundY);
			}
			else
			{
				Obstacle& last = obstacles.back();
				if (!last.nextSpawned && last.IsVisible() &&
					(last.x + last.width + last.gap) < currentWidth)
				{
					SpawnObstacle(obstacles, history, speed, groundY);
					last.nextSpawned = true;
				}
			}

			// 碰撞（原版只检测最近的第一组障碍物）
			if (!obstacles.empty())
			{
				Obstacle& first = obstacles.front();
				Rectangle firstRect = {first.x, first.y, first.width, first.height};
				if (dino.Hits(first.boxes, firstRect))
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

			// 夜间模式触发（原版：分数为 700 的倍数时）
			if (score > 0 && score % INVERT_DISTANCE == 0 && nightTimer == 0 && !nightMode)
			{
				nightMode = true;
				nightTimer = 0.001f;
				moonPhase = (moonPhase + 1) % 7;
			}
			if (nightTimer > 0)
			{
				nightTimer += dt;
				if (nightTimer >= INVERT_FADE_DURATION)
				{
					nightMode = false;
					nightTimer = 0;
				}
			}
		}
		else
		{
			dino.Update(dt, speed, false);
			dino.waiting = !gameStarted && !gameOver && introDone;
		}

		// 云始终飘动（原版等待时也更新）
		UpdateClouds(clouds, speed, dt);

		// 夜间渐变
		if (nightMode && nightAlpha < 1.0f)
			nightAlpha += NIGHT_FADE_SPEED * dt;
		else if (!nightMode && nightAlpha > 0)
			nightAlpha -= NIGHT_FADE_SPEED * dt;
		if (nightAlpha < 0) nightAlpha = 0;
		if (nightAlpha > 1.0f) nightAlpha = 1.0f;

		// 月亮与星星
		if (nightAlpha > 0)
		{
			float moonW = (moonPhase == 3 ? 40.0f : 20.0f) * YS * scaleY;
			float moonH = 40.0f * YS * scaleY;
			moonX -= 0.25f * XS * globalScale * 60.0f * dt;
			if (moonX < -moonW) moonX = currentWidth;
			for (int i = 0; i < 2; i++)
			{
				stars[i].x -= 0.3f * XS * globalScale * 60.0f * dt;
				if (stars[i].x < -24.0f * YS * scaleY)
					stars[i].x = currentWidth + 12.0f * YS * scaleY;
			}
		}
		else
		{
			// 完全回到白天后重新摆放星星（原版 placeStars）
			for (int i = 0; i < 2; i++)
			{
				int seg = currentWidth / 2;
				stars[i].x = (float)(seg * i + rand() % (seg + 1));
				stars[i].y = (float)(rand() % (int)(currentHeight * 0.47f));
			}
		}

		if (flashTimer > 0) flashTimer -= dt;

		BeginDrawing();
		ClearBackground(DN(RAYWHITE, BLACK));

		for (auto& cloud : clouds)
			cloud.Draw();

		if (nightAlpha > 0)
		{
			// 月亮（带月相）与星星
			float moonW = (moonPhase == 3 ? 40.0f : 20.0f) * YS * scaleY;
			float moonH = 40.0f * YS * scaleY;
			float my = 30.0f * YS * scaleY;
			Color whiteA = {255, 255, 255, (unsigned char)(255 * nightAlpha)};
			Color blackA = {0, 0, 0, (unsigned char)(255 * nightAlpha)};
			DrawCircleV({moonX + moonW / 2, my + moonH / 2}, moonW / 2, whiteA);
			float cover = starPhaseOffsets[moonPhase] / 180.0f;
			if (moonPhase % 2 == 0)
				DrawRectangle((int)(moonX + moonW / 2 + (0.5f - cover) * moonW), (int)my, (int)(cover * moonW + 1), (int)moonH, blackA);
			else
				DrawRectangle((int)moonX, (int)my, (int)(cover * moonW + 1), (int)moonH, blackA);
			for (int i = 0; i < 2; i++)
			{
				float s = 9.0f * YS * scaleY;
				float sx = stars[i].x, sy = stars[i].y;
				DrawLineEx({sx - s / 2, sy}, {sx + s / 2, sy}, 2.0f * globalScale, whiteA);
				DrawLineEx({sx, sy - s / 2}, {sx, sy + s / 2}, 2.0f * globalScale, whiteA);
			}
		}

		DrawGround(groundY, dt, speed);

		for (auto& obs : obstacles)
			obs.Draw();

		dino.Draw();

		int fontSize = (int)(24.0f * globalScale);
		int largeFontSize = (int)(36.0f * globalScale);

		if (!gameStarted && !gameOver && introDone)
		{
			const char* startText = "Press SPACE or UP to Start";
			int textWidth = MeasureText(startText, fontSize);
			DrawText(startText, currentWidth / 2 - textWidth / 2, currentHeight / 2 - fontSize * 2, fontSize, DN(DARKGRAY, {175, 175, 175, 255}));
		}

		const char* scoreText = TextFormat("%05d", score);
		const char* hiText = TextFormat("HI %05d", highScore);
		int hiWidth = MeasureText(hiText, fontSize);
		int scoreWidth = MeasureText(scoreText, fontSize);
		int maxWidth = std::max(hiWidth, scoreWidth);
		DrawText(hiText, currentWidth - maxWidth - (int)(30 * globalScale), (int)(30 * globalScale), fontSize, DN({128, 128, 128, 255}, {127, 127, 127, 255}));
		// 原版每 100 分闪烁 3 次（250ms 亮 / 250ms 灭）
		bool scoreVisible = true;
		if (flashTimer > 0)
		{
			int phase = (int)((0.75f - flashTimer) / 0.25f) % 2;
			scoreVisible = (phase == 0);
		}
		if (scoreVisible)
			DrawText(scoreText, currentWidth - maxWidth - (int)(30 * globalScale), (int)(30 * globalScale) + fontSize + (int)(8 * globalScale), fontSize, DN(DARKGRAY, {175, 175, 175, 255}));

		if (gameOver)
		{
			const char* gameOverText = "G A M E  O V E R";
			int goWidth = MeasureText(gameOverText, largeFontSize);
			DrawText(gameOverText, currentWidth / 2 - goWidth / 2, currentHeight / 2 - largeFontSize, largeFontSize, DN({83, 83, 83, 255}, {172, 172, 172, 255}));
			const char* restartText = "Press SPACE to Restart";
			int restartWidth = MeasureText(restartText, fontSize);
			DrawText(restartText, currentWidth / 2 - restartWidth / 2, currentHeight / 2 + (int)(10 * globalScale), fontSize, DN({128, 128, 128, 255}, {127, 127, 127, 255}));
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
