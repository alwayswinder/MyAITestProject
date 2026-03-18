# Food（食物）

## 文件路径
- 头文件: `Food.h`
- 源文件: `Food.cpp`

## 类说明

食物类，继承自 AActor，负责在随机位置生成食物，检测与蛇的碰撞，并在碰撞后播放收集动画再重新生成。支持多种食物类型，并在每次收集后触发障碍物生成。

## 主要功能

- 在随机位置生成食物（通过 SnakeManager.GetRandomValidPosition）
- 检测与蛇的碰撞（Overlap 事件）
- 收集后重新随机食物类型并重新生成
- 支持多种食物类型（普通、隐身、无敌）
- 收集动画效果（弹跳上升 + 缩放至消失，持续 0.5 秒）
- 动画结束后触发 SnakeManager.SpawnObstacle()
- 广播 OnFoodTypeChanged 委托

## 食物类型（EFoodType，定义在 Food.h）

| 类型 | 说明 | 实际概率（0-9随机数） |
|------|------|--------------------|
| Normal | 普通食物，无特殊效果 | 70%（0-6） |
| Invisible | 隐身食物，使蛇隐身一段时间 | 10%（7） |
| Invincible | 无敌食物，使蛇无敌一段时间 | 20%（8-9） |

## 公开可配置属性（UPROPERTY）

| 属性名 | 类型 | 说明 |
|--------|------|------|
| FoodType | EFoodType | 当前食物类型（EditAnywhere） |
| MeshComponent | UStaticMeshComponent* | 食物的静态网格组件（VisibleAnywhere，为RootComponent） |
| OnFoodTypeChanged | FOnFoodTypeChanged | 食物类型改变时触发的委托（BlueprintAssignable） |

## 公开函数

| 函数名 | 说明 |
|--------|------|
| OnBeginOverlap() | Overlap 碰撞回调：检测到蛇时调用 EatFood/ApplyFoodEffect，更新分数，启动收集动画 |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| FindSnakeManager() | 在场景中查找并缓存 SnakeManager，同步 GridSize |
| InitializeMesh() | 初始化网格（当前为空，待蓝图设置 Mesh） |
| RandomizeFoodType() | 使用 0-9 随机数决定食物类型，并广播 OnFoodTypeChanged |
| SpawnAtRandomLocation() | 调用 SnakeManager.GetRandomValidPosition() 移动到新位置 |
| StartCollectAnimation() | 开始收集动画：记录初始位置和缩放，禁用碰撞 |
| UpdateCollectAnimation() | 每帧更新弹跳高度（Sin曲线）和缩放（线性减小），动画结束后重新生成并触发障碍物生成 |

## 私有状态变量

| 变量名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| SnakeManager | ASnakeManager* | - | 缓存的管理器引用 |
| GridSize | float | 100.0f | 网格大小（从 SnakeManager 同步） |
| bIsCollecting | bool | false | 是否正在播放收集动画 |
| AnimationTimer | float | 0.0f | 当前动画已播放时间 |
| AnimationDuration | float | 0.5f | 动画总时长（秒） |
| BounceHeight | float | 150.0f | 弹跳最大高度 |
| InitialLocation | FVector | - | 开始动画时记录的位置 |
| InitialScale | FVector | - | 开始动画时记录的缩放 |

## 委托

| 委托名 | 签名 | 说明 |
|--------|------|------|
| OnFoodTypeChanged | `(EFoodType NewFoodType)` | 食物类型改变时触发，携带新类型作为参数 |
