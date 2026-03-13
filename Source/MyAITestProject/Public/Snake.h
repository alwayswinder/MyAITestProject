// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Snake.generated.h"

class ASnakeSegment;
class ASnakeManager;

UCLASS()
class MYAITESTPROJECT_API ASnake : public APawn
{
	GENERATED_BODY()

public:
	ASnake();

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void ChangeDirection(FVector2D NewDirection);

	UFUNCTION(BlueprintCallable)
	void EatFood();

	UFUNCTION(BlueprintCallable)
	void GameOver();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	TSubclassOf<ASnakeSegment> SnakeSegmentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	int32 InitialSegmentCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Boundary")
	float BoundaryDistanceX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Boundary")
	float BoundaryDistanceY;

protected:
	virtual void BeginPlay() override;

private:
	TArray<ASnakeSegment*> SnakeSegments;
	FVector2D CurrentDirection;
	FTimerHandle MoveTimerHandle;
	ASnakeManager* SnakeManager;
	float GridSize;
	
	UPROPERTY(VisibleAnywhere, Category = "Snake")
	UStaticMeshComponent* HeadMesh;

	void MoveSnake();
	void SpawnInitialSegments();
	void CheckCollision();
	void GetBoundaryFromManager();
};