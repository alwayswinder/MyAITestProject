# SnakeHUD（HUD类）

## 文件路径
- 头文件: `SnakeHUD.h`
- 源文件: `SnakeHUD.cpp`

## 类说明

继承自AHUD，负责管理游戏的UI界面，包括菜单UI和游戏UI。

## 主要功能

- 创建和管理菜单UI（SnakeMenuUI）
- 创建和管理游戏UI（SnakeGameUI）
- 提供UI访问接口

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| SnakeMenuUIClass | TSubclassOf&lt;USnakeMenuUI&gt; | 菜单UI类 |
| SnakeGameUIClass | TSubclassOf&lt;USnakeGameUI&gt; | 游戏UI类 |

## 主要函数

| 函数名 | 说明 |
|--------|------|
| GetSnakeMenuUI() | 获取菜单UI实例 |
| GetSnakeGameUI() | 获取游戏UI实例 |
