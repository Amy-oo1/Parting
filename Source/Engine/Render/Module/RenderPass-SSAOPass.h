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

#include "Shader/ssao_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	namespace _NameSpace_SSAOPass {
		struct Parameters final {
			float Amount{ 2.f };
			float BackgroundViewDepth{ 100.f };
			float RadiusWorld{ 0.5f };
			float SurfaceBias{ 0.1f };
			float PowerExponent{ 2.f };
			bool EnableBlur{ true };
			float BlurSharpness{ 16.f };
		};

		struct CreateParameters final {
			Math::VecI2 Dimensions{ 0 };
			bool InputLinearDepth{ false };
			bool OctEncodedNormals{ false };
			bool DirectionalOcclusion{ false };
			Int32 NumBindingSets{ 1 };
		};
	}


	template<RHI::APITagConcept APITag>
	class SSAOPass final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;

		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;

		struct SubPass final {
			RHI::RefCountPtr<Imp_Shader> Shader;
			RHI::RefCountPtr<Imp_BindingLayout> BindingLayout;
			Vector<RHI::RefCountPtr<Imp_BindingSet>> BindingSets;
			RHI::RefCountPtr<Imp_ComputePipeline> Pipeline;
		};

	public:
		using Parameters = _NameSpace_SSAOPass::Parameters;
		using CreateParameters = _NameSpace_SSAOPass::CreateParameters;


	public:
		SSAOPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>>  commonPasses, const CreateParameters& params);

		SSAOPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, Imp_Texture* gbufferDepth, Imp_Texture* gbufferNormals, Imp_Texture* destinationTexture);

		~SSAOPass(void) = default;

	public:
		void CreateBindingSet(Imp_Texture* gbufferDepth, Imp_Texture* gbufferNormals, Imp_Texture* destinationTexture, Uint32 bindingSetIndex);

		void Render(Imp_CommandList* commandList, const SSAOPass::Parameters& params, const ICompositeView& compositeView, Uint32 bindingSetIndex = 0);

	private:
		SubPass m_Deinterleave;
		SubPass m_Compute;
		SubPass m_Blur;

		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_Buffer> m_ConstantBuffer;

		RHI::RefCountPtr<Imp_Texture> m_DeinterleavedDepth;
		RHI::RefCountPtr<Imp_Texture> m_DeinterleavedOcclusion;
		Math::VecF2 m_QuantizedGbufferTextureSize;

		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;

	};




	template<RHI::APITagConcept APITag>
	inline SSAOPass<APITag>::SSAOPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, const CreateParameters& params) :
		m_Device{ device },
		m_CommonPasses{ commonPasses },
		m_QuantizedGbufferTextureSize{ static_cast<float>(params.Dimensions.X),static_cast<float>(params.Dimensions.Y) } {

		this->m_ConstantBuffer = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(Shader::SsaoConstants))
			.Set_MaxVersions(c_MaxRenderPassConstantBufferVersions)
			.Set_DebugName(_W("SSAOConstants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		RHI::RHITextureDescBuilder textureBuilder{}; textureBuilder
			.Set_Width((params.Dimensions.X + 3) / 4).Set_Height((params.Dimensions.Y + 3) / 4)
			.Set_ArraySize(16)
			.Set_Dimension(RHI::RHITextureDimension::Texture2DArray)
			.Set_IsUAV(true)
			.Set_InitialState(RHI::RHIResourceState::ShaderResource)
			.Set_KeepInitialState(true);

		this->m_DeinterleavedDepth = device->CreateTexture(textureBuilder
			.Set_Format(RHI::RHIFormat::R32_FLOAT)
			.Set_DebugName("SSAO/DeinterleavedDepth")
			.Build()
		);

		this->m_DeinterleavedOcclusion = device->CreateTexture(textureBuilder
			.Set_Format(params.DirectionalOcclusion ? RHI::RHIFormat::RGBA16_FLOAT : RHI::RHIFormat::R8_UNORM)
			.Set_DebugName("SSAO/DeinterleavedOcclusion")
			.Build()
		);

		{
			Vector<ShaderMacro> macros{
				ShaderMacro{.Name{ String{ "LINEAR_DEPTH" } }, .Definition{ String{ params.InputLinearDepth ? "1" : "0" } } }
			};
			this->m_Deinterleave.Shader = shaderFactory->CreateShader("Parting/Passes/ssao_deinterleave_cs.hlsl", "main", &macros, RHI::RHIShaderType::Compute);

			this->m_Deinterleave.BindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::Compute)
				.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_UAV(0))
				.Build()
			);

			this->m_Deinterleave.BindingSets.resize(params.NumBindingSets);

			this->m_Deinterleave.Pipeline = device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
			.Set_CS(this->m_Deinterleave.Shader)
				.AddBindingLayout(this->m_Deinterleave.BindingLayout)
				.Build()
				);
		}

		{
			Vector<ShaderMacro> macros{
				ShaderMacro{.Name{ String{ "OCT_ENCODED_NORMALS" } }, .Definition{ String{ params.OctEncodedNormals ? "1" : "0" } } },
				ShaderMacro{.Name{ String{ "DIRECTIONAL_OCCLUSION" } }, .Definition{ String{ params.DirectionalOcclusion ? "1" : "0" } } }
			};
			this->m_Compute.Shader = shaderFactory->CreateShader("Parting/Passes/ssao_compute_cs.hlsl", "main", &macros, RHI::RHIShaderType::Compute);

			this->m_Compute.BindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::Compute)
				.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(1))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_UAV(0))
				.Build()
			);

			this->m_Compute.BindingSets.resize(params.NumBindingSets);

			this->m_Compute.Pipeline = device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
			.Set_CS(this->m_Compute.Shader)
				.AddBindingLayout(this->m_Compute.BindingLayout)
				.Build()
				);
		}

		{
			Vector<ShaderMacro> macros{
				ShaderMacro{.Name{ String{ "DIRECTIONAL_OCCLUSION" } }, .Definition{ String{ params.DirectionalOcclusion ? "1" : "0" } } }
			};
			this->m_Blur.Shader = shaderFactory->CreateShader("Parting/Passes/ssao_blur_cs.hlsl", "main", &macros, RHI::RHIShaderType::Compute);

			this->m_Blur.BindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::Compute)
				.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(1))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_UAV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Sampler(0))
				.Build()
			);

			this->m_Blur.BindingSets.resize(params.NumBindingSets);

			this->m_Blur.Pipeline = device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
			.Set_CS(this->m_Blur.Shader)
				.AddBindingLayout(this->m_Blur.BindingLayout)
				.Build()
				);
		}
	}

	template<RHI::APITagConcept APITag>
	inline SSAOPass<APITag>::SSAOPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, Imp_Texture* gbufferDepth, Imp_Texture* gbufferNormals, Imp_Texture* destinationTexture) :
		SSAOPass{ device, shaderFactory, commonPasses, CreateParameters{.Dimensions{ static_cast<Int32>(gbufferDepth->Get_Desc().Extent.Width), static_cast<Int32>(gbufferDepth->Get_Desc().Extent.Height) } } }//TODO 
	{
		const auto& depthDesc{ gbufferDepth->Get_Desc() };
		const auto& normalsDesc{ gbufferNormals->Get_Desc() };
		ASSERT(depthDesc.SampleCount == normalsDesc.SampleCount);
		ASSERT(depthDesc.SampleCount == 1); // more is currently unsupported
		ASSERT(depthDesc.Dimension == RHI::RHITextureDimension::Texture2D); // arrays are currently unsupported

		this->CreateBindingSet(gbufferDepth, gbufferNormals, destinationTexture, 0);
	}

	template<RHI::APITagConcept APITag>
	inline void SSAOPass<APITag>::CreateBindingSet(Imp_Texture* gbufferDepth, Imp_Texture* gbufferNormals, Imp_Texture* destinationTexture, Uint32 bindingSetIndex) {
		RHI::RHIBindingSetDescBuilder<APITag> DeinterleaveBindingSetDescBuilder;

		this->m_Deinterleave.BindingSets[bindingSetIndex] = this->m_Device->CreateBindingSet(DeinterleaveBindingSetDescBuilder
			.Reset()
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ConstantBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, gbufferDepth))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(0, this->m_DeinterleavedDepth))
			.Build(),
			this->m_Deinterleave.BindingLayout.Get()
		);

		this->m_Compute.BindingSets[bindingSetIndex] = this->m_Device->CreateBindingSet(DeinterleaveBindingSetDescBuilder
			.Reset()
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ConstantBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, this->m_DeinterleavedDepth))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(1, gbufferNormals))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(0, this->m_DeinterleavedOcclusion))
			.Build(),
			this->m_Compute.BindingLayout.Get());

		this->m_Blur.BindingSets[bindingSetIndex] = this->m_Device->CreateBindingSet(DeinterleaveBindingSetDescBuilder
			.Reset()
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ConstantBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, this->m_DeinterleavedDepth))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(1, this->m_DeinterleavedOcclusion))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(0, destinationTexture))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, this->m_CommonPasses->m_PointClampSampler))
			.Build(),
			this->m_Blur.BindingLayout
		);
	}

	template<RHI::APITagConcept APITag>
	inline void SSAOPass<APITag>::Render(Imp_CommandList* commandList, const SSAOPass::Parameters& params, const ICompositeView& compositeView, Uint32 bindingSetIndex) {
		ASSERT(nullptr != this->m_Deinterleave.BindingSets[bindingSetIndex]);
		ASSERT(nullptr != this->m_Compute.BindingSets[bindingSetIndex]);
		ASSERT(nullptr != this->m_Blur.BindingSets[bindingSetIndex]);

		commandList->BeginMarker("SSAO");

		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); viewIndex++)
		{
			const IView* view{ compositeView.Get_ChildView(ViewType::PLANAR, viewIndex) };

			const Math::MatF44 projectionMatrix{ view->Get_ProjectionMatrix(false) };

			const auto& viewExtent{ view->Get_ViewExtent() };//NOTE : retun rvalue
			const auto quarterResExtent = RHI::RHIRect2D::Build(
				viewExtent.Offset.X / 4,
				viewExtent.Offset.Y / 4,
				(viewExtent.Extent.Width + 3) / 4,
				(viewExtent.Extent.Height + 3) / 4
			);

			Shader::SsaoConstants ssaoConstants{};
			view->FillPlanarViewConstants(ssaoConstants.View);

			ssaoConstants.ClipToView = Math::VecF2{
				projectionMatrix[2].W / projectionMatrix[0].X,
				projectionMatrix[2].W / projectionMatrix[1].Y
			};
			ssaoConstants.InvQuantizedGbufferSize = 1.f / this->m_QuantizedGbufferTextureSize;
			ssaoConstants.QuantizedViewportOrigin = Math::VecI2{ static_cast<Int32>(quarterResExtent.Offset.X), static_cast<Int32>(quarterResExtent.Offset.Y) } *4;//TODO
			ssaoConstants.Amount = params.Amount;
			ssaoConstants.InvBackgroundViewDepth = (params.BackgroundViewDepth > 0.f) ? 1.f / params.BackgroundViewDepth : 0.f;
			ssaoConstants.RadiusWorld = params.RadiusWorld;
			ssaoConstants.SurfaceBias = params.SurfaceBias;
			ssaoConstants.PowerExponent = params.PowerExponent;
			ssaoConstants.RadiusToScreen = 0.5f * ssaoConstants.View.ViewportSize.Y * Math::Abs(projectionMatrix[1].Y);
			commandList->WriteBuffer(this->m_ConstantBuffer, &ssaoConstants, sizeof(ssaoConstants));

			Uint32 dispatchWidth{ (quarterResExtent.Extent.Width + 7) / 8 };
			Uint32 dispatchHeight{ (quarterResExtent.Extent.Height + 7) / 8 };

			commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
			.Set_Pipeline(this->m_Deinterleave.Pipeline)
				.AddBindingSet(this->m_Deinterleave.BindingSets[bindingSetIndex])
				.Build()
				);
			commandList->Dispatch(dispatchWidth, dispatchHeight, 1);

			commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
			.Set_Pipeline(this->m_Compute.Pipeline)
				.AddBindingSet(this->m_Compute.BindingSets[bindingSetIndex])
				.Build()
				);
			commandList->Dispatch(dispatchWidth, dispatchHeight, 16);

			dispatchWidth = (viewExtent.Extent.Width + 15) / 16;//TODO
			dispatchHeight = (viewExtent.Extent.Height + 15) / 16;

			commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
			.Set_Pipeline(this->m_Blur.Pipeline)
				.AddBindingSet(this->m_Blur.BindingSets[bindingSetIndex])
				.Build()
				);
			commandList->Dispatch(dispatchWidth, dispatchHeight, 1);
		}

		commandList->EndMarker();
	}

}