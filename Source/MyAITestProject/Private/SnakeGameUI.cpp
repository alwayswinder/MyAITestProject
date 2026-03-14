// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeGameUI.h"
#include "SnakeManager.h"
#include "Kismet/GameplayStatics.h"

void USnakeGameUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void USnakeGameUI::ShowUI()
{
	if (!IsInViewport())
	{
		AddToViewport();
	}
}

void USnakeGameUI::HideUI()
{
	if (IsInViewport())
	{
		RemoveFromParent();
	}
}

int32 USnakeGameUI::GetScore()
{
	ASnakeManager* SnakeManager = FindSnakeManager();
	if (SnakeManager)
	{
		return SnakeManager->Score;
	}
	return 0;
}

ASnakeManager* USnakeGameUI::FindSnakeManager()
{
	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASnakeManager::StaticClass(), Managers);
	
	if (Managers.Num() > 0)
	{
		return Cast<ASnakeManager>(Managers[0]);
	}
	return nullptr;
}
