# SnakeObstacle（障碍物）

## 文件路径
- 头文件: `SnakeObstacle.h`
- 源文件: `SnakeObstacle.cpp`

## 类说明

障碍物类，代表游戏中的障碍物，蛇碰到障碍物会导致游戏结束。包含StaticMeshComponent用于可视化，设置为阻挡所有碰撞。

## 主要功能

- 作为游戏障碍物
- 提供可视化（StaticMeshComponent）
- 与SnakeManager关联，获取网格大小
- 设置碰撞检测为阻挡所有

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| MeshComponent | UStaticMeshComponent* | 障碍物的静态网格组件 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| InitializeMesh() | 初始化网格组件 |
| FindSnakeManager() | 查找并关联SnakeManager |
