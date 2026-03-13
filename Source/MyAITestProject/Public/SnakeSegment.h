// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeSegment.generated.h"

UCLASS()
class MYAITESTPROJECT_API ASnakeSegment : public AActor
{
	GENERATED_BODY()

public:
	ASnakeSegment();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Snake Segment")
	UStaticMeshComponent* MeshComponent;

	void InitializeMesh();
};