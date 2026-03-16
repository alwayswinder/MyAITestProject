// Fill out your copyright notice in the Description page of Project Settings.

#include "Food.h"
#include "Components/StaticMeshComponent.h"
#include "Snake.h"
#include "SnakeManager.h"
#include "Kismet/GameplayStatics.h"

AFood::AFood()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建网格组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 设置碰撞属性
	MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AFood::OnBeginOverlap);

	GridSize = 100.0f; // 默认网格大小
	bIsCollecting = false;
	AnimationTimer = 0.0f;
	AnimationDuration = 0.5f;
	BounceHeight = 150.0f;
	FoodType = EFoodType::Normal;
}

void AFood::BeginPlay()
{
	Super::BeginPlay();
	FindSnakeManager();
	InitializeMesh();
	RandomizeFoodType();
	SpawnAtRandomLocation();
}

void AFood::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCollecting)
	{
		UpdateCollectAnimation(DeltaTime);
	}
}

void AFood::FindSnakeManager()
{
	// 找到场景中的SnakeManager
	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeManager::StaticClass(), Managers);
	
	if (Managers.Num() > 0)
	{
		SnakeManager = Cast<ASnakeManager>(Managers[0]);
		if (SnakeManager)
		{
			GridSize = SnakeManager->GridSize;
		}
	}
}

void AFood::RandomizeFoodType()
{
	int32 RandomIndex = FMath::RandRange(0, 9);
	switch (RandomIndex)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		FoodType = EFoodType::Normal;
		break;
	case 7:
		FoodType = EFoodType::Invisible;
		break;
	case 8:
	case 9:
		FoodType = EFoodType::Invincible;
		break;
	}
	OnFoodTypeChanged.Broadcast(FoodType);
}

void AFood::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 检查是否与蛇碰撞
	ASnake* Snake = Cast<ASnake>(OtherActor);
	if (!Snake && OtherActor)
	{
		// 如果OtherActor不是Snake，检查它的Owner
		Snake = Cast<ASnake>(OtherActor->GetOwner());
	}
	
	if (Snake && !bIsCollecting)
	{
		// 蛇吃到食物
		Snake->EatFood();
		Snake->ApplyFoodEffect(FoodType);

		// 更新分数
		if (SnakeManager)
		{
			SnakeManager->Score += SnakeManager->FoodScoreValue;
		}

		// 开始收集动画
		StartCollectAnimation();
	}
}

void AFood::StartCollectAnimation()
{
	bIsCollecting = true;
	AnimationTimer = 0.0f;
	InitialLocation = GetActorLocation();
	InitialScale = GetActorScale3D();

	// 禁用碰撞
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFood::UpdateCollectAnimation(float DeltaTime)
{
	AnimationTimer += DeltaTime;
	float AnimationProgress = FMath::Clamp(AnimationTimer / AnimationDuration, 0.0f, 1.0f);

	// 弹跳效果（使用正弦函数）
	float BounceProgress = FMath::Sin(AnimationProgress * PI);
	float CurrentHeight = BounceHeight * BounceProgress;

	// 缩小效果（线性缩小到0）
	float ScaleProgress = 1.0f - AnimationProgress;
	FVector CurrentScale = InitialScale * ScaleProgress;

	// 应用变换
	FVector NewLocation = InitialLocation;
	NewLocation.Z += CurrentHeight;
	SetActorLocation(NewLocation);
	SetActorScale3D(CurrentScale);

	// 动画结束
	if (AnimationProgress >= 1.0f)
	{
		bIsCollecting = false;
		// 随机新的食物类型
		RandomizeFoodType();
		// 重新生成食物
		SpawnAtRandomLocation();
		// 重置变换
		SetActorScale3D(InitialScale);
		// 重新启用碰撞
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		
		// 生成障碍物
		if (SnakeManager)
		{
			SnakeManager->SpawnObstacle();
		}
	}
}

void AFood::SpawnAtRandomLocation()
{
	if (SnakeManager)
	{
		FVector SpawnLocation = SnakeManager->GetRandomValidPosition();
		SetActorLocation(SpawnLocation);
	}
}

void AFood::InitializeMesh()
{
	// 这里可以设置食物的网格和材质
	// 暂时使用默认的球体网格
	// 在实际项目中，你应该创建或导入一个合适的食物网格
}
