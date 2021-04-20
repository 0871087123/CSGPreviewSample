// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CSGPreviewStyle.h"

class FCSGPreviewCommands : public TCommands<FCSGPreviewCommands>
{
public:

	FCSGPreviewCommands()
		: TCommands<FCSGPreviewCommands>(TEXT("CSGPreview"), NSLOCTEXT("Contexts", "CSGPreview", "CSGPreview Plugin"), NAME_None, FCSGPreviewStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
