// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AIBpAnalyzeStyle.h"

class FAIBpAnalyzeCommands : public TCommands<FAIBpAnalyzeCommands>
{
public:

	FAIBpAnalyzeCommands()
		: TCommands<FAIBpAnalyzeCommands>(TEXT("AIBpAnalyze"),
			NSLOCTEXT("Contexts", "AIBpAnalyze", "AIBpAnalyze Plugin"),
			NAME_None,
			FAIBpAnalyzeStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
