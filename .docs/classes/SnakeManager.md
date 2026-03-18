# SnakeManager（游戏管理器）

## 文件路径
- 头文件: `SnakeManager.h`
- 源文件: `SnakeManager.cpp`

## 类说明

游戏的核心管理器类，继承自 AActor，负责管理游戏状态、生成和销毁游戏对象、提供相机和边界可视化、管理存档等功能。

## 主要功能

- 管理游戏状态（分数、游戏结束标志）
- 生成和销毁 Snake、Food 和 Obstacle
- 提供边界可视化（BoundaryMesh，根据边界距离自动缩放）
- 提供游戏相机（CameraComponent + SpringArmComponent，-90° 俯视）
- 管理 UI 的显示和隐藏（通过 SnakeHUD）
- 保存和加载最高分（存档槽 SnakeHighScores，保存 Top 10）
- 提供随机有效位置生成（避免与蛇、食物、障碍物重叠）

## 公开可配置属性（UPROPERTY）

| 属性名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| Score | int32 | 0 | 当前分数 |
| bGameOver | bool | false | 游戏结束标志 |
| FoodScoreValue | int32 | 10 | 每次吃到食物增加的分数 |
| GridSize | float | 100.0f | 网格大小（单位：UU） |
| BoundaryDistanceX | float | 500.0f | 边界X轴距离（管理器为中心） |
| BoundaryDistanceY | float | 500.0f | 边界Y轴距离（管理器为中心） |
| SnakeClass | TSubclassOf&lt;ASnake&gt; | - | Snake蓝图类 |
| FoodClass | TSubclassOf&lt;AFood&gt; | - | Food蓝图类 |
| ObstacleClass | TSubclassOf&lt;ASnakeObstacle&gt; | - | 障碍物蓝图类 |

## 私有组件

| 组件名 | 类型 | 说明 |
|--------|------|------|
| BoundaryMesh | UStaticMeshComponent* | 边界可视化平面，根据 BoundaryDistanceX/Y 自动缩放，无碰撞 |
| SpringArmComponent | USpringArmComponent* | 弹簧臂，旋转 -90° Pitch 实现俯视，禁用碰撞测试 |
| CameraComponent | UCameraComponent* | 挂载在 SpringArm 末端的相机组件 |

## 私有运行时变量

| 变量名 | 类型 | 说明 |
|--------|------|------|
| Snake | ASnake* | 当前关卡中的蛇实例 |
| Food | AFood* | 当前关卡中的食物实例 |
| Obstacles | TArray&lt;ASnakeObstacle*&gt; | 当前所有障碍物列表 |

## 公开函数

| 函数名 | 标记 | 说明 |
|--------|------|------|
| StartGame() | BlueprintCallable, CallInEditor | 重置分数、清除障碍物、生成蛇和食物、显示游戏UI |
| GameOver() | BlueprintCallable | 保存最高分、停止蛇、隐藏游戏UI、延迟0.1s显示菜单UI并销毁蛇/食物 |
| UpdateBoundaryMesh() | BlueprintCallable, CallInEditor | 根据 BoundaryDistanceX/Y 和 GridSize 更新边界平面缩放 |
| UpdateCameraDistance() | BlueprintCallable, CallInEditor | 计算并设置 SpringArm 长度以容纳整个游戏区域（边界最大值 × 1.5） |
| SpawnObstacle() | BlueprintCallable | 在随机有效位置生成障碍物并加入 Obstacles 列表 |
| IsPositionOccupied() | BlueprintCallable | 检查位置是否被食物、蛇或任意障碍物占据（容差 GridSize × 0.5） |
| GetRandomValidPosition() | BlueprintCallable | 最多尝试100次随机生成未被占据的网格位置 |
| SaveHighScore() | BlueprintCallable | 添加新分数到存档，按降序排序，最多保留10条 |
| GetHighScores() | BlueprintCallable, BlueprintPure | 从存档槽加载并返回最高分列表 |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| InitializeGame() | BeginPlay 时切换玩家视角到 SnakeManager 的相机 |
| SpawnSnake() | 在管理器位置生成蛇，对齐到网格，并将其设置到 SnakePlayerController |
| SpawnFood() | 在随机有效位置生成食物 |
| ClearObstacles() | 销毁并清空所有障碍物 |
| DelayedShowGameOverUI() | 延迟回调：显示菜单UI（GameOver状态），销毁蛇/食物/障碍物 |

## 编辑器支持

`PostEditChangeProperty` 重写：在编辑器中修改 `BoundaryDistanceX` 或 `BoundaryDistanceY` 属性时自动调用 `UpdateBoundaryMesh()` 和 `UpdateCameraDistance()` 以实时预览边界。
