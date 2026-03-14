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

	// 创建蛇头网格组件
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->SetCastShadow(false);
	HeadMesh->SetupAttachment(RootComponent);

	MoveSpeed = 0.2f;
	InitialSegmentCount = 3;
	GridSize = 100.0f; // 默认网格大小
	
	// 设置默认边界距离
	BoundaryDistanceX = 500.0f; // 默认左右各500单位
	BoundaryDistanceY = 500.0f; // 默认上下各500单位
}

void ASnake::BeginPlay()
{
    Super::BeginPlay();
    GetBoundaryFromManager();
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
            // 从SnakeManager获取边界距离和网格大小
            BoundaryDistanceX = SnakeManager->BoundaryDistanceX;
            BoundaryDistanceY = SnakeManager->BoundaryDistanceY;
            GridSize = SnakeManager->GridSize;
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
	
	// 设置初始朝向
	SetActorRotation(FRotator::ZeroRotator);

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
		// 如果有身体段，在最后一个身体段的位置添加新段
		ASnakeSegment* TailSegment = SnakeSegments.Last();
		FVector TailLocation = TailSegment->GetActorLocation();

		ASnakeSegment* NewSegment = GetWorld()->SpawnActor<ASnakeSegment>(SnakeSegmentClass, TailLocation, FRotator::ZeroRotator);
		if (NewSegment)
		{
			SnakeSegments.Add(NewSegment);
		}
	}
	else
	{
		// 如果没有身体段，在蛇头后面添加第一个身体段
		FVector HeadLocation = GetActorLocation();
		FVector TailLocation = HeadLocation;
		TailLocation.X -= GridSize;

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

void ASnake::Destroyed()
{
	Super::Destroyed();
	
	// 销毁所有蛇段
	for (ASnakeSegment* Segment : SnakeSegments)
	{
		if (Segment)
		{
			Segment->Destroy();
		}
	}
	SnakeSegments.Empty();
}

void ASnake::MoveSnake()
{
	// 记录当前蛇头位置（Snake actor的位置）
	FVector OldHeadLocation = GetActorLocation();
	
	// 根据移动方向设置旋转
	FRotator NewRotation = FRotator::ZeroRotator;
	if (CurrentDirection.X > 0)
	{
		NewRotation.Yaw = 0.0f; // 向右
	}
	else if (CurrentDirection.X < 0)
	{
		NewRotation.Yaw = 180.0f; // 向左
	}
	else if (CurrentDirection.Y > 0)
	{
		NewRotation.Yaw = 90.0f; // 向上
	}
	else if (CurrentDirection.Y < 0)
	{
		NewRotation.Yaw = -90.0f; // 向下
	}
	SetActorRotation(NewRotation);
	
	// 移动蛇头（Snake actor本身）
	FVector NewHeadLocation = OldHeadLocation;
	NewHeadLocation.X += CurrentDirection.X * GridSize;
	NewHeadLocation.Y += CurrentDirection.Y * GridSize;
	SetActorLocation(NewHeadLocation);

	// 移动身体段 - 从后往前移动
	for (int32 i = SnakeSegments.Num() - 1; i >= 0; i--)
	{
		ASnakeSegment* CurrentSegment = SnakeSegments[i];
		if (i == 0)
		{
			// 第一个身体段移动到原来的蛇头位置
			CurrentSegment->SetActorLocation(OldHeadLocation);
		}
		else
		{
			// 其他身体段移动到前一个段的原位置
			ASnakeSegment* PreviousSegment = SnakeSegments[i - 1];
			CurrentSegment->SetActorLocation(PreviousSegment->GetActorLocation());
		}
	}

	// 检查碰撞
	CheckCollision();
}

void ASnake::SpawnInitialSegments()
{
	// 使用Snake的当前位置作为起点
	FVector SpawnLocation = GetActorLocation();

	// 只生成身体段，不生成蛇头（蛇头是Snake actor本身）
	// 从InitialSegmentCount - 1开始，因为蛇头已经是Snake actor
	for (int32 i = 0; i < InitialSegmentCount - 1; i++)
	{
		// 第一个身体段在蛇头后面，然后依次排列
		SpawnLocation.X -= GridSize;
		ASnakeSegment* Segment = GetWorld()->SpawnActor<ASnakeSegment>(SnakeSegmentClass, SpawnLocation, FRotator::ZeroRotator);
		if (Segment)
		{
			SnakeSegments.Add(Segment);
		}
	}
}

void ASnake::CheckCollision()
{
	// 使用Snake actor的位置作为蛇头位置
	FVector HeadLocation = GetActorLocation();

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
	for (int32 i = 0; i < SnakeSegments.Num(); i++)
	{
		ASnakeSegment* Segment = SnakeSegments[i];
		if (HeadLocation.Equals(Segment->GetActorLocation(), GridSize * 0.1f))
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