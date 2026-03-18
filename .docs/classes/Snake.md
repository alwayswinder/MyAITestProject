# Snake（蛇）

## 文件路径
- 头文件: `Snake.h`
- 源文件: `Snake.cpp`

## 类说明

蛇的核心类，继承自APawn，负责控制蛇的移动、方向、蛇身段管理、碰撞检测和加速逻辑。支持隐身和无敌效果。

## 主要功能

- 控制蛇的移动和方向（每帧通过计时器驱动）
- 管理蛇身段（SnakeSegments 数组）
- 检测碰撞（边界环绕、自身碰撞、障碍物碰撞）
- 处理长按方向键加速逻辑
- 支持隐身和无敌效果及计时
- 应用食物效果

## 可配置属性（UPROPERTY）

| 属性名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| MoveSpeed | float | 0.2f | 正常移动间隔（秒） |
| BoostMoveSpeed | float | 0.1f | 加速移动间隔（秒） |
| InitialSegmentCount | int32 | 3 | 初始蛇身段数量（含蛇头） |
| SnakeSegmentClass | TSubclassOf&lt;ASnakeSegment&gt; | - | 蛇身段类 |
| BoundaryDistanceX | float | 500.0f | X轴边界距离（从SnakeManager获取） |
| BoundaryDistanceY | float | 500.0f | Y轴边界距离（从SnakeManager获取） |
| EffectDuration | float | 10.0f | 特效持续时间（秒） |
| HeadMesh | UStaticMeshComponent* | - | 蛇头网格组件（VisibleAnywhere） |
| SnakeSegments | TArray&lt;ASnakeSegment*&gt; | - | 蛇身段列表（EditAnywhere） |

## 公开函数

| 函数名 | 签名 | 说明 |
|--------|------|------|
| StartGame() | `void StartGame()` | 初始化蛇：清除旧段、设置初始方向、生成初始段、启动移动计时器 |
| ChangeDirection() | `void ChangeDirection(FVector2D NewDirection)` | 设置待处理方向（防止反向），并重置长按加速计时器 |
| ReleaseDirection() | `void ReleaseDirection()` | 松开方向键时调用，停止加速并重置长按计时器 |
| EatFood() | `void EatFood()` | 在蛇尾添加一段新身体 |
| GameOver() | `void GameOver()` | 停止移动计时器 |
| StartBoost() | `void StartBoost()` | 进入加速状态，切换到 BoostMoveSpeed |
| StopBoost() | `void StopBoost()` | 退出加速状态，恢复 MoveSpeed |
| IsPositionOccupiedBySnake() | `bool IsPositionOccupiedBySnake(FVector, float)` | 检查指定位置是否被蛇头或蛇身占据 |
| ApplyFoodEffect() | `void ApplyFoodEffect(EFoodType)` | 根据食物类型设置隐身或无敌状态，并广播委托 |
| IsInvisible() | `bool IsInvisible() const` | 返回当前是否处于隐身状态 |
| IsInvincible() | `bool IsInvincible() const` | 返回当前是否处于无敌状态 |
| GetCurrentDirection() | `FVector2D GetCurrentDirection() const` | 返回当前移动方向向量 |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| MoveSnake() | 计时器回调：更新方向、移动蛇头和蛇身、执行碰撞检测、重设计时器 |
| SpawnInitialSegments() | 在蛇头后面生成 InitialSegmentCount-1 个身体段 |
| CheckCollision() | 检查边界穿越、自身碰撞和障碍物碰撞 |
| GetBoundaryFromManager() | 在 BeginPlay 时从 SnakeManager 获取边界距离和网格大小 |
| UpdateEffects(float) | 每帧减少 EffectTimer，计时结束时调用 ClearEffects() |
| ClearEffects() | 清除隐身/无敌状态并广播对应的结束委托 |

## 私有状态变量

| 变量名 | 类型 | 说明 |
|--------|------|------|
| CurrentDirection | FVector2D | 当前实际移动方向（每次移动时从 PendingDirection 更新） |
| PendingDirection | FVector2D | 下一次移动时将使用的方向 |
| LastMovedDirection | FVector2D | 上一次实际移动的方向（用于防止反向移动） |
| MoveTimerHandle | FTimerHandle | 移动计时器句柄 |
| SnakeManager | ASnakeManager* | 关联的游戏管理器 |
| GridSize | float | 每格大小（从 SnakeManager 获取，默认 100.0f） |
| CurrentMoveSpeed | float | 当前移动间隔 |
| bIsBoosting | bool | 是否正在加速 |
| bIsInvisible | bool | 是否处于隐身状态 |
| bIsInvincible | bool | 是否处于无敌状态 |
| EffectTimer | float | 当前特效剩余时间 |
| bIsDirectionPressed | bool | 方向键是否持续按下 |
| PressedDirection | FVector2D | 当前按下的方向 |
| DirectionPressTimer | float | 方向键持续按下的计时（超过 BoostThreshold 触发加速） |
| BoostThreshold | const float | 长按加速阈值，固定为 0.5 秒 |

## 委托

| 委托名 | 类型 | 说明 |
|--------|------|------|
| OnInvisibleStarted | FOnEffectStarted（无参动态多播） | 隐身效果开始时触发 |
| OnInvisibleEnded | FOnEffectEnded（无参动态多播） | 隐身效果结束时触发 |
| OnInvincibleStarted | FOnEffectStarted（无参动态多播） | 无敌效果开始时触发 |
| OnInvincibleEnded | FOnEffectEnded（无参动态多播） | 无敌效果结束时触发 |

## 碰撞逻辑

- **边界碰撞**：蛇头超出边界时**从对侧穿出**（环绕机制），不触发游戏结束
- **自身碰撞**：隐身状态下跳过检测；普通/无敌状态下碰到蛇身触发游戏结束
- **障碍物碰撞**：
  - 无敌状态 → 摧毁障碍物
  - 隐身状态（非无敌）→ 忽略（穿过障碍物）
  - 普通状态 → 触发游戏结束

## 移动逻辑

蛇的移动由 `FTimerHandle MoveTimerHandle` 以 `CurrentMoveSpeed` 为间隔驱动。每次移动：
1. 将 `PendingDirection` 应用为 `CurrentDirection`
2. 蛇头（Snake Actor 本身）向前移动一格（`GridSize`）
3. 蛇身段从后往前依次移动到前一段的位置
4. 执行碰撞检测
5. 重设计时器（以便动态改变移动速度生效）
