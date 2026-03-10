// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIBpAnalyze.h"

#include <fstream>

#include "AIBpAnalyzeStyle.h"
#include "AIBpAnalyzeCommands.h"
#include "AssetToolsModule.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "EditorUtilityLibrary.h"
#include "Windows/WindowsPlatformProcess.h"
#include "BlueprintEditorContext.h"
#include "BlueprintEditor.h"


static const FName AIBpAnalyzeTabName("AIBpAnalyze");

#define LOCTEXT_NAMESPACE "FAIBpAnalyzeModule"

void FAIBpAnalyzeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FAIBpAnalyzeStyle::Initialize();
	FAIBpAnalyzeStyle::ReloadTextures();

	FAIBpAnalyzeCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FAIBpAnalyzeCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FAIBpAnalyzeModule::PluginButtonClicked),
		FCanExecuteAction());
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAIBpAnalyzeModule::RegisterMenus));
}

void FAIBpAnalyzeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAIBpAnalyzeStyle::Shutdown();

	FAIBpAnalyzeCommands::Unregister();
}

void FAIBpAnalyzeModule::PluginButtonClicked()
{
	CopilotStart();
}

void FAIBpAnalyzeModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	// FToolMenuOwnerScoped OwnerScoped(this);
	//
	// //将按钮添加蓝图编辑器的工具栏
	// FToolMenuSection& Section = ToolBar->FindOrAddSection("Script");
	//
	// // This adds our button to the toolbar, using our UI command, which is set-up
	// // earlier with our PluginCommands to call our handler.
	// FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
	// 	FAIBpAnalyzeCommands::Get().PluginAction);
	// Entry.SetCommandList(PluginCommands);
	// Section.AddEntry(Entry);
	//
	//扩展蓝图编辑器和材质编辑器的工具栏，添加AI工具菜单
	UToolMenu* ToolBar = UToolMenus::Get()->ExtendMenu("AssetEditor.BlueprintEditor.ToolBar");
	FToolMenuSection& AiSection = ToolBar->AddSection("ai-tools");
	AiSection.AddDynamicEntry("ai-toolsCommands", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
		{
			const UBlueprintEditorToolMenuContext* Context = InSection.FindContext<UBlueprintEditorToolMenuContext>();
			if (Context && Context->BlueprintEditor.IsValid() && Context->GetBlueprintObj())
			{
				TSharedPtr<class FBlueprintEditorToolbar> BlueprintEditorToolbar = Context->BlueprintEditor.Pin()->GetToolbarBuilder();
				if (BlueprintEditorToolbar.IsValid())
				{
					FToolMenuEntry& DiffEntry = InSection.AddEntry(FToolMenuEntry::InitComboButton(
						"[AI]",
						FUIAction(),
						FOnGetContent::CreateStatic(&FAIBpAnalyzeModule::MakeAiMenu, Context),
						LOCTEXT("[AI]", "[AI]"),
						LOCTEXT("BlueprintEditorAiToolTip", "ai tools for bueprint"),
						FSlateIcon(FAppStyle::Get().GetStyleSetName(), "Blueprintai.ToolbarIcon")
					));
					DiffEntry.StyleNameOverride = "CalloutToolbar";
				}
			}
		}));
	
	UToolMenu* ToolBarM = UToolMenus::Get()->ExtendMenu("AssetEditor.MaterialEditor.ToolBar");
	FToolMenuSection& AiSectionM = ToolBarM->AddSection("ai-toolsM");
	AiSectionM.AddDynamicEntry("ai-toolsCommandsM", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
			FToolMenuEntry& DiffEntry = InSection.AddEntry(FToolMenuEntry::InitComboButton(
				"[AI]",
				FUIAction(),
				FOnGetContent::CreateStatic(&FAIBpAnalyzeModule::MakeAiMenuMaterial),
				LOCTEXT("[AI]", "[AI]"),
				LOCTEXT("MaterialEditorAiToolTip", "ai tools for material"),
				FSlateIcon(FAppStyle::Get().GetStyleSetName(), "Materialai.ToolbarIcon")
			));
			DiffEntry.StyleNameOverride = "CalloutToolbar";
	}));
	
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FAIBpAnalyzeCommands::Get().PluginAction, PluginCommands);
		}
	}
	
	// {
	// 	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	// 	{
	// 		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
	// 		{
	// 			FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAIBpAnalyzeCommands::Get().PluginAction));
	// 			Entry.SetCommandList(PluginCommands);
	// 		}
	// 	}
	// }
}

void FAIBpAnalyzeModule::BPAnalyze()
{
	FString projectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString aiPath = projectPath + "Plugins/AIBpAnalyze/ai-tools";
	
	//Export the selected assets to a specific path, then execute the bat file to analyze the exported assets.
	TArray<UObject*> ObjectsSelected = UEditorUtilityLibrary::GetSelectedAssets();

	if ( ObjectsSelected.Num() <= 0 )
	{
		FText DialogText = FText::FromString("Please select an blueprint asset to analyze");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return;
	}
	
	// FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	// AssetToolsModule.Get().ExportAssets(ObjectsToExport, WorkPaht);

	MakeTaskmdFile((TCHAR_TO_UTF8(*FPaths::Combine(aiPath, "/B_Task_T.md"))),
		(TCHAR_TO_UTF8(*FPaths::Combine(aiPath, "/B_Task.md"))), 
		 ObjectsSelected[0]);
	
	FString batFile = aiPath + "/bpAnalyze.bat";
	ExecuteBatFile(batFile, FString::Printf(TEXT(" \"%s\""), *projectPath));
}

void FAIBpAnalyzeModule::MaterialAnalyze()
{
	FString projectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString aiPath = projectPath + "Plugins/AIBpAnalyze/ai-tools";
	
	//Export the selected assets to a specific path, then execute the bat file to analyze the exported assets.
	TArray<UObject*> ObjectsSelected = UEditorUtilityLibrary::GetSelectedAssets();

	if ( ObjectsSelected.Num() <= 0 )
	{
		FText DialogText = FText::FromString("Please select an blueprint asset to analyze");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return;
	}
	
	// FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	// AssetToolsModule.Get().ExportAssets(ObjectsToExport, WorkPaht);

	MakeTaskmdFile((TCHAR_TO_UTF8(*FPaths::Combine(aiPath, "/M_Task_T.md"))),
		(TCHAR_TO_UTF8(*FPaths::Combine(aiPath, "/M_Task.md"))), 
		 ObjectsSelected[0]);
	
	FString batFile = aiPath + "/MatAnalyze.bat";
	ExecuteBatFile(batFile, FString::Printf(TEXT(" \"%s\""), *projectPath));
}

void FAIBpAnalyzeModule::OpenAnalyzeOut()
{
	FString projectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	
	TArray<UObject*> ObjectsSelected = UEditorUtilityLibrary::GetSelectedAssets();

	if ( ObjectsSelected.Num() <= 0 )
	{
		FText DialogText = FText::FromString("Please select an blueprint asset to analyze");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return;
	}
	FString OutFile = projectPath + "Plugins/AIBpAnalyze/ai-tools/ai-out/" + ObjectsSelected[0]->GetName() + "_Out.txt";
	if (!FileExists(TCHAR_TO_UTF8(*OutFile)))
	{
		FText DialogText =  FText::Format(
							LOCTEXT("PluginButtonDialogText","Not Found Analyze Result For {0},please analyze the blueprint first!"),
							FText::FromString(OutFile));
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return;
	}
	FString batFile = projectPath + "Plugins/AIBpAnalyze/ai-tools/OpenBpOut.bat";
	ExecuteBatFile(batFile, FString::Printf(TEXT(" \"%s\""), *OutFile));
}

void FAIBpAnalyzeModule::CopilotStart()
{
	FString projectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	
	FString batFile = projectPath + "Plugins/AIBpAnalyze/ai-tools/CopilotStart.bat";
	ExecuteBatFile(batFile, FString::Printf(TEXT(" \"%s\""), *projectPath));
}

bool FAIBpAnalyzeModule::ExecuteBatFile(const FString& BatFilePath, const FString& Parm)
{
	const TCHAR* parms = Parm.IsEmpty() ? L"" : *Parm;
	FPlatformProcess::CreateProc(*BatFilePath, parms, true, false, false, nullptr, 0, nullptr, nullptr);
	
	return true;
}
TSharedRef<SWidget> FAIBpAnalyzeModule::MakeAiMenu(const UBlueprintEditorToolMenuContext* InContext)
{
	
	FMenuBuilder MenuBuilder(true, NULL);
	MenuBuilder.AddMenuEntry(LOCTEXT("OpenOutFile", "Open The Last Blueprint Analyzed Result"),
		FText(), FSlateIcon(), FExecuteAction::CreateStatic(&FAIBpAnalyzeModule::OpenAnalyzeOut));
	MenuBuilder.AddMenuEntry(LOCTEXT("AnalyzeBP", "Analyze Current Blueprint"),
		FText(), FSlateIcon(), FExecuteAction::CreateStatic(&FAIBpAnalyzeModule::BPAnalyze));
	MenuBuilder.AddMenuEntry(LOCTEXT("CopilotStart", "Copilot Start"),
		FText(), FSlateIcon(), FExecuteAction::CreateStatic(&FAIBpAnalyzeModule::CopilotStart));
	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> FAIBpAnalyzeModule::MakeAiMenuMaterial()
{
	FMenuBuilder MenuBuilder(true, NULL);
	MenuBuilder.AddMenuEntry(LOCTEXT("OpenOutFile", "Open The Last Material Analyzed Result"),
		FText(), FSlateIcon(), FExecuteAction::CreateStatic(&FAIBpAnalyzeModule::OpenAnalyzeOut));
	MenuBuilder.AddMenuEntry(LOCTEXT("AnalyzeM", "Analyze Current Material"),
		FText(), FSlateIcon(), FExecuteAction::CreateStatic(&FAIBpAnalyzeModule::MaterialAnalyze));
	MenuBuilder.AddMenuEntry(LOCTEXT("CopilotStart", "Copilot Start"),
		FText(), FSlateIcon(), FExecuteAction::CreateStatic(&FAIBpAnalyzeModule::CopilotStart));
	return MenuBuilder.MakeWidget();
}

void FAIBpAnalyzeModule::MakeTaskmdFile(const std::string& inputFilePath, const std::string& outputFilePath, UObject* Obj)
{
	const std::string& ObjectName =  TCHAR_TO_UTF8(*Obj->GetName());
	const std::string& ObjectFullName =  TCHAR_TO_UTF8(*Obj->GetFullName());

	
	std::ifstream inputFile(inputFilePath);
	if (!inputFile.is_open()) {
		//std::cerr << "无法打开文件 " << inputFilePath << std::endl;
		return;
	}

	std::string fileContent;
	std::string line;
	while (std::getline(inputFile, line)) {
		size_t pos = 0;
		while ((pos = line.find("#FNAME#", pos))!= std::string::npos) {
			line.replace(pos, 7, ObjectFullName);
			pos += ObjectName.length();
		}
		pos = 0;
		while ((pos = line.find("#NAME#", pos))!= std::string::npos) {
			line.replace(pos, 6, ObjectName);
			pos += ObjectName.length();
		}
		fileContent += line + "\n";
	}
	inputFile.close();

	std::ofstream outputFile(outputFilePath);
	if (!outputFile.is_open()) {
		//std::cerr << "无法打开文件 " << outputFilePath << std::endl;
		return;
	}

	outputFile << fileContent;
	outputFile.close();
}

bool FAIBpAnalyzeModule::FileExists(const std::string& filePath)
{
	std::ifstream file(filePath);
	return file.good();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAIBpAnalyzeModule, AIBpAnalyze)