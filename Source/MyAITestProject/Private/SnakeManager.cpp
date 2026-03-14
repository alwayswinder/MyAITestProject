// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeManager.h"
#include "Food.h"
#include "SnakeObstacle.h"
#include "SnakeSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Snake.h"
#include "SnakePlayerController.h"
#include "SnakeHUD.h"
#include "SnakeMenuUI.h"
#include "SnakeGameUI.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ASnakeManager::ASnakeManager() {
  PrimaryActorTick.bCanEverTick = false;

  // 创建根组件
  USceneComponent *RootComp =
      CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  RootComponent = RootComp;

  // 创建SpringArm组件
  SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
  SpringArmComponent->SetupAttachment(RootComponent);
  SpringArmComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
  SpringArmComponent->bDoCollisionTest = false;

  // 创建Camera组件
  CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
  CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);

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
  UpdateCameraDistance();
}

void ASnakeManager::UpdateBoundaryMesh() {
  // 更新边界网格的缩放，使其与边界范围匹配
  float ScaleX = BoundaryDistanceX * 2 / GridSize;
  float ScaleY = BoundaryDistanceY * 2 / GridSize;
  BoundaryMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
}

void ASnakeManager::UpdateCameraDistance() {
  if (SpringArmComponent && CameraComponent) {
    // 计算需要的相机距离，确保能看到整个游戏区域
    // 使用较大的边界距离，并添加一些边距
    float MaxBoundary = FMath::Max(BoundaryDistanceX, BoundaryDistanceY);
    float Margin = MaxBoundary * 0.5f; // 50% 边距
    float RequiredDistance = MaxBoundary + Margin;
    
    // 设置相机距离
    SpringArmComponent->TargetArmLength = RequiredDistance;
  }
}

void ASnakeManager::BeginPlay() {
  Super::BeginPlay();
  UpdateBoundaryMesh();
  UpdateCameraDistance();
  InitializeGame();
}

void ASnakeManager::InitializeGame() {
  // 切换Viewport到Manager的Camera
  APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  if (PlayerController && CameraComponent)
  {
    PlayerController->SetViewTarget(this);
  }
}

void ASnakeManager::StartGame() {
  Score = 0;
  bGameOver = false;
  ClearObstacles();
  
  SpawnSnake();
  SpawnFood();
  
  APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  if (PlayerController)
  {
    AHUD* HUD = PlayerController->GetHUD();
    if (HUD)
    {
      ASnakeHUD* SnakeHUD = Cast<ASnakeHUD>(HUD);
      if (SnakeHUD)
      {
        USnakeGameUI* GameUI = SnakeHUD->GetSnakeGameUI();
        if (GameUI)
        {
          GameUI->ShowUI();
        }
      }
    }
  }
  
  if (Snake) {
    Snake->StartGame();
  }
}

void ASnakeManager::GameOver() {
  bGameOver = true;
  
  SaveHighScore(Score);
  
  if (Snake) {
    Snake->GameOver();
  }
  
  // 隐藏游戏UI
  APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  if (PlayerController)
  {
    AHUD* HUD = PlayerController->GetHUD();
    if (HUD)
    {
      ASnakeHUD* SnakeHUD = Cast<ASnakeHUD>(HUD);
      if (SnakeHUD)
      {
        USnakeGameUI* GameUI = SnakeHUD->GetSnakeGameUI();
        if (GameUI)
        {
          GameUI->HideUI();
        }
      }
    }
  }
  
  // 延迟一帧显示UI，确保一切都稳定了
  FTimerHandle TimerHandle;
  GetWorldTimerManager().SetTimer(TimerHandle, this, &ASnakeManager::DelayedShowGameOverUI, 0.1f, false);
}

void ASnakeManager::DelayedShowGameOverUI()
{
  // 从HUD获取UI并重新显示，标记为GameOver状态
  APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  if (PlayerController)
  {
    AHUD* HUD = PlayerController->GetHUD();
    if (HUD)
    {
      ASnakeHUD* SnakeHUD = Cast<ASnakeHUD>(HUD);
      if (SnakeHUD)
      {
        USnakeMenuUI* UI = SnakeHUD->GetSnakeMenuUI();
        if (UI)
        {
          UI->ShowUI(true);
        }
      }
    }
  }
  
  if (Snake)
  {
    Snake->Destroy();
    Snake = nullptr;
  }
  if (Food)
  {
    Food->Destroy();
    Food = nullptr;
  }
  ClearObstacles();
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
    
    // 将Snake设置到PlayerController
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && Snake)
    {
      ASnakePlayerController* SnakePC = Cast<ASnakePlayerController>(PlayerController);
      if (SnakePC)
      {
        SnakePC->SetSnake(Snake);
      }
    }
  }
}

void ASnakeManager::SpawnFood() {
  if (FoodClass) {
    FVector SpawnLocation = GetRandomValidPosition();
    Food = GetWorld()->SpawnActor<AFood>(FoodClass, SpawnLocation, FRotator::ZeroRotator);
  }
}

void ASnakeManager::SpawnObstacle() {
  if (ObstacleClass) {
    FVector SpawnLocation = GetRandomValidPosition();
    ASnakeObstacle* NewObstacle = GetWorld()->SpawnActor<ASnakeObstacle>(ObstacleClass, SpawnLocation, FRotator::ZeroRotator);
    if (NewObstacle) {
      Obstacles.Add(NewObstacle);
    }
  }
}

void ASnakeManager::ClearObstacles() {
  for (ASnakeObstacle* Obstacle : Obstacles) {
    if (Obstacle && Obstacle->IsValidLowLevel()) {
      Obstacle->Destroy();
    }
  }
  Obstacles.Empty();
}

bool ASnakeManager::IsPositionOccupied(FVector Position) {
  const float Tolerance = GridSize * 0.5f;

  if (Food) {
    FVector FoodLocation = Food->GetActorLocation();
    if (FVector::Dist(Position, FoodLocation) < Tolerance) {
      return true;
    }
  }

  if (Snake) {
    if (Snake->IsPositionOccupiedBySnake(Position, Tolerance)) {
      return true;
    }
  }

  for (ASnakeObstacle* Obstacle : Obstacles) {
    if (Obstacle) {
      FVector ObstacleLocation = Obstacle->GetActorLocation();
      if (FVector::Dist(Position, ObstacleLocation) < Tolerance) {
        return true;
      }
    }
  }

  return false;
}

FVector ASnakeManager::GetRandomValidPosition() {
  FVector ManagerLocation = GetActorLocation();
  float MinX = ManagerLocation.X - BoundaryDistanceX;
  float MaxX = ManagerLocation.X + BoundaryDistanceX;
  float MinY = ManagerLocation.Y - BoundaryDistanceY;
  float MaxY = ManagerLocation.Y + BoundaryDistanceY;

  const int32 MaxAttempts = 100;
  for (int32 i = 0; i < MaxAttempts; ++i) {
    float X = FMath::FRandRange(MinX, MaxX);
    float Y = FMath::FRandRange(MinY, MaxY);
    float Z = ManagerLocation.Z;

    X = FMath::RoundToFloat(X / GridSize) * GridSize;
    Y = FMath::RoundToFloat(Y / GridSize) * GridSize;

    FVector CandidatePosition(X, Y, Z);

    if (!IsPositionOccupied(CandidatePosition)) {
      return CandidatePosition;
    }
  }

  return ManagerLocation;
}

void ASnakeManager::SaveHighScore(int32 NewScore)
{
  FString SaveSlotName = TEXT("SnakeHighScores");
  const int32 MaxHighScores = 10;

  USnakeSaveGame* SaveGameInstance = Cast<USnakeSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
  if (!SaveGameInstance)
  {
    SaveGameInstance = Cast<USnakeSaveGame>(UGameplayStatics::CreateSaveGameObject(USnakeSaveGame::StaticClass()));
  }

  if (SaveGameInstance)
  {
    SaveGameInstance->HighScores.Add(NewScore);
    SaveGameInstance->HighScores.Sort([](int32 A, int32 B) {
      return A > B;
    });

    while (SaveGameInstance->HighScores.Num() > MaxHighScores)
    {
      SaveGameInstance->HighScores.Pop();
    }

    UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0);
  }
}

TArray<int32> ASnakeManager::GetHighScores()
{
  FString SaveSlotName = TEXT("SnakeHighScores");

  USnakeSaveGame* SaveGameInstance = Cast<USnakeSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
  if (SaveGameInstance)
  {
    return SaveGameInstance->HighScores;
  }

  return TArray<int32>();
}
