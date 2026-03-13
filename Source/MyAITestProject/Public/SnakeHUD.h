// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SnakeHUD.generated.h"

class USnakeUI;

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
	USnakeUI* SnakeUIWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USnakeUI> SnakeUIClass;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	USnakeUI* GetSnakeUI() const { return SnakeUIWidget; }
};
