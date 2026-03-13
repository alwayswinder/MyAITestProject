// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeHUD.h"
#include "SnakeUI.h"

ASnakeHUD::ASnakeHUD()
{
}

void ASnakeHUD::BeginPlay()
{
	Super::BeginPlay();

	if (SnakeUIClass)
	{
		SnakeUIWidget = CreateWidget<USnakeUI>(GetWorld(), SnakeUIClass);
		if (SnakeUIWidget)
		{
			SnakeUIWidget->AddToViewport();
		}
	}
}
