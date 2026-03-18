// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakePlayerController.h"
#include "Snake.h"
#include "Kismet/GameplayStatics.h"

ASnakePlayerController::ASnakePlayerController()
{
	// 设置默认属性
	Snake = nullptr;
	PressedKeyCount = 0;
	CurrentPressedDirection = FVector2D::ZeroVector;
}

void ASnakePlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ASnakePlayerController::SetSnake(ASnake* NewSnake)
{
	Snake = NewSnake;
}

void ASnakePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 绑定输入
	InputComponent->BindAction("MoveUp", IE_Pressed, this, &ASnakePlayerController::OnMoveUpPressed);
	InputComponent->BindAction("MoveUp", IE_Released, this, &ASnakePlayerController::OnMoveUpReleased);
	InputComponent->BindAction("MoveDown", IE_Pressed, this, &ASnakePlayerController::OnMoveDownPressed);
	InputComponent->BindAction("MoveDown", IE_Released, this, &ASnakePlayerController::OnMoveDownReleased);
	InputComponent->BindAction("MoveLeft", IE_Pressed, this, &ASnakePlayerController::OnMoveLeftPressed);
	InputComponent->BindAction("MoveLeft", IE_Released, this, &ASnakePlayerController::OnMoveLeftReleased);
	InputComponent->BindAction("MoveRight", IE_Pressed, this, &ASnakePlayerController::OnMoveRightPressed);
	InputComponent->BindAction("MoveRight", IE_Released, this, &ASnakePlayerController::OnMoveRightReleased);
}

void ASnakePlayerController::MoveUp()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(0, 1));
		CurrentPressedDirection = FVector2D(0, 1);
	}
}

void ASnakePlayerController::MoveDown()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(0, -1));
		CurrentPressedDirection = FVector2D(0, -1);
	}
}

void ASnakePlayerController::MoveLeft()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(1, 0));
		CurrentPressedDirection = FVector2D(1, 0);
	}
}

void ASnakePlayerController::MoveRight()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(-1, 0));
		CurrentPressedDirection = FVector2D(-1, 0);
	}
}

void ASnakePlayerController::HandleMovePressed(TFunction<void()> MoveFunc)
{
	MoveFunc();
	PressedKeyCount++;
}

void ASnakePlayerController::HandleMoveReleased()
{
	PressedKeyCount--;
	if (Snake && PressedKeyCount == 0)
	{
		Snake->ReleaseDirection();
	}
}

void ASnakePlayerController::OnMoveUpPressed()
{
	HandleMovePressed([this]() { MoveUp(); });
}

void ASnakePlayerController::OnMoveUpReleased()
{
	HandleMoveReleased();
}

void ASnakePlayerController::OnMoveDownPressed()
{
	HandleMovePressed([this]() { MoveDown(); });
}

void ASnakePlayerController::OnMoveDownReleased()
{
	HandleMoveReleased();
}

void ASnakePlayerController::OnMoveLeftPressed()
{
	HandleMovePressed([this]() { MoveLeft(); });
}

void ASnakePlayerController::OnMoveLeftReleased()
{
	HandleMoveReleased();
}

void ASnakePlayerController::OnMoveRightPressed()
{
	HandleMovePressed([this]() { MoveRight(); });
}

void ASnakePlayerController::OnMoveRightReleased()
{
	HandleMoveReleased();
}
