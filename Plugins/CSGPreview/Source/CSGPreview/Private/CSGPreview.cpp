// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSGPreview.h"
#include "CSGPreviewStyle.h"
#include "CSGPreviewCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName CSGPreviewTabName("CSGPreview");

#define LOCTEXT_NAMESPACE "FCSGPreviewModule"

void FCSGPreviewModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FCSGPreviewStyle::Initialize();
	FCSGPreviewStyle::ReloadTextures();

	FCSGPreviewCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCSGPreviewCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FCSGPreviewModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCSGPreviewModule::RegisterMenus));
}

void FCSGPreviewModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FCSGPreviewStyle::Shutdown();

	FCSGPreviewCommands::Unregister();
}

void FCSGPreviewModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FCSGPreviewModule::PluginButtonClicked()")),
							FText::FromString(TEXT("CSGPreview.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FCSGPreviewModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCSGPreviewCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCSGPreviewCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCSGPreviewModule, CSGPreview)