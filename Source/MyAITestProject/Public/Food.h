// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Food.generated.h"

class ASnakeManager;

UCLASS()
class MYAITESTPROJECT_API AFood : public AActor
{
	GENERATED_BODY()

public:
	AFood();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Food")
	UStaticMeshComponent* MeshComponent;

	ASnakeManager* SnakeManager;
	float GridSize;

	void SpawnAtRandomLocation();
	void InitializeMesh();
	void FindSnakeManager();

	void StartCollectAnimation();
	void UpdateCollectAnimation(float DeltaTime);

	bool bIsCollecting;
	float AnimationTimer;
	float AnimationDuration;
	float BounceHeight;
	FVector InitialLocation;
	FVector InitialScale;
};