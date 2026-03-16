// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Food.generated.h"

class ASnakeManager;

UENUM(BlueprintType)
enum class EFoodType : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Invisible UMETA(DisplayName = "Invisible"),
	Invincible UMETA(DisplayName = "Invincible")
};

UCLASS()
class MYAITESTPROJECT_API AFood : public AActor
{
	GENERATED_BODY()
	
public:	
	AFood();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food")
	EFoodType FoodType;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFoodTypeChanged, EFoodType, NewFoodType);

	UPROPERTY(BlueprintAssignable, Category = "Food")
	FOnFoodTypeChanged OnFoodTypeChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Food")
	UStaticMeshComponent* MeshComponent;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	ASnakeManager* SnakeManager;
	float GridSize;

	void SpawnAtRandomLocation();
	void InitializeMesh();
	void FindSnakeManager();
	void RandomizeFoodType();

	void StartCollectAnimation();
	void UpdateCollectAnimation(float DeltaTime);

	bool bIsCollecting;
	float AnimationTimer;
	float AnimationDuration;
	float BounceHeight;
	FVector InitialLocation;
	FVector InitialScale;
};