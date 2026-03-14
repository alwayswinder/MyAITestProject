# Snake（蛇）

## 文件路径
- 头文件: `Snake.h`
- 源文件: `Snake.cpp`

## 类说明

蛇的核心类，负责控制蛇的移动、方向、蛇身段管理、碰撞检测和加速逻辑。

## 主要功能

- 控制蛇的移动和方向
- 管理蛇身段
- 检测碰撞（边界和自身）
- 处理加速逻辑

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| MoveSpeed | float | 正常移动速度 |
| BoostMoveSpeed | float | 加速移动速度 |
| InitialSegmentCount | int32 | 初始蛇身段数量 |
| SnakeSegmentClass | TSubclassOf<ASnakeSegment> | 蛇身段类 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| StartGame() | 初始化蛇 |
| ChangeDirection() | 改变移动方向 |
| EatFood() | 吃到食物，增加蛇身 |
| GameOver() | 游戏结束 |
| StartBoost() | 开始加速 |
| StopBoost() | 停止加速 |
