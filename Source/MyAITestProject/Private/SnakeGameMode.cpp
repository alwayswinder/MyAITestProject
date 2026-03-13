// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeGameMode.h"
#include "SnakePlayerController.h"

ASnakeGameMode::ASnakeGameMode()
{
	// 设置默认的PlayerController类
	PlayerControllerClass = ASnakePlayerController::StaticClass();
}

void ASnakeGameMode::BeginPlay()
{
	Super::BeginPlay();
	// 游戏模式初始化
	// 具体的游戏逻辑由SnakeManager处理
}