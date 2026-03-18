# SnakeGameUI（游戏UI）

## 文件路径
- 头文件: `SnakeGameUI.h`
- 源文件: `SnakeGameUI.cpp`

## 类说明

继承自 UUserWidget，游戏进行中的 UI 界面，负责显示当前分数等游戏信息。由 SnakeManager 控制显示和隐藏。

## 主要功能

- 由 SnakeHUD 在 BeginPlay 时创建（但不立即显示）
- 由 SnakeManager.StartGame() 调用 ShowUI() 显示
- 由 SnakeManager.GameOver() 调用 HideUI() 隐藏
- 提供 GetScore() 供蓝图绑定当前分数显示

## 主要函数

| 函数名 | 说明 |
|--------|------|
| ShowUI() | 如果不在 Viewport 则 AddToViewport（BlueprintCallable） |
| HideUI() | 如果在 Viewport 则 RemoveFromParent（BlueprintCallable） |
| GetScore() | 从 SnakeManager 获取当前 Score 值（BlueprintCallable） |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| FindSnakeManager() | 通过 GetAllActorsOfClass 查找并返回 SnakeManager 引用 |

## 蓝图设置说明

- 蓝图父类设为 `SnakeGameUI`
- 可在 Widget 的 Graph 中调用 `Get Score` 函数获取当前分数用于文本绑定
