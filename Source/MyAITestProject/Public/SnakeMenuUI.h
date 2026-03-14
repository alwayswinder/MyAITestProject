// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnakeMenuUI.generated.h"

class UButton;
class ASnakeManager;

UCLASS()
class MYAITESTPROJECT_API USnakeMenuUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bIsGameOver;
	
private:
	bool bIsButtonBound;
	
public:
	USnakeMenuUI(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowUI(bool bInIsGameOver = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	int32 GetScore();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	TArray<int32> GetHighScores();

private:
	UPROPERTY(meta = (BindWidget))
	UButton* StartGameButton;

	UFUNCTION()
	void OnStartGameClicked();

	ASnakeManager* FindSnakeManager();
	
	void RemoveUI();
};
