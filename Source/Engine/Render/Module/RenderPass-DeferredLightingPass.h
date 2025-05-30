#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT VectorMath;
PARTING_IMPORT Logger;


PARTING_SUBMODULE(Parting, SSAOPass)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include<random>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/RenderPass-Base.h"

#include "Engine/Render/Module/SceneTypes.h"
#include "Engine/Engine/Module/SceneGraph.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"
#include "Engine/Render/Module/View.h"

#include "Engine/Render/Module/GBuffer.h"

#include "Shader/deferred_lighting_cb.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {

	template<RHI::APITagConcept APITag>
	class DeferredLightingPass final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;

	public:
		struct Inputs final {
			Imp_Texture* Depth{ nullptr };
			Imp_Texture* GBufferNormals{ nullptr };
			Imp_Texture* GBufferDiffuse{ nullptr };
			Imp_Texture* GBufferSpecular{ nullptr };
			Imp_Texture* GBufferEmissive{ nullptr };
			Imp_Texture* IndirectDiffuse{ nullptr };
			Imp_Texture* IndirectSpecular{ nullptr };
			Imp_Texture* ShadowChannels{ nullptr };
			Imp_Texture* AmbientOcclusion{ nullptr };
			Imp_Texture* Output{ nullptr };

			const Vector<SharedPtr<Light<APITag>>>* Lights{ nullptr };
			const Vector<SharedPtr<LightProbe<APITag>>>* LightProbes{ nullptr };

			Math::VecF3 AmbientColorTop{ Math::VecF3::Zero() };
			Math::VecF3 AmbientColorBottom{ Math::VecF3::Zero() };

			// Fills the GBuffer-related textures (depth, normals, etc.) from the provided structure.
			void Set_GBuffer(const GBufferRenderTargets<APITag>& targets) {
				this->Depth = targets.Depth;
				this->GBufferNormals = targets.GBufferNormals;
				this->GBufferDiffuse = targets.GBufferDiffuse;
				this->GBufferSpecular = targets.GBufferSpecular;
				this->GBufferEmissive = targets.GBufferEmissive;
			}
		};

	public:
		DeferredLightingPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonPasses);
		~DeferredLightingPass(void) = default;

	public:
		void Init(const SharedPtr<ShaderFactory<APITag>>& shaderFactory);

		void ResetBindingCache(void);

		void Render(Imp_CommandList* commandList, const ICompositeView& compositeView, const Inputs& inputs, Math::VecF2 randomOffset = Math::VecF2::Zero());

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;

		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;

		RHI::RefCountPtr<Imp_Shader> m_ComputeShader;
		RHI::RefCountPtr<Imp_Sampler> m_ShadowSampler;
		RHI::RefCountPtr<Imp_Sampler> m_ShadowSamplerComparison;
		RHI::RefCountPtr<Imp_Buffer> m_DeferredLightingCB;
		RHI::RefCountPtr<Imp_ComputePipeline> m_PSO;



		RHI::RefCountPtr<Imp_BindingLayout> m_BindingLayout;
		BindingCache<APITag> m_BindingSets{ this->m_Device };



	};


	template<RHI::APITagConcept APITag>
	inline DeferredLightingPass<APITag>::DeferredLightingPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonPasses) :
		m_Device{ device },
		m_CommonPasses{ ::MoveTemp(commonPasses) } {
	}

	template<RHI::APITagConcept APITag>
	inline void DeferredLightingPass<APITag>::Init(const SharedPtr<ShaderFactory<APITag>>& shaderFactory) {
		this->m_ComputeShader = shaderFactory->CreateShader("Parting/Passes/deferred_lighting_cs.hlsl", "main", nullptr, RHI::RHIShaderType::Compute);

		RHI::RHISamplerDescBuilder samplerDescBuilder{}; samplerDescBuilder
			.Set_AddressModeUVW(RHI::RHISamplerAddressMode::Border)
			.Set_BorderColor(Color{ 1.f });

		this->m_ShadowSampler = this->m_Device->CreateSampler(samplerDescBuilder.Build());

		this->m_ShadowSamplerComparison = this->m_Device->CreateSampler(samplerDescBuilder.Set_ReductionType(RHI::RHISamplerReductionType::Comparison).Build());

		this->m_DeferredLightingCB = this->m_Device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(DeferredLightingConstants))
			.Set_MaxVersions(c_MaxRenderPassConstantBufferVersions)
			.Set_DebugName(_W("DeferredLightingPass/Constants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		this->m_BindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Compute)
			.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(1))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(2))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(3))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(8))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(9))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(10))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(11))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(12))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(14))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(15))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(16))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(17))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_UAV(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(1))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(2))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(3))
			.Build()
		);

		this->m_PSO = this->m_Device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
		.Set_CS(this->m_ComputeShader)
			.AddBindingLayout(this->m_BindingLayout)
			.Build()
			);
	}

	template<RHI::APITagConcept APITag>
	inline void DeferredLightingPass<APITag>::ResetBindingCache(void) {
		this->m_BindingSets.Clear();
	}

	template<RHI::APITagConcept APITag>
	void DeferredLightingPass<APITag>::Render(Imp_CommandList* commandList, const ICompositeView& compositeView, const Inputs& inputs, Math::VecF2 randomOffset) {
		ASSERT(nullptr != inputs.Depth);
		ASSERT(nullptr != inputs.GBufferNormals);
		ASSERT(nullptr != inputs.GBufferDiffuse);
		ASSERT(nullptr != inputs.GBufferSpecular);
		ASSERT(nullptr != inputs.GBufferEmissive);
		ASSERT(nullptr != inputs.Output);

		commandList->BeginMarker("DeferredLighting");

		DeferredLightingConstants deferredConstants{};
		deferredConstants.RandomOffset = randomOffset;
		deferredConstants.NoisePattern[0] = Math::VecF4{ 0.059f, 0.529f, 0.176f, 0.647f };
		deferredConstants.NoisePattern[1] = Math::VecF4{ 0.765f, 0.294f, 0.882f, 0.412f };
		deferredConstants.NoisePattern[2] = Math::VecF4{ 0.235f, 0.706f, 0.118f, 0.588f };
		deferredConstants.NoisePattern[3] = Math::VecF4{ 0.941f, 0.471f, 0.824f, 0.353f };
		deferredConstants.AmbientColorTop = Math::VecF4{ inputs.AmbientColorTop, 0.f };
		deferredConstants.AmbientColorBottom = Math::VecF4{ inputs.AmbientColorBottom, 0.f };
		deferredConstants.EnableAmbientOcclusion = (nullptr != inputs.AmbientOcclusion);
		deferredConstants.IndirectDiffuseScale = 1.f;
		deferredConstants.IndirectSpecularScale = 1.f;

		Imp_Texture* shadowMapTexture{ nullptr };

		Uint32 numShadows{ 0 };

		if (nullptr != inputs.Lights) {
			for (const auto& light : *inputs.Lights) {
				if (nullptr != light->ShadowMap) {
					if (nullptr == shadowMapTexture) {
						shadowMapTexture = light->ShadowMap->Get_Texture();
						deferredConstants.ShadowMapTextureSize = Math::VecF2{ light->ShadowMap->Get_TextureSize() };
					}
					else if (shadowMapTexture != light->ShadowMap->Get_Texture()) {
						LOG_ERROR("All lights submitted to DeferredLightingPass::Render(...) must use the same shadow map textures");
						return;
					}
				}

				if (deferredConstants.NumLights >= DEFERRED_MAX_LIGHTS) {//TODO :
					LOG_ERROR("Maximum number of active lights (%d) exceeded in DeferredLightingPass" /*,DEFERRED_MAX_LIGHTS*/);
					break;

				}

				LightConstants& lightConstants{ deferredConstants.Lights[deferredConstants.NumLights] };
				light->FillLightConstants(lightConstants);

				if (nullptr != light->ShadowMap) {
					for (Uint32 cascade = 0; cascade < light->ShadowMap->Get_NumberOfCascades(); ++cascade)
						if (numShadows < DEFERRED_MAX_SHADOWS) {
							light->ShadowMap->Get_Cascade(cascade)->FillShadowConstants(deferredConstants.Shadows[numShadows]);
							lightConstants.ShadowCascades[cascade] = numShadows;
							++numShadows;
						}

					for (Uint32 perObjectShadow = 0; perObjectShadow < light->ShadowMap->Get_NumberOfPerObjectShadows(); ++perObjectShadow)
						if (numShadows < DEFERRED_MAX_SHADOWS) {
							light->ShadowMap->Get_PerObjectShadow(perObjectShadow)->FillShadowConstants(deferredConstants.Shadows[numShadows]);
							lightConstants.PerObjectShadows[perObjectShadow] = numShadows;
							++numShadows;
						}
				}

				++deferredConstants.NumLights;
			}
		}

		Imp_Texture* lightProbeDiffuse{ nullptr };
		Imp_Texture* lightProbeSpecular{ nullptr };
		Imp_Texture* lightProbeEnvironmentBrdf{ nullptr };

		if (inputs.LightProbes) {
			for (const auto& probe : *inputs.LightProbes) {
				if (!probe->Is_Active())
					continue;

				LightProbeConstants& lightProbeConstants{ deferredConstants.LightProbes[deferredConstants.NumLightProbes] };
				probe->FillLightProbeConstants(lightProbeConstants);

				++deferredConstants.NumLightProbes;

				if (deferredConstants.NumLightProbes >= DEFERRED_MAX_LIGHT_PROBES) {
					LOG_ERROR("Maximum number of active light probes (%d) exceeded in DeferredLightingPass"/*, DEFERRED_MAX_LIGHT_PROBES*/);

					break;
				}

				if (nullptr == lightProbeDiffuse || nullptr == lightProbeSpecular || nullptr == lightProbeEnvironmentBrdf) {
					lightProbeDiffuse = probe->DiffuseMap;
					lightProbeSpecular = probe->SpecularMap;
					lightProbeEnvironmentBrdf = probe->EnvironmentBRDF;
				}
				else if (lightProbeDiffuse != probe->DiffuseMap || lightProbeSpecular != probe->SpecularMap || lightProbeEnvironmentBrdf != probe->EnvironmentBRDF) {
					LOG_ERROR("All light probes submitted to DeferredLightingPass::Render(...) must use the same set of textures");

					return;
				}
			}
		}

		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* view{ compositeView.Get_ChildView(ViewType::PLANAR, viewIndex) };
			const auto& viewSubresources{ view->Get_Subresources() };

			const auto bindingSet{ this->m_BindingSets.GetOrCreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
				.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_DeferredLightingCB))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, nullptr != shadowMapTexture ? shadowMapTexture : this->m_CommonPasses->m_BlackTexture2DArray.Get()))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(1, nullptr != lightProbeDiffuse ? lightProbeDiffuse : this->m_CommonPasses->m_BlackCubeMapArray.Get()))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(2, nullptr != lightProbeSpecular ? lightProbeSpecular : this->m_CommonPasses->m_BlackCubeMapArray.Get()))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(3, nullptr != lightProbeEnvironmentBrdf ? lightProbeEnvironmentBrdf : this->m_CommonPasses->m_BlackTexture.Get()))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(8, inputs.Depth,RHI::RHIFormat::UNKNOWN,viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(9, inputs.GBufferDiffuse, RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(10, inputs.GBufferSpecular, RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(11, inputs.GBufferNormals, RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(12, inputs.GBufferEmissive, RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(14, nullptr != inputs.IndirectDiffuse ? inputs.IndirectDiffuse : this->m_CommonPasses->m_BlackTexture.Get(), RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(15, nullptr != inputs.IndirectSpecular ? inputs.IndirectSpecular : this->m_CommonPasses->m_BlackTexture.Get(), RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(16, nullptr != inputs.ShadowChannels ? inputs.ShadowChannels : this->m_CommonPasses->m_BlackTexture.Get()))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(17, nullptr != inputs.AmbientOcclusion ? inputs.AmbientOcclusion : this->m_CommonPasses->m_WhiteTexture.Get(), RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(0, inputs.Output, RHI::RHIFormat::UNKNOWN, viewSubresources))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, this->m_ShadowSampler))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(1, this->m_ShadowSamplerComparison))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(2, this->m_CommonPasses->m_LinearWrapSampler))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(3, this->m_CommonPasses->m_LinearClampSampler))
				.Build(),
				this->m_BindingLayout
			) };

			view->FillPlanarViewConstants(deferredConstants.View);
			commandList->WriteBuffer(this->m_DeferredLightingCB, &deferredConstants, sizeof(DeferredLightingConstants));

			commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
			.Set_Pipeline(this->m_PSO)
				.AddBindingSet(bindingSet)
				.Build()
				);

			const auto& viewExtent{ view->Get_ViewExtent() };
			commandList->Dispatch(Math::DivCeil(viewExtent.Extent.Width, 16u), Math::DivCeil(viewExtent.Extent.Height, 16u));
		}

		commandList->EndMarker();
	}


}