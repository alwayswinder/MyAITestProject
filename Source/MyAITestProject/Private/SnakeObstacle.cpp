// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeObstacle.h"
#include "Components/StaticMeshComponent.h"
#include "SnakeManager.h"
#include "Kismet/GameplayStatics.h"

ASnakeObstacle::ASnakeObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

	GridSize = 100.0f;
}

void ASnakeObstacle::BeginPlay()
{
	Super::BeginPlay();
	FindSnakeManager();
	InitializeMesh();
}

void ASnakeObstacle::FindSnakeManager()
{
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

void ASnakeObstacle::InitializeMesh()
{
}
