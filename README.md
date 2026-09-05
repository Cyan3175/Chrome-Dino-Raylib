# Chrome Dino Raylib

使用 C++ 和 [raylib](https://www.raylib.com/) 6.0 编写的 Chrome 恐龙小游戏（T-Rex Runner）复刻版。

## 版本与下载

| 版本 | 说明 | 下载 |
| ---- | ---- | ---- |
| v1.4 | 修复全屏下蹲无敌（下蹲碰撞盒按地面锚定，仙人掌仍会命中）与空中按 ↓ 跳高 Bug（取消上行反向加速），-Ofast 编译 | [ChromeDino-1.4.exe](https://github.com/Cyan3175/Chrome-Dino-Raylib/releases/download/v1.4/ChromeDino-1.4.exe) |
| v1.3 | 修复窗口缩放时云的位置/大小错位（云改为基准坐标实时缩放，障碍物同步重缩放） | [ChromeDino-1.3.exe](https://github.com/Cyan3175/Chrome-Dino-Raylib/releases/download/v1.3/ChromeDino-1.3.exe) |
| v1.2 | 基于 Chromium 原版源码深度仿制（raylib 6.0 构建）：距离计分、原版速度曲线、夜间模式、原版碰撞盒、翼龙三档高度 | [ChromeDino-1.2.exe](https://github.com/Cyan3175/Chrome-Dino-Raylib/releases/download/v1.2/ChromeDino-1.2.exe) |
| v1.1 | 自适应显示器刷新率（raylib 6.0 构建），无需输入 FPS，双击即玩 | [ChromeDino-1.1.exe](https://github.com/Cyan3175/Chrome-Dino-Raylib/releases/download/v1.1/ChromeDino-1.1.exe) |
| v1.0 | 原版（启动时在控制台输入 FPS）（raylib 6.0 构建） | [ChromeDino-1.0.exe](https://github.com/Cyan3175/Chrome-Dino-Raylib/releases/download/v1.0/ChromeDino-1.0.exe) |

## 玩法

- **空格 / ↑ / 鼠标左键**：跳跃
- **↓**：下蹲（可躲避低飞的翼龙）
- **F11**：切换全屏

躲避仙人掌和翼龙，跑得越远分数越高。最高分会自动保存到 `highscore.dat`。

## 特性

- 窗口可自由缩放，游戏画面按比例适配
- 垂直同步，画面流畅
- 仙人掌与翼龙（三种高度）障碍物
- 分数与最高分记录（本地持久化）
- 游戏结束一键重新开始

## 构建

需要安装 [raylib 6.0](https://github.com/raysan5/raylib/releases/tag/6.0)（Windows 使用 `raylib-6.0_win64_mingw-w64.zip`）。

**Windows（MinGW / g++）：**

```sh
g++ "Chrome Dino Raylib 1.4.cpp" -o dino.exe -lraylib -lopengl32 -lgdi32 -lwinmm -std=c++11 -Ofast
```

**发布构建（-Ofast 优化 + 静态链接，无控制台窗口）：**

```sh
g++ "Chrome Dino Raylib 1.4.cpp" -o release/ChromeDino-1.4.exe -I"vendor/raylib-6.0/raylib-6.0_win64_mingw-w64/include" -L"vendor/raylib-6.0/raylib-6.0_win64_mingw-w64/lib" -lraylib -lopengl32 -lgdi32 -lwinmm -std=c++11 -Ofast -mwindows -static
```

**Windows（MSVC）：**

```sh
cl "Chrome Dino Raylib 1.4.cpp" /O2 /I"path\to\raylib\src" /link /LIBPATH:"path\to\raylib" raylib.lib user32.lib gdi32.lib winmm.lib shell32.lib
```

**Linux：**

```sh
g++ "Chrome Dino Raylib 1.4.cpp" -o dino -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -std=c++11 -Ofast
```

运行生成的 `dino` / `dino.exe` 即可开始游戏。
