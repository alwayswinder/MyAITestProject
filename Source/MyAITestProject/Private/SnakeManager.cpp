// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeManager.h"
#include "Snake.h"
#include "Food.h"
#include "Kismet/GameplayStatics.h"

ASnakeManager::ASnakeManager()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建根组件
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// 创建边界可视化网格组件
	BoundaryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoundaryMesh"));
	BoundaryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoundaryMesh->SetCastShadow(false);
	BoundaryMesh->SetupAttachment(RootComponent);

	// 设置默认值
	Score = 0;
	bGameOver = false;
	FoodScoreValue = 10; // 默认每次吃到食物增加10分

	// 设置默认边界距离
	BoundaryDistanceX = 500.0f; // 默认左右各500单位
	BoundaryDistanceY = 500.0f; // 默认上下各500单位
}

// 移动UpdateBoundaryMesh方法到public部分
void ASnakeManager::UpdateBoundaryMesh()
{
	// 更新边界网格的缩放，使其与边界范围匹配
	// 假设基础网格的大小是100x100单位
	float ScaleX = BoundaryDistanceX * 2 / 100.0f;
	float ScaleY = BoundaryDistanceY * 2 / 100.0f;
	BoundaryMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
}

void ASnakeManager::BeginPlay()
{
	Super::BeginPlay();
	UpdateBoundaryMesh();
	InitializeGame();
}

void ASnakeManager::InitializeGame()
{
	Score = 0;
	bGameOver = false;
	SpawnSnake();
	SpawnFood();
}

void ASnakeManager::StartGame()
{
	if (Snake)
	{
		Snake->StartGame();
	}
}

void ASnakeManager::GameOver()
{
	bGameOver = true;
	if (Snake)
	{
		Snake->GameOver();
	}
}

void ASnakeManager::SpawnSnake()
{
	if (SnakeClass)
	{
		// 生成蛇在SnakeManager的当前位置
		FVector SpawnLocation = GetActorLocation();
		// 确保位置是100的倍数，与蛇段对齐
		SpawnLocation.X = FMath::RoundToFloat(SpawnLocation.X / 100.0f) * 100.0f;
		SpawnLocation.Y = FMath::RoundToFloat(SpawnLocation.Y / 100.0f) * 100.0f;
		Snake = GetWorld()->SpawnActor<ASnake>(SnakeClass, SpawnLocation, FRotator::ZeroRotator);
	}
}

void ASnakeManager::SpawnFood()
{
	if (FoodClass)
	{
		// 生成食物在SnakeManager的当前位置附近
		FVector ManagerLocation = GetActorLocation();
		
		// 计算边界，确保食物在游戏区域内
		float MinX = ManagerLocation.X - BoundaryDistanceX;
		float MaxX = ManagerLocation.X + BoundaryDistanceX;
		float MinY = ManagerLocation.Y - BoundaryDistanceY;
		float MaxY = ManagerLocation.Y + BoundaryDistanceY;
		
		// 生成随机位置
		float X = FMath::FRandRange(MinX, MaxX);
		float Y = FMath::FRandRange(MinY, MaxY);

		// 确保位置是100的倍数，与蛇段对齐
		X = FMath::RoundToFloat(X / 100.0f) * 100.0f;
		Y = FMath::RoundToFloat(Y / 100.0f) * 100.0f;

		FVector SpawnLocation = FVector(X, Y, 0);
		Food = GetWorld()->SpawnActor<AFood>(FoodClass, SpawnLocation, FRotator::ZeroRotator);
	}
}
