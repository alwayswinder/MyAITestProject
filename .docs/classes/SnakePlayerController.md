# SnakePlayerController（玩家控制器）

## 文件路径
- 头文件: `SnakePlayerController.h`
- 源文件: `SnakePlayerController.cpp`

## 类说明

玩家控制器类，负责处理玩家输入、控制蛇的移动方向、处理长按加速逻辑。

## 主要功能

- 处理玩家输入
- 控制蛇的移动方向
- 处理长按加速逻辑

## 输入绑定

| 输入名称 | 绑定按键 | 说明 |
|----------|----------|------|
| MoveUp | W 键 / 上箭头 | 向上移动 |
| MoveDown | S 键 / 下箭头 | 向下移动 |
| MoveLeft | A 键 / 左箭头 | 向左移动 |
| MoveRight | D 键 / 右箭头 | 向右移动 |

## 加速机制

- 长按任意方向键即可加速
- 松开后恢复正常速度
