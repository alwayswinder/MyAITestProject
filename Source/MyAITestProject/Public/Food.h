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

private:
	UPROPERTY(VisibleAnywhere, Category = "Food")
	UStaticMeshComponent* MeshComponent;

	ASnakeManager* SnakeManager;

	void SpawnAtRandomLocation();
	void InitializeMesh();
	void FindSnakeManager();
};