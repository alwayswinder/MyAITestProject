// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeManager.generated.h"

class ASnake;
class AFood;
class ASnakeHUD;
class USnakeUI;

UCLASS()
class MYAITESTPROJECT_API ASnakeManager : public AActor
{
	GENERATED_BODY()

public:
	ASnakeManager();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void GameOver();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	bool bGameOver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 FoodScoreValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	float GridSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	TSubclassOf<ASnake> SnakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food")
	TSubclassOf<AFood> FoodClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Boundary", meta=(OnChanged="UpdateBoundaryMesh"))
	float BoundaryDistanceX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Boundary", meta=(OnChanged="UpdateBoundaryMesh"))
	float BoundaryDistanceY;

protected:
	virtual void BeginPlay() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateBoundaryMesh();
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateCameraDistance();

private:
	ASnake* Snake;
	AFood* Food;
	
	UPROPERTY(VisibleAnywhere, Category = "Game Boundary")
	UStaticMeshComponent* BoundaryMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* CameraComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* SpringArmComponent;

	void InitializeGame();
	void SpawnSnake();
	void SpawnFood();
	
	UFUNCTION()
	void DelayedShowGameOverUI();
};