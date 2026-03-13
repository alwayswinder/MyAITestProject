// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeSegment.h"
#include "Components/StaticMeshComponent.h"

ASnakeSegment::ASnakeSegment()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建网格组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void ASnakeSegment::BeginPlay()
{
	Super::BeginPlay();
	InitializeMesh();
}

void ASnakeSegment::InitializeMesh()
{
	// 这里可以设置蛇段的网格和材质
	// 暂时使用默认的立方体网格
	// 在实际项目中，你应该创建或导入一个合适的蛇段网格
}
