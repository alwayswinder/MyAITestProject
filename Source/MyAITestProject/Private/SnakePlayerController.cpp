// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakePlayerController.h"
#include "Snake.h"
#include "Kismet/GameplayStatics.h"

ASnakePlayerController::ASnakePlayerController()
{
	// 设置默认属性
	Snake = nullptr;
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
	InputComponent->BindAction("MoveUp", IE_Pressed, this, &ASnakePlayerController::MoveUp);
	InputComponent->BindAction("MoveDown", IE_Pressed, this, &ASnakePlayerController::MoveDown);
	InputComponent->BindAction("MoveLeft", IE_Pressed, this, &ASnakePlayerController::MoveLeft);
	InputComponent->BindAction("MoveRight", IE_Pressed, this, &ASnakePlayerController::MoveRight);
}

void ASnakePlayerController::MoveUp()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(0, 1));
	}
}

void ASnakePlayerController::MoveDown()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(0, -1));
	}
}

void ASnakePlayerController::MoveLeft()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(1, 0));
	}
}

void ASnakePlayerController::MoveRight()
{
	if (Snake)
	{
		Snake->ChangeDirection(FVector2D(-1, 0));
	}
}
