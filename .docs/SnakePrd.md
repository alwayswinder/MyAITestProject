  请从一个全新的 Unreal Engine 5.3+ C++ Blank 项目开始，实现一个完整可玩的 3D
  贪吃蛇项目，功能必须与以下需求一致。技术要求：核心逻辑用 C++，UI 用
  UMG，允许用蓝图做资源绑定和场景配置，但不要把核心逻辑只写在蓝图里。

   必须实现这些功能：
   1. 基础玩法
   - 蛇在网格地图中移动，吃到食物后身体增长，分数 +10。
   - 撞到自己时游戏结束。
   - 撞到障碍物时游戏结束。
   - 注意：边界不是死亡，蛇头超出边界后要从另一侧穿出。

   2. 输入与移动
   - 使用传统输入系统配置 MoveUp/MoveDown/MoveLeft/MoveRight。
   - 键位：W/S/A/D 和 上下左右。
   - 长按任意方向键 0.5 秒后进入加速，松开恢复正常速度。

   3. 食物系统
   - 食物有 3 种类型：
     - Normal：70%
     - Invisible：10%
     - Invincible：20%
   - 每次吃到食物后：
     - 蛇身 +1
     - 分数 +10
     - 播放 0.5 秒收集动画（弹跳上升 + 缩放消失）
     - 在新的随机合法位置重新生成食物
     - 额外生成 1 个障碍物
   - Invisible 效果：持续 10 秒，期间可穿过自身和障碍物。
   - Invincible 效果：持续 10 秒，期间撞到障碍物会摧毁障碍物，但撞到自身仍然游戏结束。

   4. 障碍物系统
   - 每次吃到食物后随机生成一个障碍物。
   - 障碍物必须生成在合法网格位置，不能与蛇、食物、已有障碍物重叠。

   5. 存档与排行榜
   - 自动保存最高分。
   - 使用 SaveGame 保存 Top 10 分数，按降序排列。
   - 存档槽名：SnakeHighScores，UserIndex = 0。

   6. UI 与流程
   - 游戏启动先显示菜单 UI。
   - 菜单 UI 要有开始按钮、当前分数/最高分列表显示。
   - 点击开始后隐藏菜单 UI、显示游戏 UI、生成蛇和食物。
   - 游戏 UI 显示当前分数。
   - 游戏结束后隐藏游戏 UI，重新显示菜单 UI，并显示本次分数和最高分列表。

   7. 类与架构
   请按以下 C++ 类组织：
   - SnakeSaveGame
   - SnakeSegment
   - SnakeObstacle
   - Food（包含 EFoodType）
   - Snake（APawn）
   - SnakePlayerController
   - SnakeGameUI
   - SnakeMenuUI
   - SnakeHUD
   - SnakeManager（核心管理器）
   - SnakeGameMode

   职责要求：
   - SnakeGameMode：设置默认 PlayerController 和 HUD。
   - SnakeManager：管理分数、游戏状态、生成/销毁 Snake/Food/Obstacle、随机合法位置、相机、边界可视化、存档。
   - Snake：处理移动、方向、防反向、蛇身、边界环绕、碰撞、加速、隐身/无敌效果。
   - SnakePlayerController：处理输入并把方向传给 Snake。
   - SnakeHUD：创建并管理菜单 UI 和游戏 UI。

   8. 蓝图和地图
   请同时说明并完成必要的蓝图/资源配置：
   - BP_SnakeManager
   - BP_Snake
   - BP_Food
   - BP_SnakeSegment
   - BP_SnakeObstacle
   - WBP_SnakeMenuUI
   - WBP_SnakeGameUI
   - BP_SnakeHUD
   - 地图 M_Start
   - World Settings 中将 GameMode 设置为 SnakeGameMode
   - 在场景中放置 SnakeManager

   9. 构建要求
   - Build.cs 包含 Core、CoreUObject、Engine、InputCore、UMG、Slate、SlateCore。
   - 配置 DefaultInput.ini。
   - 代码要能编译通过。
   - 先实现核心功能，不做音效、粒子、多人模式等扩展功能。

   10. 输出方式
   请分阶段完成：
   - 先列出实施计划
   - 再创建 C++ 类和配置文件
   - 再说明蓝图和地图如何配置
   - 最后给出验收清单，并逐项自检

   验收标准：
   - 可以从空项目编译并运行
   - 能开始游戏
   - 蛇能移动、加速、吃食物、增长
   - 能触发隐身/无敌效果
   - 能生成障碍物
   - 边界可环绕
   - 碰撞逻辑正确
   - 游戏结束流程正确
   - 最高分 Top 10 可保存和读取