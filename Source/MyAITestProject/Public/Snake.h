// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Food.h"
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
	void ReleaseDirection();

	UFUNCTION(BlueprintCallable)
	void EatFood();

	UFUNCTION(BlueprintCallable)
	void GameOver();

	UFUNCTION(BlueprintCallable, Category = "Snake")
	void StartBoost();

	UFUNCTION(BlueprintCallable, Category = "Snake")
	void StopBoost();

	UFUNCTION(BlueprintCallable, Category = "Snake")
	bool IsPositionOccupiedBySnake(FVector Position, float Tolerance);

	UFUNCTION(BlueprintCallable, Category = "Snake")
	void ApplyFoodEffect(EFoodType EffectType);

	UFUNCTION(BlueprintCallable, Category = "Snake")
	bool IsInvisible() const { return bIsInvisible; }

	UFUNCTION(BlueprintCallable, Category = "Snake")
	bool IsInvincible() const { return bIsInvincible; }

	UFUNCTION(BlueprintCallable, Category = "Snake")
	FVector2D GetCurrentDirection() const { return CurrentDirection; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	float BoostMoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	TSubclassOf<ASnakeSegment> SnakeSegmentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake")
	int32 InitialSegmentCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Boundary")
	float BoundaryDistanceX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Boundary")
	float BoundaryDistanceY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake Effects")
	float EffectDuration;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectStarted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectEnded);

	UPROPERTY(BlueprintAssignable, Category = "Snake Effects")
	FOnEffectStarted OnInvisibleStarted;

	UPROPERTY(BlueprintAssignable, Category = "Snake Effects")
	FOnEffectEnded OnInvisibleEnded;

	UPROPERTY(BlueprintAssignable, Category = "Snake Effects")
	FOnEffectStarted OnInvincibleStarted;

	UPROPERTY(BlueprintAssignable, Category = "Snake Effects")
	FOnEffectEnded OnInvincibleEnded;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake", meta = (AllowPrivateAccess = "true"))
	TArray<ASnakeSegment*> SnakeSegments;
	
	FVector2D CurrentDirection;
	FVector2D PendingDirection;
	FVector2D LastMovedDirection;
	FTimerHandle MoveTimerHandle;
	ASnakeManager* SnakeManager;
	float GridSize;
	float CurrentMoveSpeed;
	bool bIsBoosting;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snake", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* HeadMesh;

	bool bIsInvisible;
	bool bIsInvincible;
	float EffectTimer;
	
	// 方向长按加速相关变量
	bool bIsDirectionPressed;
	FVector2D PressedDirection;
	float DirectionPressTimer;
	const float BoostThreshold = 0.5f; // 长按0.5秒进入加速状态

	void MoveSnake();
	void SpawnInitialSegments();
	void CheckCollision();
	void GetBoundaryFromManager();
	void UpdateEffects(float DeltaTime);
	void ClearEffects();
};