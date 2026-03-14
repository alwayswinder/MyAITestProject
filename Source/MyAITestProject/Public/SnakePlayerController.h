// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SnakePlayerController.generated.h"

class ASnake;

UCLASS()
class MYAITESTPROJECT_API ASnakePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASnakePlayerController();
	
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void SetSnake(ASnake* NewSnake);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	ASnake* Snake;

	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void OnMoveUpPressed();
	void OnMoveUpReleased();
	void OnMoveDownPressed();
	void OnMoveDownReleased();
	void OnMoveLeftPressed();
	void OnMoveLeftReleased();
	void OnMoveRightPressed();
	void OnMoveRightReleased();
	
	int32 PressedKeyCount;
};