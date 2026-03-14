// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeObstacle.generated.h"

class ASnakeManager;

UCLASS()
class MYAITESTPROJECT_API ASnakeObstacle : public AActor
{
	GENERATED_BODY()

public:
	ASnakeObstacle();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Obstacle")
	UStaticMeshComponent* MeshComponent;

	ASnakeManager* SnakeManager;
	float GridSize;

	void InitializeMesh();
	void FindSnakeManager();
};