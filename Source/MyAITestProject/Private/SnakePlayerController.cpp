// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakePlayerController.h"
#include "Snake.h"
#include "Kismet/GameplayStatics.h"

ASnakePlayerController::ASnakePlayerController()
{
	// 设置默认属性
	Snake = nullptr;
	PressedKeyCount = 0;
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

void ASnakePlayerController::OnMoveUpPressed()
{
	MoveUp();
	PressedKeyCount++;
	if (Snake && PressedKeyCount == 1)
	{
		Snake->StartBoost();
	}
}

void ASnakePlayerController::OnMoveUpReleased()
{
	PressedKeyCount--;
	if (Snake && PressedKeyCount == 0)
	{
		Snake->StopBoost();
	}
}

void ASnakePlayerController::OnMoveDownPressed()
{
	MoveDown();
	PressedKeyCount++;
	if (Snake && PressedKeyCount == 1)
	{
		Snake->StartBoost();
	}
}

void ASnakePlayerController::OnMoveDownReleased()
{
	PressedKeyCount--;
	if (Snake && PressedKeyCount == 0)
	{
		Snake->StopBoost();
	}
}

void ASnakePlayerController::OnMoveLeftPressed()
{
	MoveLeft();
	PressedKeyCount++;
	if (Snake && PressedKeyCount == 1)
	{
		Snake->StartBoost();
	}
}

void ASnakePlayerController::OnMoveLeftReleased()
{
	PressedKeyCount--;
	if (Snake && PressedKeyCount == 0)
	{
		Snake->StopBoost();
	}
}

void ASnakePlayerController::OnMoveRightPressed()
{
	MoveRight();
	PressedKeyCount++;
	if (Snake && PressedKeyCount == 1)
	{
		Snake->StartBoost();
	}
}

void ASnakePlayerController::OnMoveRightReleased()
{
	PressedKeyCount--;
	if (Snake && PressedKeyCount == 0)
	{
		Snake->StopBoost();
	}
}
