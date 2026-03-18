# SnakeSegment（蛇身段）

## 文件路径
- 头文件: `SnakeSegment.h`
- 源文件: `SnakeSegment.cpp`

## 类说明

简单的 Actor 类，代表蛇的身体段，包含 StaticMeshComponent 用于可视化。由 Snake 类在 SpawnInitialSegments() 和 EatFood() 中生成和管理。

## 主要功能

- 作为蛇的身体段
- 提供可视化（StaticMeshComponent）
- 由 Snake 类直接通过 `SetActorLocation()` 控制位置

## 可配置属性（UPROPERTY）

| 属性名 | 类型 | 说明 |
|--------|------|------|
| MeshComponent | UStaticMeshComponent* | 蛇身段的静态网格组件（VisibleAnywhere，为 RootComponent） |

## 私有函数

| 函数名 | 说明 |
|--------|------|
| InitializeMesh() | 初始化网格（当前为空，待蓝图设置 Mesh） |

## 生命周期

- 由 `Snake::SpawnInitialSegments()` 在 StartGame 时批量生成
- 由 `Snake::EatFood()` 在吃到食物时在蛇尾追加
- 由 `Snake::Destroyed()` 在 Snake 销毁时逐一销毁
