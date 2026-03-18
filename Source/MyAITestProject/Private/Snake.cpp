// Fill out your copyright notice in the Description page of Project Settings.

#include "Snake.h"
#include "SnakeSegment.h"
#include "SnakeManager.h"
#include "SnakeObstacle.h"
#include "Kismet/GameplayStatics.h"

ASnake::ASnake()
{
	// 设置Pawn属性
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// 创建蛇头网格组件
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HeadMesh->SetCastShadow(false);
	HeadMesh->SetupAttachment(RootComponent);

	MoveSpeed = 0.2f;
	BoostMoveSpeed = 0.1f; // 默认加速速度
	InitialSegmentCount = 3;
	GridSize = 100.0f; // 默认网格大小
	CurrentMoveSpeed = MoveSpeed;
	bIsBoosting = false;
	EffectDuration = 10.0f;
	bIsInvisible = false;
	bIsInvincible = false;
	EffectTimer = 0.0f;
	CurrentDirection = FVector2D::ZeroVector;
	PendingDirection = FVector2D::ZeroVector;
	LastMovedDirection = FVector2D::ZeroVector;
	
	// 设置默认边界距离
	BoundaryDistanceX = 500.0f; // 默认左右各500单位
	BoundaryDistanceY = 500.0f; // 默认上下各500单位
}

void ASnake::BeginPlay()
{
    Super::BeginPlay();
    GetBoundaryFromManager();
}

void ASnake::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateEffects(DeltaTime);
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
	PendingDirection = CurrentDirection; // 初始待处理方向与当前方向一致
	LastMovedDirection = CurrentDirection; // 初始上次移动方向与当前方向一致
	
	// 重置加速状态
	bIsBoosting = false;
	CurrentMoveSpeed = MoveSpeed;
	
	// 清除效果状态
	ClearEffects();
	
	// 设置初始朝向
	SetActorRotation(FRotator::ZeroRotator);

	// 生成初始蛇段
	SpawnInitialSegments();

	// 开始移动
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &ASnake::MoveSnake, CurrentMoveSpeed, true);
}

void ASnake::ChangeDirection(FVector2D NewDirection)
{
	// 防止蛇直接反向移动（使用LastMovedDirection检查，因为这是蛇最后一次真正移动的方向）
	if ((LastMovedDirection.X + NewDirection.X) != 0 || (LastMovedDirection.Y + NewDirection.Y) != 0)
	{
		PendingDirection = NewDirection;
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

void ASnake::ApplyFoodEffect(EFoodType EffectType)
{
	switch (EffectType)
	{
	case EFoodType::Normal:
		// 普通食物没有特殊效果
		break;
	case EFoodType::Invisible:
		bIsInvisible = true;
		bIsInvincible = false;
		EffectTimer = EffectDuration;
		OnInvisibleStarted.Broadcast();
		break;
	case EFoodType::Invincible:
		bIsInvincible = true;
		bIsInvisible = false;
		EffectTimer = EffectDuration;
		OnInvincibleStarted.Broadcast();
		break;
	}
}

void ASnake::UpdateEffects(float DeltaTime)
{
	if (EffectTimer > 0.0f)
	{
		EffectTimer -= DeltaTime;
		if (EffectTimer <= 0.0f)
		{
			ClearEffects();
		}
	}
}

void ASnake::ClearEffects()
{
	bool WasInvisible = bIsInvisible;
	bool WasInvincible = bIsInvincible;
	
	bIsInvisible = false;
	bIsInvincible = false;
	EffectTimer = 0.0f;
	
	if (WasInvisible)
	{
		OnInvisibleEnded.Broadcast();
	}
	if (WasInvincible)
	{
		OnInvincibleEnded.Broadcast();
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
	// 在移动前更新当前方向为待处理方向
	CurrentDirection = PendingDirection;
	
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

	// 更新上次移动的方向
	LastMovedDirection = CurrentDirection;

	// 重新设置计时器，使用当前的移动速度
	if (GetWorldTimerManager().IsTimerActive(MoveTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &ASnake::MoveSnake, CurrentMoveSpeed, true);
	}
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

	// 检查边界碰撞 - 任何状态下都检查
	if (HeadLocation.X < MinX || HeadLocation.X > MaxX || HeadLocation.Y < MinY || HeadLocation.Y > MaxY)
	{
		GameOver();
		if (SnakeManager)
		{
			SnakeManager->GameOver();
		}
		return;
	}

	// 检查自身碰撞 - 隐身状态下不检查自身碰撞
	if (!bIsInvisible)
	{
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

	// 检查障碍物碰撞
	TArray<AActor*> Obstacles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeObstacle::StaticClass(), Obstacles);
	for (AActor* Obstacle : Obstacles)
	{
		if (HeadLocation.Equals(Obstacle->GetActorLocation(), GridSize * 0.1f))
		{
			if (bIsInvincible)
			{
				// 无敌状态下摧毁障碍物
				Obstacle->Destroy();
			}
			else if (!bIsInvisible)
			{
				// 非隐身和非无敌状态下游戏结束
				GameOver();
				if (SnakeManager)
				{
					SnakeManager->GameOver();
				}
				return;
			}
		}
	}
}

void ASnake::StartBoost()
{
	if (!bIsBoosting)
	{
		bIsBoosting = true;
		CurrentMoveSpeed = BoostMoveSpeed;
		
		// 不重置移动计时器，只修改移动速度
		// 下一次移动会自动使用新的速度
	}
}

void ASnake::StopBoost()
{
	if (bIsBoosting)
	{
		bIsBoosting = false;
		CurrentMoveSpeed = MoveSpeed;
		
		// 不重置移动计时器，只修改移动速度
		// 下一次移动会自动使用新的速度
	}
}

bool ASnake::IsPositionOccupiedBySnake(FVector Position, float Tolerance)
{
	FVector HeadLocation = GetActorLocation();
	if (FVector::Dist(Position, HeadLocation) < Tolerance)
	{
		return true;
	}

	for (ASnakeSegment* Segment : SnakeSegments)
	{
		if (Segment)
		{
			FVector SegmentLocation = Segment->GetActorLocation();
			if (FVector::Dist(Position, SegmentLocation) < Tolerance)
			{
				return true;
			}
		}
	}

	return false;
}