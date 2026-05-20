#include "CustomWorldNormalPass.h"

#include "ScenePrivate.h"
#include "SceneRendering.h"
#include "LightMapRendering.h"

#include "Shader.h"
#include "GlobalShader.h"
#include "MeshMaterialShader.h"
#include "MeshPassProcessor.h"
#include "MeshPassProcessor.inl"

class FMyNormalPassShaderElementData : public FMeshMaterialShaderElementData
{
public:
	const FLightCacheInterface* LCI = nullptr;
};

class FMyNormalPassVSPolicyParamType : public FMeshMaterialShader, public FUniformLightMapPolicy::VertexParametersType
{
	DECLARE_INLINE_TYPE_LAYOUT_EXPLICIT_BASES(FMyNormalPassVSPolicyParamType, NonVirtual, FMeshMaterialShader, FUniformLightMapPolicy::VertexParametersType);

protected:
	FMyNormalPassVSPolicyParamType() {}
	FMyNormalPassVSPolicyParamType(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		FUniformLightMapPolicy::VertexParametersType::Bind(Initializer.ParameterMap);
	}

public:
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FMeshMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		FMobileDistanceFieldShadowsAndLQLightMapPolicy::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
		const FMyNormalPassShaderElementData& MyShaderElementData = static_cast<const FMyNormalPassShaderElementData&>(ShaderElementData);
		FUniformLightMapPolicy::GetVertexShaderBindings(PrimitiveSceneProxy, MyShaderElementData.LCI, this, ShaderBindings);
	}
};

class FMyNormalPassVSBaseType : public FMyNormalPassVSPolicyParamType
{
	typedef FMyNormalPassVSPolicyParamType Super;
	DECLARE_INLINE_TYPE_LAYOUT(FMyNormalPassVSBaseType, NonVirtual);

protected:
	FMyNormalPassVSBaseType() {}
	FMyNormalPassVSBaseType(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
		: Super(Initializer)
	{
	}

public:
	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsMobilePlatform(Parameters.Platform)
			&& FMobileDistanceFieldShadowsAndLQLightMapPolicy::ShouldCompilePermutation(Parameters);
	}
};

class FMyNormalPassVS : public FMyNormalPassVSBaseType
{
	DECLARE_SHADER_TYPE(FMyNormalPassVS, MeshMaterial);

public:
	FMyNormalPassVS() {}
	FMyNormalPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMyNormalPassVSBaseType(Initializer)
	{
	}
};

class FMyNormalPassPSPolicyParamType : public FMeshMaterialShader, public FUniformLightMapPolicy::PixelParametersType
{
	DECLARE_INLINE_TYPE_LAYOUT_EXPLICIT_BASES(FMyNormalPassPSPolicyParamType, NonVirtual, FMeshMaterialShader, FUniformLightMapPolicy::PixelParametersType);

protected:
	FMyNormalPassPSPolicyParamType() {}
	FMyNormalPassPSPolicyParamType(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		FUniformLightMapPolicy::PixelParametersType::Bind(Initializer.ParameterMap);
	}

public:
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FMeshMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		FMobileDistanceFieldShadowsAndLQLightMapPolicy::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
		const FMyNormalPassShaderElementData& MyShaderElementData = static_cast<const FMyNormalPassShaderElementData&>(ShaderElementData);
		FUniformLightMapPolicy::GetPixelShaderBindings(PrimitiveSceneProxy, MyShaderElementData.LCI, this, ShaderBindings);
	}
};

class FMyNormalPassPSBaseType : public FMyNormalPassPSPolicyParamType
{
	typedef FMyNormalPassPSPolicyParamType Super;
	DECLARE_INLINE_TYPE_LAYOUT(FMyNormalPassPSBaseType, NonVirtual);

protected:
	FMyNormalPassPSBaseType() {}
	FMyNormalPassPSBaseType(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
		: Super(Initializer)
	{
	}

public:
	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsMobilePlatform(Parameters.Platform)
			&& FMobileDistanceFieldShadowsAndLQLightMapPolicy::ShouldCompilePermutation(Parameters);
	}
};

class FMyNormalPassPS : public FMyNormalPassPSBaseType
{
	DECLARE_SHADER_TYPE(FMyNormalPassPS, MeshMaterial);

public:
	FMyNormalPassPS() {}
	FMyNormalPassPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMyNormalPassPSBaseType(Initializer)
	{
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

	FMyNormalPassShaderElementData ShaderElementData;
	ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, StaticMeshId, true);
	ShaderElementData.LCI = MeshBatch.LCI;

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