// Fill out your copyright notice in the Description page of Project Settings.

#include "Snake.h"
#include "SnakeSegment.h"
#include "SnakeManager.h"
#include "Kismet/GameplayStatics.h"

ASnake::ASnake()
{
	// 设置Pawn属性
	PrimaryActorTick.bCanEverTick = false;

	// 创建根组件
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	MoveSpeed = 0.2f;
	InitialSegmentCount = 3;
	
	// 设置默认边界距离
	BoundaryDistanceX = 500.0f; // 默认左右各500单位
	BoundaryDistanceY = 500.0f; // 默认上下各500单位
}

void ASnake::BeginPlay()
{
    Super::BeginPlay();
    GetBoundaryFromManager();
    StartGame();
}

void ASnake::GetBoundaryFromManager()
{
    // 找到场景中的SnakeManager
    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeManager::StaticClass(), Managers);
    
    if (Managers.Num() > 0)
    {
        SnakeManager = Cast<ASnakeManager>(Managers[0]);
        if (SnakeManager)
        {
            // 从SnakeManager获取边界距离
            BoundaryDistanceX = SnakeManager->BoundaryDistanceX;
            BoundaryDistanceY = SnakeManager->BoundaryDistanceY;
        }
    }
}


void ASnake::StartGame()
{
	// 清除现有蛇段
	for (ASnakeSegment* Segment : SnakeSegments)
	{
		if (Segment)
		{
			Segment->Destroy();
		}
	}
	SnakeSegments.Empty();

	// 初始化方向
	CurrentDirection = FVector2D(1, 0); // 初始向右移动

	// 生成初始蛇段
	SpawnInitialSegments();

	// 开始移动
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &ASnake::MoveSnake, MoveSpeed, true);
}

void ASnake::ChangeDirection(FVector2D NewDirection)
{
	// 防止蛇直接反向移动
	if ((CurrentDirection.X + NewDirection.X) != 0 || (CurrentDirection.Y + NewDirection.Y) != 0)
	{
		CurrentDirection = NewDirection;
	}
}

void ASnake::EatFood()
{
	// 蛇吃到食物后，在尾部添加一个新段
	if (SnakeSegments.Num() > 0)
	{
		ASnakeSegment* TailSegment = SnakeSegments.Last();
		FVector TailLocation = TailSegment->GetActorLocation();

		ASnakeSegment* NewSegment = GetWorld()->SpawnActor<ASnakeSegment>(SnakeSegmentClass, TailLocation, FRotator::ZeroRotator);
		if (NewSegment)
		{
			SnakeSegments.Add(NewSegment);
		}
	}
}

void ASnake::GameOver()
{
	// 停止移动
	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
}

void ASnake::MoveSnake()
{
	if (SnakeSegments.Num() == 0)
	{
		return;
	}

	// 移动蛇头
	ASnakeSegment* HeadSegment = SnakeSegments[0];
	FVector NewHeadLocation = HeadSegment->GetActorLocation();
	NewHeadLocation.X += CurrentDirection.X * 100; // 假设每个段的大小是100单位
	NewHeadLocation.Y += CurrentDirection.Y * 100;

	// 移动身体段
	for (int32 i = SnakeSegments.Num() - 1; i > 0; i--)
	{
		ASnakeSegment* CurrentSegment = SnakeSegments[i];
		ASnakeSegment* PreviousSegment = SnakeSegments[i - 1];
		CurrentSegment->SetActorLocation(PreviousSegment->GetActorLocation());
	}

	// 设置新的蛇头位置
	HeadSegment->SetActorLocation(NewHeadLocation);

	// 检查碰撞
	CheckCollision();
}

void ASnake::SpawnInitialSegments()
{
	// 使用Snake的当前位置作为起点
	FVector SpawnLocation = GetActorLocation();

	for (int32 i = 0; i < InitialSegmentCount; i++)
	{
		ASnakeSegment* Segment = GetWorld()->SpawnActor<ASnakeSegment>(SnakeSegmentClass, SpawnLocation, FRotator::ZeroRotator);
		if (Segment)
		{
			SnakeSegments.Add(Segment);
			SpawnLocation.X -= 100; // 每个段间隔100单位
		}
	}
}

void ASnake::CheckCollision()
{
	if (SnakeSegments.Num() == 0)
	{
		return;
	}

	ASnakeSegment* HeadSegment = SnakeSegments[0];
	FVector HeadLocation = HeadSegment->GetActorLocation();

	// 计算实际边界
	float MinX, MaxX, MinY, MaxY;
	if (SnakeManager)
	{
		// 使用SnakeManager的位置和边界距离计算边界
		FVector ManagerLocation = SnakeManager->GetActorLocation();
		MinX = ManagerLocation.X - BoundaryDistanceX;
		MaxX = ManagerLocation.X + BoundaryDistanceX;
		MinY = ManagerLocation.Y - BoundaryDistanceY;
		MaxY = ManagerLocation.Y + BoundaryDistanceY;
	}
	else
	{
		// 使用Snake的位置和边界距离计算边界
		FVector SnakeLocation = GetActorLocation();
		MinX = SnakeLocation.X - BoundaryDistanceX;
		MaxX = SnakeLocation.X + BoundaryDistanceX;
		MinY = SnakeLocation.Y - BoundaryDistanceY;
		MaxY = SnakeLocation.Y + BoundaryDistanceY;
	}

	// 检查边界碰撞
	if (HeadLocation.X < MinX || HeadLocation.X > MaxX || HeadLocation.Y < MinY || HeadLocation.Y > MaxY)
	{
		GameOver();
		if (SnakeManager)
		{
			SnakeManager->GameOver();
		}
		return;
	}

	// 检查自身碰撞
	for (int32 i = 1; i < SnakeSegments.Num(); i++)
	{
		ASnakeSegment* Segment = SnakeSegments[i];
		if (HeadLocation.Equals(Segment->GetActorLocation(), 10))
		{
			GameOver();
			if (SnakeManager)
			{
				SnakeManager->GameOver();
			}
			return;
		}
	}
}