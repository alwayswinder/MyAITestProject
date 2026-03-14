// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SnakeSaveGame.generated.h"

UCLASS()
class MYAITESTPROJECT_API USnakeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	USnakeSaveGame();

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	TArray<int32> HighScores;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	uint32 UserIndex;
};