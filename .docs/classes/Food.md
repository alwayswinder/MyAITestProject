# Food（食物）

## 文件路径
- 头文件: `Food.h`
- 源文件: `Food.cpp`

## 类说明

食物类，负责在随机位置生成食物，检测与蛇的碰撞，并在碰撞后重新生成。支持多种食物类型，包括普通食物、隐身食物和无敌食物，并提供收集动画效果。

## 主要功能

- 在随机位置生成食物
- 检测与蛇的碰撞
- 碰撞后重新生成
- 支持多种食物类型（普通、隐身、无敌）
- 收集动画效果（弹跳和缩放）
- 触发障碍物生成

## 食物类型（EFoodType）

| 类型 | 说明 | 概率 |
|------|------|------|
| Normal | 普通食物，无特殊效果 | 70% |
| Invisible | 隐身食物，使蛇隐身一段时间 | 10% |
| Invincible | 无敌食物，使蛇无敌一段时间 | 20% |

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| FoodType | EFoodType | 当前食物类型 |
| MeshComponent | UStaticMeshComponent* | 食物的静态网格组件 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| OnBeginOverlap() | 碰撞检测回调 |
| SpawnAtRandomLocation() | 在随机位置生成食物 |
| RandomizeFoodType() | 随机化食物类型 |
| StartCollectAnimation() | 开始收集动画 |
| UpdateCollectAnimation() | 更新收集动画 |

## 委托

| 委托名 | 说明 |
|--------|------|
| OnFoodTypeChanged | 食物类型改变时触发 |
