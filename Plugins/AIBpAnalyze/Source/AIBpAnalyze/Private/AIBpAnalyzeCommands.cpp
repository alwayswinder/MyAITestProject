// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIBpAnalyzeCommands.h"

#define LOCTEXT_NAMESPACE "FAIBpAnalyzeModule"

void FAIBpAnalyzeCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "[AI]", "Execute Open Copilot action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
