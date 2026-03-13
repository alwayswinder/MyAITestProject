// Fill out your copyright notice in the Description page of Project Settings.

#include "Food.h"
#include "Components/StaticMeshComponent.h"
#include "Snake.h"
#include "SnakeManager.h"
#include "Kismet/GameplayStatics.h"

AFood::AFood()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建网格组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 设置碰撞属性
	MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AFood::OnBeginOverlap);
}

void AFood::BeginPlay()
{
	Super::BeginPlay();
	FindSnakeManager();
	InitializeMesh();
	SpawnAtRandomLocation();
}

void AFood::FindSnakeManager()
{
	// 找到场景中的SnakeManager
	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeManager::StaticClass(), Managers);
	
	if (Managers.Num() > 0)
	{
		SnakeManager = Cast<ASnakeManager>(Managers[0]);
	}
}

void AFood::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 检查是否与蛇头碰撞
	// 这里需要根据实际的蛇头类进行调整
	// 暂时假设蛇头也是ASnakeSegment类型
	ASnake* Snake = Cast<ASnake>(OtherActor->GetParentActor());
	if (Snake)
	{
		// 蛇吃到食物
		Snake->EatFood();

		// 重新生成食物
		SpawnAtRandomLocation();

		// 更新分数
		if (SnakeManager)
		{
			SnakeManager->Score += SnakeManager->FoodScoreValue;
		}
	}
}

void AFood::SpawnAtRandomLocation()
{
	// 生成随机位置
	float X, Y;
	
	if (SnakeManager)
	{
		// 使用SnakeManager的位置和边界距离计算边界
		FVector ManagerLocation = SnakeManager->GetActorLocation();
		float MinX = ManagerLocation.X - SnakeManager->BoundaryDistanceX;
		float MaxX = ManagerLocation.X + SnakeManager->BoundaryDistanceX;
		float MinY = ManagerLocation.Y - SnakeManager->BoundaryDistanceY;
		float MaxY = ManagerLocation.Y + SnakeManager->BoundaryDistanceY;
		
		// 生成随机位置
		X = FMath::FRandRange(MinX, MaxX);
		Y = FMath::FRandRange(MinY, MaxY);
	}
	else
	{
		// 默认边界
		X = FMath::FRandRange(-400.0f, 400.0f);
		Y = FMath::FRandRange(-400.0f, 400.0f);
	}

	// 确保位置是100的倍数，与蛇段对齐
	X = FMath::RoundToFloat(X / 100.0f) * 100.0f;
	Y = FMath::RoundToFloat(Y / 100.0f) * 100.0f;

	SetActorLocation(FVector(X, Y, 0));
}

void AFood::InitializeMesh()
{
	// 这里可以设置食物的网格和材质
	// 暂时使用默认的球体网格
	// 在实际项目中，你应该创建或导入一个合适的食物网格
}
