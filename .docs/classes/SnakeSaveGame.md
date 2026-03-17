# SnakeSaveGame（存档类）

## 文件路径
- 头文件: `SnakeSaveGame.h`
- 源文件: `SnakeSaveGame.cpp`

## 类说明

继承自USaveGame，用于保存和加载游戏数据，主要保存最高分记录。

## 主要功能

- 保存最高分记录
- 管理存档槽名称和用户索引

## 可配置属性

| 属性名 | 类型 | 说明 |
|--------|------|------|
| HighScores | TArray&lt;int32&gt; | 最高分列表 |
| SaveSlotName | FString | 存档槽名称，默认值为"SnakeHighScores" |
| UserIndex | uint32 | 用户索引，默认值为0 |
