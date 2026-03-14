# SnakeHUD（HUD）

## 文件路径
- 头文件: `SnakeHUD.h`
- 源文件: `SnakeHUD.cpp`

## 类说明

HUD类，负责创建和管理UI Widget，提供获取UI的接口。

## 主要功能

- 创建和管理UI Widget
- 提供获取UI的接口

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| SnakeMenuUIClass | TSubclassOf<USnakeMenuUI> | 菜单UI类 |
| SnakeGameUIClass | TSubclassOf<USnakeGameUI> | 游戏UI类 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| GetSnakeMenuUI() | 获取菜单UI |
| GetSnakeGameUI() | 获取游戏UI |
