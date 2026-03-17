# Snake（蛇）

## 文件路径
- 头文件: `Snake.h`
- 源文件: `Snake.cpp`

## 类说明

蛇的核心类，继承自APawn，负责控制蛇的移动、方向、蛇身段管理、碰撞检测和加速逻辑。支持隐身和无敌效果。

## 主要功能

- 控制蛇的移动和方向
- 管理蛇身段
- 检测碰撞（边界、自身、障碍物）
- 处理加速逻辑
- 支持隐身和无敌效果
- 应用食物效果

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| MoveSpeed | float | 正常移动速度 |
| BoostMoveSpeed | float | 加速移动速度 |
| InitialSegmentCount | int32 | 初始蛇身段数量 |
| SnakeSegmentClass | TSubclassOf&lt;ASnakeSegment&gt; | 蛇身段类 |
| BoundaryDistanceX | float | X轴边界距离 |
| BoundaryDistanceY | float | Y轴边界距离 |
| EffectDuration | float | 特效持续时间 |
| HeadMesh | UStaticMeshComponent* | 蛇头网格组件 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| StartGame() | 初始化蛇 |
| ChangeDirection() | 改变移动方向 |
| EatFood() | 吃到食物，增加蛇身 |
| GameOver() | 游戏结束 |
| StartBoost() | 开始加速 |
| StopBoost() | 停止加速 |
| IsPositionOccupiedBySnake() | 检查位置是否被蛇占据 |
| ApplyFoodEffect() | 应用食物效果 |
| IsInvisible() | 检查是否隐身 |
| IsInvincible() | 检查是否无敌 |

## 委托

| 委托名 | 说明 |
|--------|------|
| OnInvisibleStarted | 隐身效果开始时触发 |
| OnInvisibleEnded | 隐身效果结束时触发 |
| OnInvincibleStarted | 无敌效果开始时触发 |
| OnInvincibleEnded | 无敌效果结束时触发 |

## 碰撞逻辑

- **边界碰撞**：任何状态下都会触发游戏结束
- **自身碰撞**：隐身状态下不检查
- **障碍物碰撞**：无敌状态下摧毁障碍物，否则触发游戏结束
