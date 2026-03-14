# SnakeManager（游戏管理器）

## 文件路径
- 头文件: `SnakeManager.h`
- 源文件: `SnakeManager.cpp`

## 类说明

游戏的核心管理器类，负责管理游戏状态、生成和销毁游戏对象、提供相机和边界可视化等功能。

## 主要功能

- 管理游戏状态（分数、游戏结束等）
- 生成和销毁Snake和Food
- 提供边界可视化（BoundaryMesh）
- 提供游戏相机（CameraComponent + SpringArmComponent）
- 管理UI的显示和隐藏

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| Score | int32 | 当前分数 |
| FoodScoreValue | int32 | 每次吃到食物增加的分数 |
| GridSize | float | 网格大小 |
| BoundaryDistanceX | float | 边界X轴距离 |
| BoundaryDistanceY | float | 边界Y轴距离 |
| SnakeClass | TSubclassOf<ASnake> | Snake类 |
| FoodClass | TSubclassOf<AFood> | Food类 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| StartGame() | 开始游戏 |
| GameOver() | 游戏结束 |
| UpdateBoundaryMesh() | 更新边界网格 |
| UpdateCameraDistance() | 更新相机距离 |
