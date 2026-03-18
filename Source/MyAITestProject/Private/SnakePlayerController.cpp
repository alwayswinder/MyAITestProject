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
	
	// 如果是第一个按键，启动定时器
	if (Snake && PressedKeyCount == 1)
	{
		// 保存当前按下的方向
		FVector2D PressedDirection = CurrentPressedDirection;
		
		// 启动定时器，0.2秒后检查是否依然按下
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, PressedDirection]() {
			if (Snake && PressedKeyCount > 0 && CurrentPressedDirection == PressedDirection && PressedDirection == Snake->GetCurrentDirection())
			{
				// 如果依然按下且方向一致，进入加速状态
				Snake->StartBoost();
			}
		});
	}
}

void ASnakePlayerController::HandleMoveReleased()
{
	PressedKeyCount--;
	if (Snake && PressedKeyCount == 0)
	{
		Snake->StopBoost();
	}
	else if (Snake && PressedKeyCount > 0)
	{
		// 检查是否还有同方向的按键
		if (CurrentPressedDirection == Snake->GetCurrentDirection())
		{
			// 如果还有同方向的按键，继续加速
			Snake->StartBoost();
		}
		else
		{
			// 如果没有同方向的按键，停止加速
			Snake->StopBoost();
		}
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
