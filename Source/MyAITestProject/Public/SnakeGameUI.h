// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnakeGameUI.generated.h"

class ASnakeManager;

UCLASS()
class MYAITESTPROJECT_API USnakeGameUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowUI();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideUI();

	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetScore();

private:
	ASnakeManager* FindSnakeManager();
};
