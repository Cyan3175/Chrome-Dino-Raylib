# Chrome Dino Raylib

使用 C++ 和 [raylib](https://www.raylib.com/) 编写的 Chrome 恐龙小游戏（T-Rex Runner）复刻版。

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

需要安装 [raylib](https://github.com/raysan5/raylib/releases)（4.x / 5.x 均可）。

**Windows（MinGW / g++）：**

```sh
g++ "Chrome Dino Raylib.cpp" -o dino.exe -lraylib -lopengl32 -lgdi32 -lwinmm -std=c++11
```

**Windows（MSVC）：**

```sh
cl "Chrome Dino Raylib.cpp" /I"path\to\raylib\src" /link /LIBPATH:"path\to\raylib" raylib.lib user32.lib gdi32.lib winmm.lib shell32.lib
```

**Linux：**

```sh
g++ "Chrome Dino Raylib.cpp" -o dino -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -std=c++11
```

运行生成的 `dino` / `dino.exe` 即可开始游戏。
