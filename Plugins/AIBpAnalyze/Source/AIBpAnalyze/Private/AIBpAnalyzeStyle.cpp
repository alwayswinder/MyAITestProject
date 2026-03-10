// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIBpAnalyzeStyle.h"
#include "AIBpAnalyze.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FAIBpAnalyzeStyle::StyleInstance = nullptr;

void FAIBpAnalyzeStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAIBpAnalyzeStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAIBpAnalyzeStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AIBpAnalyzeStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FAIBpAnalyzeStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("AIBpAnalyzeStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("AIBpAnalyze")->GetBaseDir() / TEXT("Resources"));

	Style->Set("AIBpAnalyze.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FAIBpAnalyzeStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FAIBpAnalyzeStyle::Get()
{
	return *StyleInstance;
}
