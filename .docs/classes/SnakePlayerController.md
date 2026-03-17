# SnakePlayerController（玩家控制器）

## 文件路径
- 头文件: `SnakePlayerController.h`
- 源文件: `SnakePlayerController.cpp`

## 类说明

继承自APlayerController，负责处理玩家的输入，包括方向控制和加速逻辑。

## 主要功能

- 处理方向输入（WASD/方向键）
- 控制蛇的移动方向
- 管理加速状态（长按方向键加速）
- 与Snake关联

## 输入设置

| 输入动作 | 绑定键 |
|----------|--------|
| MoveUp | W / 上箭头 |
| MoveDown | S / 下箭头 |
| MoveLeft | A / 左箭头 |
| MoveRight | D / 右箭头 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| SetSnake() | 设置关联的Snake |
| MoveUp() | 向上移动 |
| MoveDown() | 向下移动 |
| MoveLeft() | 向左移动 |
| MoveRight() | 向右移动 |
| HandleMovePressed() | 处理移动键按下 |
| HandleMoveReleased() | 处理移动键释放 |

## 加速逻辑

- 长按任意方向键开始加速
- 松开所有方向键停止加速
- 通过PressedKeyCount跟踪按键数量
