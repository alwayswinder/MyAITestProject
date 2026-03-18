# SnakeSaveGame（存档类）

## 文件路径
- 头文件: `SnakeSaveGame.h`
- 源文件: `SnakeSaveGame.cpp`

## 类说明

继承自 USaveGame，用于保存和加载游戏数据，主要保存最高分记录。由 SnakeManager 的 SaveHighScore/GetHighScores 管理。

## 主要功能

- 持久化最高分记录（最多 Top 10）
- 记录存档槽名称和用户索引

## 可配置属性（UPROPERTY，VisibleAnywhere）

| 属性名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| HighScores | TArray&lt;int32&gt; | 空数组 | 最高分列表（按降序排列） |
| SaveSlotName | FString | "SnakeHighScores" | 存档槽名称（固定，由 SnakeManager 使用） |
| UserIndex | uint32 | 0 | 用户索引（固定为 0） |

## 使用方式

由 SnakeManager 负责所有存档的读写：
- `SaveHighScore(int32)` → 读取存档 → 添加新分数 → 降序排序 → 截取前10条 → 写回存档
- `GetHighScores()` → 读取存档 → 返回 HighScores 数组
