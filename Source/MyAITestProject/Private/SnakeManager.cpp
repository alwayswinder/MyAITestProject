// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeManager.h"
#include "Food.h"
#include "Kismet/GameplayStatics.h"
#include "Snake.h"

ASnakeManager::ASnakeManager() {
  PrimaryActorTick.bCanEverTick = false;

  // 创建根组件
  USceneComponent *RootComp =
      CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  RootComponent = RootComp;

  // 创建边界可视化网格组件
  BoundaryMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoundaryMesh"));
  BoundaryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  BoundaryMesh->SetCastShadow(false);
  BoundaryMesh->SetupAttachment(RootComponent);

  // 设置默认值
  Score = 0;
  bGameOver = false;
  FoodScoreValue = 10; // 默认每次吃到食物增加10分
  GridSize = 100.0f; // 默认网格大小为100单位

  // 设置默认边界距离
  BoundaryDistanceX = 500.0f; // 默认左右各500单位
  BoundaryDistanceY = 500.0f; // 默认上下各500单位

}

void ASnakeManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);
  UpdateBoundaryMesh();
}

void ASnakeManager::UpdateBoundaryMesh() {
  // 更新边界网格的缩放，使其与边界范围匹配
  float ScaleX = BoundaryDistanceX * 2 / GridSize;
  float ScaleY = BoundaryDistanceY * 2 / GridSize;
  BoundaryMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
}

void ASnakeManager::BeginPlay() {
  Super::BeginPlay();
  UpdateBoundaryMesh();
  InitializeGame();
}

void ASnakeManager::InitializeGame() {
  Score = 0;
  bGameOver = false;
  SpawnSnake();
  SpawnFood();
}

void ASnakeManager::StartGame() {
  if (Snake) {
    Snake->StartGame();
  }
}

void ASnakeManager::GameOver() {
  bGameOver = true;
  if (Snake) {
    Snake->GameOver();
  }
}

void ASnakeManager::SpawnSnake() {
  if (SnakeClass) {
    // 生成蛇在SnakeManager的当前位置
    FVector SpawnLocation = GetActorLocation();
    // 确保位置是GridSize的倍数，与蛇段对齐
    SpawnLocation.X = FMath::RoundToFloat(SpawnLocation.X / GridSize) * GridSize;
    SpawnLocation.Y = FMath::RoundToFloat(SpawnLocation.Y / GridSize) * GridSize;
    Snake = GetWorld()->SpawnActor<ASnake>(SnakeClass, SpawnLocation,
                                           FRotator::ZeroRotator);
  }
}

void ASnakeManager::SpawnFood() {
  if (FoodClass) {
    // 只生成Food actor，位置由Food自己处理
    Food = GetWorld()->SpawnActor<AFood>(FoodClass, GetActorLocation(),
                                         FRotator::ZeroRotator);
  }
}
