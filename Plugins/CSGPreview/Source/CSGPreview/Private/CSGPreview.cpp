// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSGPreview.h"
#include "CSGPreviewStyle.h"
#include "CSGPreviewCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

// for shader

#include "GlobalShader.h"
#include "Core/Public/Core.h"
#include "RHI/Public/RHI.h"
#include "RHI/Public/RHIDefinitions.h"
#include "RHI/Public/RHIStaticStates.h"
#include "RenderCore/Public/RenderCore.h"
#include "RenderCore/Public/Shader.h"
#include "RenderCore/Public/ShaderParameters.h"
#include "RenderCore/Public/ShaderParameterUtils.h"
#include "RenderCore/Public/StaticBoundShaderState.h"
#include "RenderCore/Public/RenderUtils.h"
#include "Renderer/Private/DeferredShadingRenderer.h"
#include "Renderer/Public/SceneRenderTargetParameters.h"
#include "HAL/IConsoleManager.h"


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
							LOCTEXT("PPLL On or Off", "Now PPLL is {0}"),
							FText::FromString(FString::FromInt(!EnablePPLL))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	if (!EnablePPLL)
	{
		// TODO : now on
		EnablePPLL = true;
	}
	else
	{
		// TODO : now off
		EnablePPLL = false;
	}
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

// ��δ��������ͷ�ļ���cpp�ļ�������
class FMyTestVS : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FMyTestVS, Global, /*MYMODULE_API*/);

	FMyTestVS() { }
	FMyTestVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
	}
};

// �˲�������cpp�ļ��Ͻ���
IMPLEMENT_SHADER_TYPE(, FMyTestVS, TEXT("/Plugin/CSGPreview/Private/MyTest.usf"), TEXT("MainVS"), SF_Vertex);

class FMyTestPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMyTestPS, Global);

	FShaderParameter MyColorParameter;

	FMyTestPS() { }
	FMyTestPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		MyColorParameter.Bind(Initializer.ParameterMap, TEXT("MyColor"), SPF_Mandatory);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("MY_DEFINE"), 1);
	}


	static bool ShouldCache(EShaderPlatform Platform)
	{
		// ���磬������ "Platform == SP_METAL" �ı���
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
	}

	//// Fshader �ӿڡ�
	//virtual bool Serialize(FArchive& Ar) override
	//{
	//	bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
	//	Ar << MyColorParameter;
	//	return bShaderHasOutdatedParameters;
	//}

	void SetColor(FRHICommandList& RHICmdList, const FLinearColor& Color)
	{
		//SetShaderValue(RHICmdList,  GetPixelShader(), MyColorParameter, Color);
	}
};

// Դ�ļ����ǰ��ͬ������ڲ�ͬ
IMPLEMENT_SHADER_TYPE(, FMyTestPS, TEXT("/Plugin/CSGPreview/Private/MyTest.usf"), TEXT("MainPS"), SF_Pixel);


void RenderMyTest(FRHICommandList& RHICmdList, ERHIFeatureLevel::Type FeatureLevel, const FLinearColor& Color)
{
	// ��ȡȫ����ɫ������
	auto ShaderMap = GetGlobalShaderMap(FeatureLevel);

	// ��ShaderMap��ȡʵ�ʵ���ɫ��ʵ��
	TShaderMapRef<FScreenVS> MyVS(ShaderMap);
	TShaderMapRef<FScreenPS> MyPS(ShaderMap);

	// ʹ����Щ��ɫ����������ɫ��״̬��������Ӧ�õ������б�
	//static FGlobalBoundShaderState MyTestBoundShaderState;
	//SetGlobalBoundShaderState(RHICmdList, FeatureLevel, MyTestBoundShaderState, GetVertexDeclarationFVector4(), *MyVS, *MyPS);

	//// ���ú��������ò���
	//MyPS->SetColor(RHICmdList, Color);

	//// Ԥ������GPU���Ի���ʵ���ı���
	//RHICmdList.SetRasterizerState(TStaticRasterizerState::GetRHI());
	//RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
	//RHICmdList.SetDepthStencilState(TStaticDepthStencilState::GetRHI(), 0);

	//// ���ö���
	//FVector4 Vertices[4];
	//Vertices[0].Set(-1.0f, 1.0f, 0, 1.0f);
	//Vertices[1].Set(1.0f, 1.0f, 0, 1.0f);
	//Vertices[2].Set(-1.0f, -1.0f, 0, 1.0f);
	//Vertices[3].Set(1.0f, -1.0f, 0, 1.0f);

	// �����ı���
	//DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));
	//DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));
}


static TAutoConsoleVariable<int32> CVarMyTest(
	TEXT("r.MyTest"),
	0,
	TEXT("Test My Global Shader, set it to 0 to disable, or to 1, 2 or 3 for fun!"),
	ECVF_RenderThreadSafe
);

//void FDeferredShadingSceneRenderer::RenderFinish(FRHICommandListImmediate& RHICmdList)
//{
//	//[...]
//	// ***
//	// Inserted code, just before finishing rendering, so we can overwrite the screen��s contents!
//	int32 MyTestValue = CVarMyTest.GetValueOnAnyThread();
//	if (MyTestValue != 0)
//	{
//		FLinearColor Color(MyTestValue == 1, MyTestValue == 2, MyTestValue == 3, 1);
//		RenderMyTest(RHICmdList, FeatureLevel, Color);
//	}
//	// End Inserted code
//	// ***
//	FSceneRenderer::RenderFinish(RHICmdList);
//	//[...]
//}
