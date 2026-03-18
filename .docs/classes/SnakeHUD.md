# SnakeHUD（HUD类）

## 文件路径
- 头文件: `SnakeHUD.h`
- 源文件: `SnakeHUD.cpp`

## 类说明

继承自 AHUD，负责在 BeginPlay 时创建和管理游戏的 UI 界面实例（菜单UI和游戏UI），并提供访问接口供 SnakeManager 调用。

## 主要功能

- 在 BeginPlay 时创建 SnakeMenuUI 并直接加入 Viewport（初始显示）
- 在 BeginPlay 时创建 SnakeGameUI（不加入 Viewport，由 SnakeManager 控制显示时机）
- 提供 GetSnakeMenuUI() / GetSnakeGameUI() 访问接口

## 可配置属性（UPROPERTY，EditAnywhere）

| 属性名 | 类型 | 说明 |
|--------|------|------|
| SnakeMenuUIClass | TSubclassOf&lt;USnakeMenuUI&gt; | 菜单UI蓝图类（在编辑器Details面板设置） |
| SnakeGameUIClass | TSubclassOf&lt;USnakeGameUI&gt; | 游戏UI蓝图类（在编辑器Details面板设置） |

## 主要函数

| 函数名 | 返回值 | 说明 |
|--------|--------|------|
| GetSnakeMenuUI() | USnakeMenuUI* | 返回菜单UI实例（BlueprintCallable, BlueprintPure） |
| GetSnakeGameUI() | USnakeGameUI* | 返回游戏UI实例（BlueprintCallable, BlueprintPure） |

## 生命周期

- `BeginPlay()`：若 SnakeMenuUIClass 有效则创建并 AddToViewport；若 SnakeGameUIClass 有效则仅创建不显示
