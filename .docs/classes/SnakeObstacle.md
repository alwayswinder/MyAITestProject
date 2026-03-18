# SnakeObstacle（障碍物）

## 文件路径
- 头文件: `SnakeObstacle.h`
- 源文件: `SnakeObstacle.cpp`

## 类说明

障碍物类，继承自 AActor，代表游戏中的障碍物。蛇碰到障碍物会根据状态触发不同结果（详见 Snake 碰撞逻辑）。包含 StaticMeshComponent 用于可视化，碰撞设置为 `BlockAll`。

## 主要功能

- 作为游戏中的静态障碍物
- 提供可视化（StaticMeshComponent，碰撞：BlockAll）
- BeginPlay 时从 SnakeManager 同步 GridSize

## 可配置属性（UPROPERTY）

| 属性名 | 类型 | 说明 |
|--------|------|------|
| MeshComponent | UStaticMeshComponent* | 障碍物的静态网格组件（VisibleAnywhere，为 RootComponent，碰撞为 BlockAll） |

## 私有状态变量

| 变量名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| SnakeManager | ASnakeManager* | - | 缓存的管理器引用 |
| GridSize | float | 100.0f | 网格大小（从 SnakeManager 同步） |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| FindSnakeManager() | 在场景中查找 SnakeManager，同步 GridSize |
| InitializeMesh() | 初始化网格（当前为空，待蓝图设置 Mesh） |

## 生命周期

- 由 SnakeManager.SpawnObstacle() 在随机有效位置生成
- 由 SnakeManager.ClearObstacles() 或蛇无敌碰撞时销毁
