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

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/RenderPass-Base.h"

#include "Engine/Render/Module/SceneTypes.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"

#include "Shader/tonemapping_cb.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {


	namespace _NameSpace_ToneMappingPass {
		struct Parameters final {
			float HistogramLowPercentile{ 0.8f };
			float HistogramHighPercentile{ 0.95f };
			float EyeAdaptationSpeedUp{ 1.f };
			float EyeAdaptationSpeedDown{ 0.5f };
			float MinAdaptedLuminance{ 0.02f };
			float MaxAdaptedLuminance{ 0.5f };
			float ExposureBias{ -0.5f };
			float WhitePoint{ 3.f };
			bool EnableColorLUT{ true };
		};

		static const float MinLogLuminance{ -10 }; // TODO: figure out how to set these properly
		static const float MaxLogLuminamce{ 4 };

	}



	template<RHI::APITagConcept APITag>
	class ToneMappingPass final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;

	public:
		using Parameters = _NameSpace_ToneMappingPass::Parameters;

		struct CreateParameters final {
			bool IsTextureArray{ false };
			Uint32 HistogramBins{ 256 };
			Uint32 NumConstantBufferVersions{ 16 };
			Imp_Buffer* ExposureBufferOverride{ nullptr };
			Imp_Texture* ColorLUT{ nullptr };
		};

	public:
		ToneMappingPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, SharedPtr<FrameBufferFactory<APITag>> framebufferFactory, const ICompositeView& compositeView, const CreateParameters& params);

		~ToneMappingPass(void) = default;


	public:



		void AdvanceFrame(float frameTime) { this->m_FrameTime = frameTime; }

		void AddFrameToHistogram(Imp_CommandList* commandList, const ICompositeView& compositeView, Imp_Texture* sourceTexture);

		auto Get_ExposureBuffer(void) -> RHI::RefCountPtr<Imp_Buffer>;

		void ResetExposure(Imp_CommandList* commandList, float initialExposure);

		void ResetHistogram(Imp_CommandList* commandList);

		void ComputeExposure(Imp_CommandList* commandList, const ToneMappingPass<APITag>::Parameters& params);

		void Render(Imp_CommandList* commandList, const ToneMappingPass<APITag>::Parameters& params, const ICompositeView& compositeView, Imp_Texture* sourceTexture);

		void SimpleRender(Imp_CommandList* commandList, const ToneMappingPass<APITag>::Parameters& params, const ICompositeView& compositeView, Imp_Texture* sourceTexture);


	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_Shader> m_PixelShader;
		RHI::RefCountPtr<Imp_Shader> m_HistogramComputeShader;
		RHI::RefCountPtr<Imp_Shader> m_ExposureComputeShader;
		Uint32 m_HistogramBins;

		RHI::RefCountPtr<Imp_Buffer> m_ToneMappingCB;
		RHI::RefCountPtr<Imp_Buffer> m_HistogramBuffer;
		RHI::RefCountPtr<Imp_Buffer> m_ExposureBuffer;
		float m_FrameTime{ 0.f };

		RHI::RefCountPtr<Imp_Texture> m_ColorLUT;
		float m_ColorLUTSize{ 0.f };

		RHI::RefCountPtr<Imp_BindingLayout> m_HistogramBindingLayout;
		RHI::RefCountPtr<Imp_ComputePipeline> m_HistogramPSO;

		RHI::RefCountPtr<Imp_BindingLayout> m_ExposureBindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_ExposureBindingSet;
		RHI::RefCountPtr<Imp_ComputePipeline> m_ExposurePSO;

		RHI::RefCountPtr<Imp_BindingLayout> m_RenderBindingLayout;
		RHI::RefCountPtr<Imp_GraphicsPipeline> m_RenderPSO;

		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;
		SharedPtr<FrameBufferFactory<APITag>> m_FrameBufferFactory;

		UnorderedMap<Imp_Texture*, RHI::RefCountPtr<Imp_BindingSet>> m_HistogramBindingSets;
		UnorderedMap<Imp_Texture*, RHI::RefCountPtr<Imp_BindingSet>> m_RenderBindingSets;



	};







	template<RHI::APITagConcept APITag>
	inline ToneMappingPass<APITag>::ToneMappingPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, SharedPtr<FrameBufferFactory<APITag>> framebufferFactory, const ICompositeView& compositeView, const CreateParameters& params) :
		m_Device{ device },
		m_HistogramBins{ params.HistogramBins },
		m_CommonPasses{ ::MoveTemp(commonPasses) },
		m_FrameBufferFactory{ ::MoveTemp(framebufferFactory) } {

		ASSERT(params.HistogramBins <= 256);

		const IView* sampleView{ compositeView.Get_ChildView(ViewType::PLANAR, 0) };
		Imp_FrameBuffer* sampleFramebuffer{ this->m_FrameBufferFactory->Get_FrameBuffer(*sampleView) };

		{
			Vector<ShaderMacro> Macros{
				ShaderMacro{.Name{ String{ "HISTOGRAM_BINS" } }, .Definition{ ::IntegralToString(params.HistogramBins) } },
				ShaderMacro{.Name{ String{ "SOURCE_ARRAY" } }, .Definition{ String{ params.IsTextureArray ? "1" : "0" } } }
			};

			this->m_HistogramComputeShader = shaderFactory->CreateShader(String{ "Parting/Passes/histogram_cs.hlsl" }, String{ "main" }, &Macros, RHI::RHIShaderType::Compute);
			this->m_ExposureComputeShader = shaderFactory->CreateShader(String{ "Parting/Passes/exposure_cs.hlsl" }, String{ "main" }, &Macros, RHI::RHIShaderType::Compute);
			this->m_PixelShader = shaderFactory->CreateShader(String{ "Parting/Passes/tonemapping_ps.hlsl" }, String{ "main" }, &Macros, RHI::RHIShaderType::Pixel);
		}

		this->m_ToneMappingCB = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(Shader::ToneMappingConstants))
			.Set_MaxVersions(params.NumConstantBufferVersions)
			.Set_DebugName(_W("ToneMappingConstants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		this->m_HistogramBuffer = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(Uint32) * this->m_HistogramBins)
			.Set_DebugName(_W("HistogramBuffer"))
			.Set_Format(RHI::RHIFormat::R32_UINT)
			.Set_CanHaveUAVs(true)
			.Set_CanHaveTypedViews(true)
			.Set_InitialState(RHI::RHIResourceState::UnorderedAccess)
			.Set_KeepInitialState(true)
			.Build()
		);

		if (nullptr != params.ExposureBufferOverride)
			this->m_ExposureBuffer = params.ExposureBufferOverride;
		else
			this->m_ExposureBuffer = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
				.Set_ByteSize(sizeof(Uint32))
				.Set_DebugName(_W("ExposureBuffer"))
				.Set_Format(RHI::RHIFormat::R32_UINT)
				.Set_CanHaveUAVs(true)
				.Set_CanHaveTypedViews(true)
				.Set_InitialState(RHI::RHIResourceState::UnorderedAccess)
				.Set_KeepInitialState(true)
				.Build()
			);

		this->m_ColorLUT = this->m_CommonPasses->m_BlackTexture;
		if (nullptr != params.ColorLUT) {
			const auto& desc{ params.ColorLUT->Get_Desc() };
			this->m_ColorLUTSize = static_cast<float>(desc.Extent.Height);

			if (desc.Extent.Width != desc.Extent.Height * desc.Extent.Height) {
				LOG_ERROR("Color LUT texture size must be: width = (n*n), height = (n)");
				this->m_ColorLUTSize = 0.f;
			}
			else
				this->m_ColorLUT = params.ColorLUT;
		}

		{
			this->m_HistogramBindingLayout = device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::Compute)
				.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::TypedBuffer_UAV(0))
				.Build()
			);

			this->m_HistogramPSO = device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
			.Set_CS(this->m_HistogramComputeShader)
				.AddBindingLayout(this->m_HistogramBindingLayout)
				.Build()
				);
		}

		{
			::Tie(this->m_ExposureBindingLayout, this->m_ExposureBindingSet) = device->CreateBindingLayoutAndSet(
				RHI::RHIShaderType::Compute,
				0,
				RHI::RHIBindingSetDescBuilder<APITag>{}
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ToneMappingCB))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::TypedBuffer_SRV(0, this->m_HistogramBuffer))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::TypedBuffer_UAV(0, this->m_ExposureBuffer))
				.Build()
				);

			this->m_ExposurePSO = device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
			.Set_CS(this->m_ExposureComputeShader)
				.AddBindingLayout(this->m_ExposureBindingLayout)
				.Build()
				);
		}

		{
			this->m_RenderBindingLayout = device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::Pixel)
				.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::TypedBuffer_SRV(1))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(2))
				.AddBinding(RHI::RHIBindingLayoutItem::Sampler(0))
				.Build()
			);

			this->m_RenderPSO = device->CreateGraphicsPipeline(RHI::RHIGraphicsPipelineDescBuilder<APITag>{}
			.Set_PrimType(RHI::RHIPrimitiveType::TriangleStrip)
				.Set_VS(this->m_CommonPasses->m_FullscreenVS)
				.Set_PS(this->m_PixelShader)
				.AddBindingLayout(this->m_RenderBindingLayout)
				.Set_CullMode(RHI::RHIRasterCullMode::None)
				.Set_DepthTestEnable(false)
				.Build(),
				sampleFramebuffer
				);
		}
	}



	template<RHI::APITagConcept APITag>
	inline void ToneMappingPass<APITag>::AddFrameToHistogram(Imp_CommandList* commandList, const ICompositeView& compositeView, Imp_Texture* sourceTexture) {
		auto& bindingSet{ this->m_HistogramBindingSets[sourceTexture] };
		if (nullptr == bindingSet)
			bindingSet = this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
		.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ToneMappingCB))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, sourceTexture))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::TypedBuffer_UAV(0, this->m_HistogramBuffer))
			.Build(),
			this->m_HistogramBindingLayout
			);

		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* view{ compositeView.Get_ChildView(ViewType::PLANAR, viewIndex) };

			const auto& viewportState{ view->Get_ViewportState() };

			for (Uint32 viewportIndex = 0; viewportIndex < viewportState.ScissorCount; ++viewportIndex) {
				Shader::ToneMappingConstants toneMappingConstants{};
				{
					toneMappingConstants.LogLuminanceScale = 1.0f / (_NameSpace_ToneMappingPass::MaxLogLuminamce - _NameSpace_ToneMappingPass::MinLogLuminance);
					toneMappingConstants.LogLuminanceBias = -_NameSpace_ToneMappingPass::MinLogLuminance * toneMappingConstants.LogLuminanceScale;

					const auto& scissor{ viewportState.ScissorRects[viewportIndex] };
					toneMappingConstants.ViewOrigin = Math::VecU2{ scissor.Offset.X, scissor.Offset.Y };
					toneMappingConstants.ViewSize = Math::VecU2{ scissor.Extent.Width, scissor.Extent.Height };
					toneMappingConstants.SourceSlice = view->Get_Subresources().BaseArraySlice;
				}

				commandList->WriteBuffer(this->m_ToneMappingCB, &toneMappingConstants, sizeof(decltype(toneMappingConstants)));

				commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
				.Set_Pipeline(this->m_HistogramPSO)
					.AddBindingSet(bindingSet)
					.Build()
					);

				Math::VecU2 numGroups{ Math::DivCeil(toneMappingConstants.ViewSize,Math::VecU2{ 16u }) };
				commandList->Dispatch(numGroups.X, numGroups.Y, 1);
			}
		}
	}

	template<RHI::APITagConcept APITag>
	inline auto ToneMappingPass<APITag>::Get_ExposureBuffer(void) -> RHI::RefCountPtr<Imp_Buffer> {
		return this->m_ExposureBuffer;
	}

	template<RHI::APITagConcept APITag>
	inline void ToneMappingPass<APITag>::ResetExposure(Imp_CommandList* commandList, float initialExposure) {
		commandList->ClearBufferUInt(this->m_ExposureBuffer, *reinterpret_cast<Uint32*>(&initialExposure));//TODO : NOTE : float bit to int bit not value 
	}

	template<RHI::APITagConcept APITag>
	inline void ToneMappingPass<APITag>::ResetHistogram(Imp_CommandList* commandList) {
		commandList->ClearBufferUInt(this->m_HistogramBuffer, 0u);
	}

	template<RHI::APITagConcept APITag>
	inline void ToneMappingPass<APITag>::ComputeExposure(Imp_CommandList* commandList, const ToneMappingPass<APITag>::Parameters& params) {
		Shader::ToneMappingConstants toneMappingConstants{};
		{
			toneMappingConstants.LogLuminanceScale = _NameSpace_ToneMappingPass::MaxLogLuminamce - _NameSpace_ToneMappingPass::MinLogLuminance;
			toneMappingConstants.LogLuminanceBias = _NameSpace_ToneMappingPass::MinLogLuminance;
			toneMappingConstants.HistogramLowPercentile = Math::Clamp(params.HistogramLowPercentile, 0.f, 0.99f);
			toneMappingConstants.HistogramHighPercentile = Math::Clamp(params.HistogramHighPercentile, toneMappingConstants.HistogramLowPercentile, 1.f);
			toneMappingConstants.EyeAdaptationSpeedUp = params.EyeAdaptationSpeedUp;
			toneMappingConstants.EyeAdaptationSpeedDown = params.EyeAdaptationSpeedDown;
			toneMappingConstants.MinAdaptedLuminance = params.MinAdaptedLuminance;
			toneMappingConstants.MaxAdaptedLuminance = params.MaxAdaptedLuminance;
			toneMappingConstants.FrameTime = this->m_FrameTime;
		}

		commandList->WriteBuffer(this->m_ToneMappingCB, &toneMappingConstants, sizeof(decltype(toneMappingConstants)));

		commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
		.Set_Pipeline(this->m_ExposurePSO)
			.AddBindingSet(this->m_ExposureBindingSet)
			.Build()
			);
		commandList->Dispatch(1);
	}

	template<RHI::APITagConcept APITag>
	inline void ToneMappingPass<APITag>::Render(Imp_CommandList* commandList, const ToneMappingPass<APITag>::Parameters& params, const ICompositeView& compositeView, Imp_Texture* sourceTexture) {
		auto& bindingSet{ this->m_RenderBindingSets[sourceTexture] };
		if (nullptr == bindingSet)
			bindingSet = this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
		.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ToneMappingCB))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, sourceTexture))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::TypedBuffer_SRV(1, this->m_ExposureBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(2, this->m_ColorLUT))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, this->m_CommonPasses->m_LinearClampSampler))
			.Build(),
			this->m_RenderBindingLayout
			);

		bool enableColorLUT{ params.EnableColorLUT && this->m_ColorLUTSize > 0 };

		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* view{ compositeView.Get_ChildView(ViewType::PLANAR, viewIndex) };

			Shader::ToneMappingConstants toneMappingConstants{};
			{
				toneMappingConstants.ExposureScale = Math::Exp2f(params.ExposureBias);//TODO :
				toneMappingConstants.WhitePointInvSquared = 1.f / Math::Pow(params.WhitePoint, 2.f);
				toneMappingConstants.MinAdaptedLuminance = params.MinAdaptedLuminance;
				toneMappingConstants.MaxAdaptedLuminance = params.MaxAdaptedLuminance;
				toneMappingConstants.SourceSlice = view->Get_Subresources().BaseArraySlice;
				toneMappingConstants.ColorLUTTextureSize = enableColorLUT ? Math::VecF2(this->m_ColorLUTSize * this->m_ColorLUTSize, this->m_ColorLUTSize) : Math::VecF2::Zero();
				toneMappingConstants.ColorLUTTextureSizeInv = enableColorLUT ? 1.f / toneMappingConstants.ColorLUTTextureSize : Math::VecF2::Zero();
			}
			commandList->WriteBuffer(this->m_ToneMappingCB, &toneMappingConstants, sizeof(decltype(toneMappingConstants)));

			commandList->SetGraphicsState(RHI::RHIGraphicsStateBuilder<APITag>{}
			.Set_Pipeline(this->m_RenderPSO)
				.Set_FrameBuffer(this->m_FrameBufferFactory->Get_FrameBuffer(*view))
				.AddBindingSet(bindingSet)
				.Set_ViewportState(view->Get_ViewportState())
				.Build()
				);

			commandList->Draw(RHI::RHIDrawArguments{ .VertexCount{ 4 } });
		}
	}

	template<RHI::APITagConcept APITag>
	inline void ToneMappingPass<APITag>::SimpleRender(Imp_CommandList* commandList, const ToneMappingPass<APITag>::Parameters& params, const ICompositeView& compositeView, Imp_Texture* sourceTexture) {
		commandList->BeginMarker("ToneMapping");
		this->ResetHistogram(commandList);
		this->AddFrameToHistogram(commandList, compositeView, sourceTexture);
		this->ComputeExposure(commandList, params);
		this->Render(commandList, params, compositeView, sourceTexture);
		commandList->EndMarker();
	}
}