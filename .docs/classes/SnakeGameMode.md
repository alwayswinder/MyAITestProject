# SnakeGameMode（游戏模式）

## 文件路径
- 头文件: `SnakeGameMode.h`
- 源文件: `SnakeGameMode.cpp`

## 类说明

继承自 AGameModeBase，是游戏的入口点，负责在构造函数中设置默认的 PlayerController 和 HUD 类。

## 主要功能

- 在构造函数中指定默认 PlayerController 类（SnakePlayerController）
- 在构造函数中指定默认 HUD 类（SnakeHUD）
- 作为游戏的 World Settings 入口

## 配置说明

在关卡的 World Settings 中将 Game Mode Override 设置为 `SnakeGameMode`（或其蓝图子类），即可自动使用 SnakePlayerController 和 SnakeHUD。

无需额外配置，所有游戏对象的生成和管理均由场景中的 SnakeManager Actor 负责。
