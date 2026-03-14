// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeHUD.h"
#include "SnakeMenuUI.h"
#include "SnakeGameUI.h"

ASnakeHUD::ASnakeHUD()
{
}

void ASnakeHUD::BeginPlay()
{
	Super::BeginPlay();

	if (SnakeMenuUIClass)
	{
		SnakeMenuUIWidget = CreateWidget<USnakeMenuUI>(GetWorld(), SnakeMenuUIClass);
		if (SnakeMenuUIWidget)
		{
			SnakeMenuUIWidget->AddToViewport();
		}
	}
	
	if (SnakeGameUIClass)
	{
		SnakeGameUIWidget = CreateWidget<USnakeGameUI>(GetWorld(), SnakeGameUIClass);
	}
}
