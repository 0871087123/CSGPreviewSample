// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSGPreviewCommands.h"

#define LOCTEXT_NAMESPACE "FCSGPreviewModule"

void FCSGPreviewCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "CSGPreview", "Execute CSGPreview action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
