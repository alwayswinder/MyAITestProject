# SnakeMenuUI（菜单UI）

## 文件路径
- 头文件: `SnakeMenuUI.h`
- 源文件: `SnakeMenuUI.cpp`

## 类说明

继承自 UUserWidget，游戏的菜单UI界面，包括开始游戏按钮、分数显示和最高分显示。兼作游戏结束界面（由 `bIsGameOver` 标记区分）。

## 主要功能

- 在 NativeConstruct 时绑定 StartGameButton 按钮事件（仅绑定一次）
- 显示/隐藏鼠标光标，切换输入模式（UI Only ↔ Game Only）
- 显示当前分数（从 SnakeManager 获取）
- 显示最高分列表（从 SnakeManager 获取）
- 点击开始游戏时调用 SnakeManager.StartGame() 并隐藏自身

## 可配置属性（UPROPERTY）

| 属性名 | 类型 | 说明 |
|--------|------|------|
| bIsGameOver | bool | 是否为游戏结束状态（EditAnywhere，供蓝图判断显示内容） |
| StartGameButton | UButton* | 开始游戏按钮（meta=BindWidget，蓝图中控件名必须为 StartGameButton） |

## 公开函数

| 函数名 | 签名 | 说明 |
|--------|------|------|
| ShowUI() | `void ShowUI(bool bInIsGameOver = false)` | 更新 bIsGameOver，重新 AddToViewport，显示鼠标并切换UI输入模式 |
| GetScore() | `int32 GetScore()` | 从 SnakeManager 获取当前分数（BlueprintCallable, BlueprintPure） |
| GetHighScores() | `TArray<int32> GetHighScores()` | 从 SnakeManager 获取最高分列表（BlueprintCallable, BlueprintPure） |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| OnStartGameClicked() | 按钮回调：调用 SnakeManager.StartGame()，然后调用 RemoveUI() |
| FindSnakeManager() | 通过 GetAllActorsOfClass 查找并返回场景中的 SnakeManager |
| RemoveUI() | 隐藏鼠标，切换 Game Only 输入模式，从 Viewport 移除 |

## 蓝图设置要求

- 蓝图父类设为 `SnakeMenuUI`
- Widget 中必须有名为 `StartGameButton` 的 UButton 控件（与 BindWidget 宏对应）
