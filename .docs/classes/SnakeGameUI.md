# SnakeGameUI（游戏UI）

## 文件路径
- 头文件: `SnakeGameUI.h`
- 源文件: `SnakeGameUI.cpp`

## 类说明

继承自UUserWidget，游戏进行中的UI界面，负责显示分数等游戏信息。

## 主要功能

- 显示/隐藏游戏UI
- 获取当前分数
- 与SnakeManager关联

## 主要函数

| 函数名 | 说明 |
|--------|------|
| ShowUI() | 显示游戏UI |
| HideUI() | 隐藏游戏UI |
| GetScore() | 获取当前分数 |
| FindSnakeManager() | 查找并关联SnakeManager |
