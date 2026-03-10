@echo off
:: 关闭命令回显（避免重复显示执行的命令）
chcp 65001 >nul
:: 设置编码为 UTF-8，避免中文乱码

cd %1

copilot -i "Perform the task as described in /Plugins/AIBpAnalyze/ai-tools/M_Task.md."  --allow-all
