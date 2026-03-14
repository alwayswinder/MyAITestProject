// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SnakeHUD.generated.h"

class USnakeMenuUI;
class USnakeGameUI;

UCLASS()
class MYAITESTPROJECT_API ASnakeHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ASnakeHUD();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	USnakeMenuUI* SnakeMenuUIWidget;

	UPROPERTY()
	USnakeGameUI* SnakeGameUIWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USnakeMenuUI> SnakeMenuUIClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USnakeGameUI> SnakeGameUIClass;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	USnakeMenuUI* GetSnakeMenuUI() const { return SnakeMenuUIWidget; }
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	USnakeGameUI* GetSnakeGameUI() const { return SnakeGameUIWidget; }
};
