# SnakeMenuUI（菜单UI）

## 文件路径
- 头文件: `SnakeMenuUI.h`
- 源文件: `SnakeMenuUI.cpp`

## 类说明

继承自UUserWidget，游戏的菜单UI界面，包括开始游戏按钮、分数显示和最高分显示。

## 主要功能

- 显示菜单UI
- 处理开始游戏按钮点击
- 显示当前分数
- 显示最高分列表
- 管理鼠标光标和输入模式

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| bIsGameOver | bool | 是否为游戏结束状态 |
| StartGameButton | UButton* | 开始游戏按钮（需绑定） |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| ShowUI() | 显示菜单UI |
| GetScore() | 获取当前分数 |
| GetHighScores() | 获取最高分列表 |
| OnStartGameClicked() | 开始游戏按钮点击回调 |
| FindSnakeManager() | 查找并关联SnakeManager |
