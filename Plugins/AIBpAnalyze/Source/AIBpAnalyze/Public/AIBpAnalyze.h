// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <string>

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UBlueprintEditorToolMenuContext;
class FToolBarBuilder;
class FMenuBuilder;

class FAIBpAnalyzeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	static void BPAnalyze();
	static void MaterialAnalyze();
	static void OpenAnalyzeOut();
	static void CopilotStart();
   
	static bool ExecuteBatFile(const FString& BatFilePath,  const FString& Parm = FString());
	
	static TSharedRef<SWidget> MakeAiMenu(const UBlueprintEditorToolMenuContext* InContext);
	static TSharedRef<SWidget> MakeAiMenuMaterial();

	static void MakeTaskmdFile(const std::string& inputFilePath, const std::string& outputFilePath, UObject* Obj);
	static bool FileExists(const std::string& filePath);
private:
	TSharedPtr<class FUICommandList> PluginCommands;
};

