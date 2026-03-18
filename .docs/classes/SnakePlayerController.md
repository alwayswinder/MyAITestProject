# SnakePlayerController（玩家控制器）

## 文件路径
- 头文件: `SnakePlayerController.h`
- 源文件: `SnakePlayerController.cpp`

## 类说明

继承自 APlayerController，负责处理玩家的输入，将方向键/WASD映射到蛇的移动方向，并管理长按加速状态。

## 主要功能

- 处理方向输入（WASD/方向键的 Pressed/Released 事件）
- 调用 Snake::ChangeDirection() 改变蛇的移动方向
- 通过 PressedKeyCount 跟踪当前按下的按键数量，全部松开时调用 ReleaseDirection()
- 与 Snake 实例关联（由 SnakeManager 在生成蛇后调用 SetSnake()）

## 输入绑定

使用传统输入系统（`InputComponent->BindAction`），需在项目设置中配置以下 Action：

| 输入动作 | 推荐绑定键 | 传入方向向量 |
|----------|-----------|-------------|
| MoveUp | W / 上箭头 | `FVector2D(0, 1)` → +Y 方向 |
| MoveDown | S / 下箭头 | `FVector2D(0, -1)` → -Y 方向 |
| MoveLeft | A / 左箭头 | `FVector2D(1, 0)` → +X 方向 |
| MoveRight | D / 右箭头 | `FVector2D(-1, 0)` → -X 方向 |

> ⚠️ 注意：MoveLeft 传入 +X（Yaw=0°），MoveRight 传入 -X（Yaw=180°）。实际屏幕上的显示方向取决于相机朝向（SnakeManager 的 SpringArm 为 -90° Pitch，Yaw=0）。

## 公开函数

| 函数名 | 说明 |
|--------|------|
| SetSnake() | 设置关联的 Snake 实例（由 SnakeManager 在 SpawnSnake() 中调用） |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| MoveUp() | 调用 `Snake->ChangeDirection(FVector2D(0, 1))` |
| MoveDown() | 调用 `Snake->ChangeDirection(FVector2D(0, -1))` |
| MoveLeft() | 调用 `Snake->ChangeDirection(FVector2D(1, 0))` |
| MoveRight() | 调用 `Snake->ChangeDirection(FVector2D(-1, 0))` |
| HandleMovePressed() | 执行移动函数，PressedKeyCount++ |
| HandleMoveReleased() | PressedKeyCount--，当归零时调用 `Snake->ReleaseDirection()` |
| OnMoveUpPressed/Released | MoveUp/Down/Left/Right 的 Pressed 和 Released 回调 |

## 私有状态变量

| 变量名 | 类型 | 说明 |
|--------|------|------|
| Snake | ASnake* | 关联的蛇实例 |
| PressedKeyCount | int32 | 当前按下的方向键数量（用于多键并按处理） |
| CurrentPressedDirection | FVector2D | 当前按下的方向（仅记录，实际逻辑由 Snake 管理） |

## 加速逻辑

- 每个 Pressed 事件调用对应的 `ChangeDirection()`，Snake 内部开始记录长按时间
- 长按超过 `BoostThreshold`（0.5秒）后，Snake 自动调用 `StartBoost()`
- 当所有方向键释放（`PressedKeyCount == 0`）时，调用 `Snake->ReleaseDirection()` 停止加速
