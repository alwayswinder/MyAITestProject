// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeMenuUI.h"
#include "Components/Button.h"
#include "SnakeManager.h"
#include "Kismet/GameplayStatics.h"

USnakeMenuUI::USnakeMenuUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsGameOver = false;
	bIsButtonBound = false;
}

void USnakeMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 只绑定一次按钮事件
	if (StartGameButton && !bIsButtonBound)
	{
		StartGameButton->OnClicked.AddDynamic(this, &USnakeMenuUI::OnStartGameClicked);
		bIsButtonBound = true;
	}
	
	// 显示鼠标
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeUIOnly());
	}
}

void USnakeMenuUI::ShowUI(bool bInIsGameOver)
{
	bIsGameOver = bInIsGameOver;
	
	// 如果UI已经在Viewport中，先移除
	if (IsInViewport())
	{
		RemoveFromParent();
	}
	
	// 添加到Viewport
	AddToViewport();
	
	// 显示鼠标
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeUIOnly());
	}
}

void USnakeMenuUI::OnStartGameClicked()
{
	ASnakeManager* SnakeManager = FindSnakeManager();
	if (SnakeManager)
	{
		SnakeManager->StartGame();
	}
	
	RemoveUI();
}

void USnakeMenuUI::RemoveUI()
{
	// 隐藏鼠标
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
	
	RemoveFromParent();
}

ASnakeManager* USnakeMenuUI::FindSnakeManager()
{
	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeManager::StaticClass(), Managers);
	
	if (Managers.Num() > 0)
	{
		return Cast<ASnakeManager>(Managers[0]);
	}
	return nullptr;
}

int32 USnakeMenuUI::GetScore()
{
	ASnakeManager* SnakeManager = FindSnakeManager();
	if (SnakeManager)
	{
		return SnakeManager->Score;
	}
	return 0;
}

TArray<int32> USnakeMenuUI::GetHighScores()
{
	ASnakeManager* SnakeManager = FindSnakeManager();
	if (SnakeManager)
	{
		return SnakeManager->GetHighScores();
	}
	return TArray<int32>();
}
