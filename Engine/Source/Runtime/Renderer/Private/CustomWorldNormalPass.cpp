#include "CustomWorldNormalPass.h"

#include "ScenePrivate.h"
#include "SceneRendering.h"

#include "Shader.h"
#include "GlobalShader.h"
#include "MeshMaterialShader.h"
#include "MeshPassProcessor.h"
#include "MeshPassProcessor.inl"

class FMyNormalPassVS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FMyNormalPassVS, MeshMaterial);

	FMyNormalPassVS() {}
public:

	FMyNormalPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
	}

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsMobilePlatform(Parameters.Platform) ;
	}

	void GetShaderBindings(const FScene* Scene, ERHIFeatureLevel::Type FeatureLevel, const FPrimitiveSceneProxy* PrimitiveSceneProxy, const FMaterialRenderProxy& MaterialRenderProxy, const FMaterial& Material, const FMeshPassProcessorRenderState& DrawRenderState, const FMeshMaterialShaderElementData& ShaderElementData, FMeshDrawSingleShaderBindings& ShaderBindings)
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
	}

};

class FMyNormalPassPS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FMyNormalPassPS, MeshMaterial);

public:

	FMyNormalPassPS() {}
	FMyNormalPassPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		//PassUniformBuffer.Bind(Initializer.ParameterMap, FMobileSceneTextureUniformParameters::StaticStructMetadata.GetShaderVariableName());
	}

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsMobilePlatform(Parameters.Platform);
	}
	void GetShaderBindings(const FScene* Scene, ERHIFeatureLevel::Type FeatureLevel, const FPrimitiveSceneProxy* PrimitiveSceneProxy, const FMaterialRenderProxy& MaterialRenderProxy, const FMaterial& Material, const FMeshPassProcessorRenderState& DrawRenderState, const FMeshMaterialShaderElementData& ShaderElementData, FMeshDrawSingleShaderBindings& ShaderBindings)
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
	}
};

IMPLEMENT_MATERIAL_SHADER_TYPE(, FMyNormalPassVS, TEXT("/Engine/Private/CustomWorldNormalPass.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FMyNormalPassPS, TEXT("/Engine/Private/CustomWorldNormalPass.usf"), TEXT("MainPS"), SF_Pixel);

void FMobileSceneRenderer::RenderCustomWorldNormalPass(FRHICommandListImmediate& RHICmdList, const TArrayView<const FViewInfo*> PassViews)
{
	// do we have primitives in this pass?  
	bool bPrimitives = false;

	if (!Scene->World || (Scene->World->WorldType != EWorldType::EditorPreview && Scene->World->WorldType != EWorldType::Inactive))
	{
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
		{
			const FViewInfo& View = Views[ViewIndex];
			if (View.bHasCustomWorldNormalPrimitives)
			{
				bPrimitives = true;
				break;
			}
		}
	}

	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
	//const FCustomCaptureTextures CustomCaptureTextures = SceneContext.RequestCustomCapture(RHICmdList, bPrimitives);
	const FCustomWorldNormalTextures CustomWorldNormalTextures = SceneContext.RequestCustomWorldNormal(RHICmdList, bPrimitives);

	if (CustomWorldNormalTextures.CustomWorldNormal)
	{
		SCOPED_DRAW_EVENT(RHICmdList, CustomWorldNormalPass);

		//TRefCountPtr<FRHIUniformBuffer> SceneTexturesUniformBuffer = CreateMobileSceneTextureUniformBuffer(RHICmdList, EMobileSceneTextureSetupMode::CustomCapture);
		//SCOPED_UNIFORM_BUFFER_GLOBAL_BINDINGS(RHICmdList, SceneTexturesUniformBuffer);

		//RHICmdList.Transition(FRHITransitionInfo(CustomCaptureTextures.CustomColor, ERHIAccess::SRVGraphics, ERHIAccess::RTV));
		RHICmdList.Transition(FRHITransitionInfo(CustomWorldNormalTextures.CustomWorldNormal, ERHIAccess::SRVGraphics, ERHIAccess::RTV));

		FRHIRenderPassInfo RPInfo(CustomWorldNormalTextures.CustomWorldNormal, ERenderTargetActions::Clear_Store);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("CustomWorldNormalRendering"));

		for (int32 ViewIndex = 0; ViewIndex < PassViews.Num(); ViewIndex++)
		{
			const FViewInfo& View = *PassViews[ViewIndex];
			if (!View.ShouldRenderView())
			{
				continue;
			}

			if (Scene->UniformBuffers.UpdateViewUniformBuffer(View))
			{
				UpdateOpaqueBasePassUniformBuffer(RHICmdList, View);
				UpdateTranslucentBasePassUniformBuffer(RHICmdList, View);
			}

			RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0, View.ViewRect.Max.X, View.ViewRect.Max.Y, 1);
			View.ParallelMeshDrawCommandPasses[EMeshPass::CustomWorldNormalPass].DispatchDraw(nullptr, RHICmdList);

			//RDG_EVENT_SCOPE_CONDITIONAL(GraphBuilder, PassViews.Num() > 1, "View%d", ViewIndex);
			//FRDGBuilder GraphBuilder(RHICmdList);
			// Only clear once the target for every views it contains.
			//ERenderTargetLoadAction LoadAction = ERenderTargetLoadAction::EClear;
			//FRenderTargetParameters* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
			//PassParameters->RenderTargets[0] = FRenderTargetBinding(CustomCaptureTextures.CustomColor, LoadAction);
			////PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding();

			//// Next pass, do not clear the shared target, only render into it
			//LoadAction = ERenderTargetLoadAction::ELoad;
			//
			//GraphBuilder.AddPass(
			//	RDG_EVENT_NAME("CustomCapture"),
			//	PassParameters,
			//	ERDGPassFlags::Raster,
			//	[this, &View](FRHICommandListImmediate& RHICmdList)
			//	{
			//		FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);

			//		Scene->UniformBuffers.CustomDepthViewUniformBuffer.UpdateUniformBufferImmediate(*View.CachedViewUniformShaderParameters);
			//		// If we don't render this pass in stereo we simply update the buffer with the same view uniform parameters.
			//		Scene->UniformBuffers.InstancedCustomDepthViewUniformBuffer.UpdateUniformBufferImmediate(reinterpret_cast<FInstancedViewUniformShaderParameters&>(*View.CachedViewUniformShaderParameters));

			//		View.ParallelMeshDrawCommandPasses[EMeshPass::CustomCapturePass].DispatchDraw(nullptr, RHICmdList);
			//	});

		}

		RHICmdList.EndRenderPass();

		//RHICmdList.Transition(FRHITransitionInfo(CustomCaptureTextures.CustomColor, ERHIAccess::RTV, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(CustomWorldNormalTextures.CustomWorldNormal, ERHIAccess::RTV, ERHIAccess::SRVGraphics));
	}
}

FMyNormalPassProcessor::FMyNormalPassProcessor(
	const FScene* Scene,
	const FSceneView* InViewIfDynamicMeshCommand,
	const bool InbRespectUseAsOccluderFlag,
	const bool InbEarlyZPassMoveabe,
	FMeshPassDrawListContext* InDrawListContext
)
	:FMeshPassProcessor(
		Scene
		, Scene->GetFeatureLevel()
		, InViewIfDynamicMeshCommand
		, InDrawListContext
	)
	, bRespectUseAsOccluderFlag(InbRespectUseAsOccluderFlag)
	, bEarlyZPassMoveable(InbEarlyZPassMoveabe)
{
	PassDrawRenderState.SetViewUniformBuffer(Scene->UniformBuffers.ViewUniformBuffer);
	PassDrawRenderState.SetInstancedViewUniformBuffer(Scene->UniformBuffers.InstancedViewUniformBuffer);
	PassDrawRenderState.SetBlendState(TStaticBlendState<CW_RGBA>::GetRHI());
	//need no depth
	PassDrawRenderState.SetDepthStencilState(TStaticDepthStencilState<false, CF_Never>::GetRHI());
}

void FMyNormalPassProcessor::AddMeshBatch(const FMeshBatch& RESTRICT MeshBatch, uint64 BatchElementMask, const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy, int32 StaticMeshId)
{
	const FMaterialRenderProxy* FallbackMaterialRenderProxyPtr = nullptr;
	const FMaterial& Material = MeshBatch.MaterialRenderProxy->GetMaterialWithFallback(Scene->GetFeatureLevel(), FallbackMaterialRenderProxyPtr);
	const FMaterialRenderProxy& MaterialRenderProxy = FallbackMaterialRenderProxyPtr ? *FallbackMaterialRenderProxyPtr : *MeshBatch.MaterialRenderProxy;
	/* if use MeshBatch's mateiril
	const EBlendMode BlendMode = Material.GetBlendMode();
	const bool bIsTranslucent = IsTranslucentBlendMode(BlendMode);
	if (bIsTranslucent)
	{
		PassDrawRenderState.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_Source1Color, BO_Add, BF_One, BF_Source1Alpha>::GetRHI());
		PassDrawRenderState.SetPassUniformBuffer(Scene->UniformBuffers.MobileTranslucentBasePassUniformBuffer);
	}
	else
	{
		PassDrawRenderState.SetBlendState(TStaticBlendStateWriteMask<CW_RGBA>::GetRHI());
		PassDrawRenderState.SetPassUniformBuffer(Scene->UniformBuffers.MobileOpaqueBasePassUniformBuffer);
	}
	*/

	if ((!PrimitiveSceneProxy || PrimitiveSceneProxy->ShouldRenderInMainPass())
		&& ShouldIncludeDomainInMeshPass(Material.GetMaterialDomain())
		&& PrimitiveSceneProxy->ShouldRenderWorldNormal()
		)
	{
		// if use MeshBatch's mateiril
		Process(
			MeshBatch,
			BatchElementMask,
			StaticMeshId,
			PrimitiveSceneProxy,
			MaterialRenderProxy,
			Material
		);
		

	}

}

void FMyNormalPassProcessor::Process(
	const FMeshBatch& MeshBatch,
	uint64 BatchElementMask,
	int32 StaticMeshId,
	const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
	const FMaterialRenderProxy& RESTRICT MaterialRenderProxy,
	const FMaterial& RESTRICT MaterialResource
)
{
	const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;

	TMeshProcessorShaders<
		FMyNormalPassVS,
		FBaseHS,
		FBaseDS,
		FMyNormalPassPS
	>MyPassShaders;

	MyPassShaders.VertexShader = MaterialResource.GetShader<FMyNormalPassVS>(VertexFactory->GetType());
	MyPassShaders.PixelShader = MaterialResource.GetShader<FMyNormalPassPS>(VertexFactory->GetType());

	const FMeshDrawingPolicyOverrideSettings OverrideSettings = ComputeMeshOverrideSettings(MeshBatch);
	const ERasterizerFillMode MeshFillMode = ComputeMeshFillMode(MeshBatch, MaterialResource, OverrideSettings);
	const ERasterizerCullMode MeshCullMode = ComputeMeshCullMode(MeshBatch, MaterialResource, OverrideSettings);

	FMeshMaterialShaderElementData ShaderElementData;
	ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, StaticMeshId, true);

	const FMeshDrawCommandSortKey SortKey = CalculateMeshStaticSortKey(MyPassShaders.VertexShader, MyPassShaders.PixelShader);

	BuildMeshDrawCommands(
		MeshBatch,
		BatchElementMask,
		PrimitiveSceneProxy,
		MaterialRenderProxy,
		MaterialResource,
		PassDrawRenderState,
		MyPassShaders,
		MeshFillMode,
		MeshCullMode,
		SortKey,
		EMeshPassFeatures::Default,
		ShaderElementData
	);

}

FMeshPassProcessor* CreateNormalPassProcessor(
	const FScene* Scene,
	const FSceneView* InViewIfDynamicMeshCommand,
	FMeshPassDrawListContext* InDrawListContext
)
{
	return new(FMemStack::Get()) FMyNormalPassProcessor(
		Scene,
		InViewIfDynamicMeshCommand,
		true,
		Scene->bEarlyZPassMovable,
		InDrawListContext
	);
}

FRegisterPassProcessorCreateFunction RegisterNormalPass(
	&CreateNormalPassProcessor,
	EShadingPath::Mobile,
	EMeshPass::CustomWorldNormalPass,
	EMeshPassFlags::CachedMeshCommands | EMeshPassFlags::MainView
);