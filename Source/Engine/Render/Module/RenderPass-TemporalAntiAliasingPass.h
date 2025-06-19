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

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"
#include "Engine/Render/Module/View.h"

#include "Shader/taa_cb.h"



#endif // PARTING_MODULE_BUILD


namespace Parting {

	enum class TemporalAntiAliasingJitter :Uint8 {
		MSAA,
		Halton,
		R2,
		WhiteNoise
	};

	float VanDerCorput(Uint64 base, Uint64 index) {
		/*float ret = 0.0f;
		float denominator = float(base);
		while (index > 0)
		{
			size_t multiplier = index % base;
			ret += float(multiplier) / denominator;
			index = index / base;
			denominator *= base;
		}
		return ret;*/

		float result{ 0.f };
		float denom{ static_cast<float>(base) };
		while (index > 0) {
			result += (1.f / denom) * (index % base);
			index /= base;
			denom *= static_cast<float>(base);
		}
		return result;
	}

	namespace _NameSpace_TemporalAntiAliasingPass {
		struct Parameters final {
			float NewFrameWeight{ 0.1f };
			float ClampingFactor{ 1.0f };
			float MaxRadiance{ 10000.f };
			bool EnableHistoryClamping{ true };

			// Requires CreateParameters::historyClampRelax single channel [0, 1] mask to be provided. 
			// For texels with mask value of 0 the behavior is unchanged; for texels with mask value > 0, 
			// 'newFrameWeight' will be reduced and 'clampingFactor' will be increased proportionally. 
			bool UseHistoryClampRelax{ false };
		};
	}



	template<RHI::APITagConcept APITag>
	class TemporalAntiAliasingPass final {
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;

	public:
		struct CreateParameters final {
			Imp_Texture* SourceDepth{ nullptr };
			Imp_Texture* MotionVectors{ nullptr };
			Imp_Texture* UnresolvedColor{ nullptr };
			Imp_Texture* ResolvedColor{ nullptr };
			Imp_Texture* Feedback1{ nullptr };
			Imp_Texture* Feedback2{ nullptr };
			Imp_Texture* HistoryClampRelax{ nullptr };
			bool UseCatmullRomFilter{ true };
			Uint32 MotionVectorStencilMask{ 0 };
			Uint32 NumConstantBufferVersions{ 16 };
		};

		using Parameters = _NameSpace_TemporalAntiAliasingPass::Parameters;

	public:
		TemporalAntiAliasingPass(
			Imp_Device* device,
			SharedPtr<ShaderFactory<APITag>> shaderFactory,
			SharedPtr<CommonRenderPasses<APITag>> commonPasses,
			const ICompositeView& compositeView,
			const CreateParameters& params
		);
		~TemporalAntiAliasingPass(void) = default;


	public:

		void RenderMotionVectors(Imp_CommandList* commandList, const ICompositeView& compositeView, const ICompositeView& compositeViewPrevious, Math::VecF3 preViewTranslationDifference = Math::VecF3::Zero());

		void TemporalResolve(Imp_CommandList* commandList, const Parameters& params, bool feedbackIsValid, const ICompositeView& compositeViewInput, const ICompositeView& compositeViewOutput);

		void AdvanceFrame(void);

		void Set_Jitter(TemporalAntiAliasingJitter jitter) { this->m_Jitter = jitter; }

		STDNODISCARD Math::VecF2 Get_CurrentPixelOffset(void);



	private:
		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;

		RHI::RefCountPtr<Imp_Shader> m_MotionVectorPS;
		RHI::RefCountPtr<Imp_Shader> m_TemporalAntiAliasingCS;
		RHI::RefCountPtr<Imp_Sampler> m_BilinearSampler;
		RHI::RefCountPtr<Imp_Buffer> m_TemporalAntiAliasingCB;

		RHI::RefCountPtr<Imp_BindingLayout> m_MotionVectorsBindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_MotionVectorsBindingSet;
		RHI::RefCountPtr<Imp_GraphicsPipeline> m_MotionVectorsPSO;
		UniquePtr<FrameBufferFactory<APITag>> m_MotionVectorsFrameBufferFactory;

		RHI::RefCountPtr<Imp_BindingLayout> m_ResolveBindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_ResolveBindingSet;
		RHI::RefCountPtr<Imp_BindingSet> m_ResolveBindingSetPrevious;
		RHI::RefCountPtr<Imp_ComputePipeline> m_ResolvePSO;

		Uint32 m_FrameIndex{ 0 };
		Uint32 m_StencilMask{ 0 };
		Math::VecF2 m_ResolvedColorSize;

		Math::VecF2 m_R2Jitter{ Math::VecF2::Zero() };
		TemporalAntiAliasingJitter m_Jitter{ TemporalAntiAliasingJitter::MSAA };

		bool m_HasHistoryClampRelaxTexture{ false };



	};



	template<RHI::APITagConcept APITag>
	inline TemporalAntiAliasingPass<APITag>::TemporalAntiAliasingPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, const ICompositeView& compositeView, const CreateParameters& params) :
		m_CommonPasses{ ::MoveTemp(commonPasses) },
		m_StencilMask{ params.MotionVectorStencilMask } {

		const IView* sampleView{ compositeView.Get_ChildView(ViewType::PLANAR, 0) };

		const auto& unresolvedColorDesc{ params.UnresolvedColor->Get_Desc() };
		const auto& resolvedColorDesc{ params.ResolvedColor->Get_Desc() };
		const auto& feedback1Desc{ params.Feedback1->Get_Desc() };
		const auto& feedback2Desc{ params.Feedback2->Get_Desc() };

		ASSERT(feedback1Desc.Extent.Width == feedback2Desc.Extent.Width);
		ASSERT(feedback1Desc.Extent.Height == feedback2Desc.Extent.Height);
		ASSERT(feedback1Desc.Format == feedback2Desc.Format);
		ASSERT(feedback1Desc.IsUAV);
		ASSERT(feedback2Desc.IsUAV);
		ASSERT(resolvedColorDesc.IsUAV);

		this->m_ResolvedColorSize = Math::VecF2{ static_cast<float>(resolvedColorDesc.Extent.Width), static_cast<float>(resolvedColorDesc.Extent.Height) };

		bool useStencil{ false };
		RHI::RHIFormat stencilFormat = RHI::RHIFormat::UNKNOWN;
		if (params.MotionVectorStencilMask) {
			useStencil = true;

			RHI::RHIFormat depthFormat{ params.SourceDepth->Get_Desc().Format };

			if (depthFormat == RHI::RHIFormat::D24S8)
				stencilFormat = RHI::RHIFormat::X24G8_UINT;
			else if (depthFormat == RHI::RHIFormat::D32S8)
				stencilFormat = RHI::RHIFormat::X32G8_UINT;
			else
				LOG_ERROR("Stencil format not supported for motion vectors. Only D24S8 and D32S8 depth formats are supported.");
		}

		Vector<ShaderMacro> MotionVectorMacros{
			ShaderMacro{.Name{ String{ "USE_STENCIL" } }, .Definition{ String{ useStencil ? "1" : "0" } } }
		};
		this->m_MotionVectorPS = shaderFactory->CreateShader(String{ "Parting/Passes/motion_vectors_ps.hlsl" }, String{ "main" }, &MotionVectorMacros, RHI::RHIShaderType::Pixel);

		Vector<ShaderMacro> ResolveMacros{
			ShaderMacro{.Name{ "SAMPLE_COUNT" }, .Definition{::IntegralToString(unresolvedColorDesc.SampleCount) } },
			ShaderMacro{.Name{ "USE_CATMULL_ROM_FILTER" },.Definition{ params.UseCatmullRomFilter ? "1" : "0" } }
		};
		this->m_TemporalAntiAliasingCS = shaderFactory->CreateShader(String{ "Parting/Passes/taa_cs.hlsl" }, String{ "main" }, &ResolveMacros, RHI::RHIShaderType::Compute);

		this->m_BilinearSampler = device->CreateSampler(RHI::RHISamplerDescBuilder{}
			.Set_BorderColor(Color{ 0.f })
			.Set_AddressModeUVW(RHI::RHISamplerAddressMode::Border)
			.Build()
		);

		this->m_TemporalAntiAliasingCB = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(TemporalAntiAliasingConstants))
			.Set_MaxVersions(params.NumConstantBufferVersions)
			.Set_DebugName(_W("TemporalAntiAliasingConstants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		if (nullptr != params.SourceDepth) {
			RHI::RHIBindingSetDescBuilder<APITag> bindingSetDescBuilder{}; bindingSetDescBuilder
				.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_TemporalAntiAliasingCB))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, params.SourceDepth));
			if (useStencil)
				bindingSetDescBuilder.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(1, params.SourceDepth, stencilFormat));

			::Tie(this->m_MotionVectorsBindingLayout, this->m_MotionVectorsBindingSet) = device->CreateBindingLayoutAndSet(RHI::RHIShaderType::Pixel, 0, bindingSetDescBuilder.Build());

			this->m_MotionVectorsFrameBufferFactory = MakeUnique<FrameBufferFactory<APITag>>(device);
			this->m_MotionVectorsFrameBufferFactory->RenderTargets.assign({ params.MotionVectors });

			this->m_MotionVectorsPSO = device->CreateGraphicsPipeline(RHI::RHIGraphicsPipelineDescBuilder<APITag>{}
			.Set_PrimType(RHI::RHIPrimitiveType::TriangleStrip)
				.Set_VS(this->m_CommonPasses->m_FullscreenVS)
				.Set_PS(this->m_MotionVectorPS)
				.AddBindingLayout(this->m_MotionVectorsBindingLayout)
				.Set_CullMode(RHI::RHIRasterCullMode::None)
				.Set_DepthTestEnable(false)
				.Build(),

				this->m_MotionVectorsFrameBufferFactory->Get_FrameBuffer(*sampleView)
				);
		}

		{
			RHI::RHIBindingSetDescBuilder<APITag> bindingSetDescBuilder{}; bindingSetDescBuilder
				.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_TemporalAntiAliasingCB))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, this->m_BilinearSampler))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, params.UnresolvedColor))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(1, params.MotionVectors))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(2, params.Feedback1))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(0, params.ResolvedColor))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(1, params.Feedback2));

			this->m_HasHistoryClampRelaxTexture = (nullptr != params.HistoryClampRelax);
			if (nullptr != params.HistoryClampRelax)
				bindingSetDescBuilder.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(3, params.HistoryClampRelax));
			else// No relax mask, but we need to bind something to match the shader binding slots
				bindingSetDescBuilder.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(3, params.UnresolvedColor));

			::Tie(this->m_ResolveBindingLayout, this->m_ResolveBindingSet) = device->CreateBindingLayoutAndSet(RHI::RHIShaderType::Compute, 0, bindingSetDescBuilder.Build());

			// Swap resolvedColor and resolvedColorPrevious (t2 and u0)
			bindingSetDescBuilder.m_Desc.Bindings[4].ResourcePtr = params.Feedback2;//TODO :
			bindingSetDescBuilder.m_Desc.Bindings[6].ResourcePtr = params.Feedback1;//TODO : !!!! .....
			this->m_ResolveBindingSetPrevious = device->CreateBindingSet(bindingSetDescBuilder.Build(), this->m_ResolveBindingLayout);

			this->m_ResolvePSO = device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
			.Set_CS(this->m_TemporalAntiAliasingCS)
				.AddBindingLayout(this->m_ResolveBindingLayout)
				.Build()
				);
		}
	}

	template<RHI::APITagConcept APITag>
	inline void TemporalAntiAliasingPass<APITag>::RenderMotionVectors(Imp_CommandList* commandList, const ICompositeView& compositeView, const ICompositeView& compositeViewPrevious, Math::VecF3 preViewTranslationDifference) {
		ASSERT(compositeView.Get_NumChildViews(ViewType::PLANAR) == compositeViewPrevious.Get_NumChildViews(ViewType::PLANAR));
		ASSERT(nullptr != this->m_MotionVectorsPSO);

		commandList->BeginMarker("MotionVectors");

		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* view{ compositeView.Get_ChildView(ViewType::PLANAR, viewIndex) };
			const IView* viewPrevious{ compositeViewPrevious.Get_ChildView(ViewType::PLANAR, viewIndex) };

			const auto& viewportState{ view->Get_ViewportState() };

			// This pass only works for planar, single-viewport views
			ASSERT(1 == viewportState.ViewportCount);

			const auto& inputViewport{ viewportState.Viewports[0] };

			TemporalAntiAliasingConstants taaConstants{};
			{
				Math::AffineF3 viewReprojection{ Math::Inverse(view->Get_ViewMatrix()) * Math::Translation(-preViewTranslationDifference) * viewPrevious->Get_ViewMatrix() };
				taaConstants.ReprojectionMatrix = Math::Inverse(view->Get_ProjectionMatrix(false)) * Math::AffineToHomogeneous(viewReprojection) * viewPrevious->Get_ProjectionMatrix(false);
				taaConstants.InputViewOrigin = Math::VecF2{ inputViewport.MinX, inputViewport.MinY };
				taaConstants.InputViewSize = Math::VecF2{ inputViewport.Width(), inputViewport.Height() };
				taaConstants.StencilMask = this->m_StencilMask;
			}
			commandList->WriteBuffer(this->m_TemporalAntiAliasingCB, &taaConstants, sizeof(taaConstants));

			commandList->SetGraphicsState(RHI::RHIGraphicsStateBuilder<APITag>{}
			.Set_Pipeline(this->m_MotionVectorsPSO)
				.Set_FrameBuffer(this->m_MotionVectorsFrameBufferFactory->Get_FrameBuffer(*view))
				.Set_ViewportState(viewportState)
				.AddBindingSet(this->m_MotionVectorsBindingSet)
				.Build()

				);

			commandList->Draw(RHI::RHIDrawArguments{ .VertexCount{ 4 } });
		}

		commandList->EndMarker();
	}

	template<RHI::APITagConcept APITag>
	inline void TemporalAntiAliasingPass<APITag>::TemporalResolve(Imp_CommandList* commandList, const Parameters& params, bool feedbackIsValid, const ICompositeView& compositeViewInput, const ICompositeView& compositeViewOutput) {
		ASSERT(compositeViewInput.Get_NumChildViews(ViewType::PLANAR) == compositeViewOutput.Get_NumChildViews(ViewType::PLANAR));

		commandList->BeginMarker("TemporalAA");

		for (Uint32 viewIndex = 0; viewIndex < compositeViewInput.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* viewInput{ compositeViewInput.Get_ChildView(ViewType::PLANAR, viewIndex) };
			const IView* viewOutput{ compositeViewOutput.Get_ChildView(ViewType::PLANAR, viewIndex) };

			const auto& viewportInput{ viewInput->Get_ViewportState().Viewports[0] };
			const auto& viewportOutput{ viewOutput->Get_ViewportState().Viewports[0] };

			TemporalAntiAliasingConstants taaConstants{};
			{
				taaConstants.InputViewOrigin = Math::VecF2{ viewportInput.MinX, viewportInput.MinY };
				taaConstants.InputViewSize = Math::VecF2{ viewportInput.Width(), viewportInput.Height() };
				taaConstants.OutputViewOrigin = Math::VecF2{ viewportOutput.MinX, viewportOutput.MinY };
				taaConstants.OutputViewSize = Math::VecF2{ viewportOutput.Width(), viewportOutput.Height() };
				taaConstants.InputPixelOffset = viewInput->Get_PixelOffset();
				taaConstants.OutputTextureSizeInv = 1.f / this->m_ResolvedColorSize;
				taaConstants.InputOverOutputViewSize = taaConstants.InputViewSize / taaConstants.OutputViewSize;
				taaConstants.OutputOverInputViewSize = taaConstants.OutputViewSize / taaConstants.InputViewSize;
				taaConstants.ClampingFactor = params.EnableHistoryClamping ? params.ClampingFactor : -1.f;
				taaConstants.NewFrameWeight = feedbackIsValid ? params.NewFrameWeight : 1.f;
				taaConstants.PQC = Math::Clamp(params.MaxRadiance, 1e-4f, 1e8f);
				taaConstants.InvPQC = 1.f / taaConstants.PQC;
				taaConstants.UseHistoryClampRelax = (params.UseHistoryClampRelax && this->m_HasHistoryClampRelaxTexture) ? 1 : 0;
			}
			commandList->WriteBuffer(this->m_TemporalAntiAliasingCB, &taaConstants, sizeof(taaConstants));

			commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
			.Set_Pipeline(this->m_ResolvePSO)
				.AddBindingSet(this->m_ResolveBindingSet)
				.Build()
				);

			Math::VecI2 viewportSize{ Math::VecI2{ taaConstants.OutputViewSize } };
			Math::VecI2 gridSize{ Math::DivCeil(viewportSize, Math::VecI2{ 16 }) };//TODO :

			commandList->Dispatch(gridSize.X, gridSize.Y, 1);
		}

		commandList->EndMarker();
	}

	template<RHI::APITagConcept APITag>
	inline void TemporalAntiAliasingPass<APITag>::AdvanceFrame(void) {
		++this->m_FrameIndex;

		::Swap(this->m_ResolveBindingSet, this->m_ResolveBindingSetPrevious);

		if (m_Jitter == TemporalAntiAliasingJitter::R2) {
			// Advance R2 jitter sequence
			// http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

			constexpr float g{ 1.32471795724474602596f };
			constexpr float a1{ 1.0f / g };
			constexpr float a2{ 1.0f / (g * g) };
			this->m_R2Jitter[0] = Math::ModPositive(this->m_R2Jitter[0] + a1, 1.0f);
			this->m_R2Jitter[1] = Math::ModPositive(this->m_R2Jitter[1] + a2, 1.0f);
		}
	}

	template<RHI::APITagConcept APITag>
	inline Math::VecF2 TemporalAntiAliasingPass<APITag>::Get_CurrentPixelOffset(void) {
		switch (this->m_Jitter) {
		case TemporalAntiAliasingJitter::MSAA: {
			// This is the standard 8x MSAA sample pattern, source can be found e.g. here:
			// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
			constexpr Array<Math::VecF2, 8> offsets{
				Math::VecF2{  0.0625f, -0.1875f }, Math::VecF2{ -0.0625f,  0.1875f }, Math::VecF2{  0.3125f,  0.0625f }, Math::VecF2{ -0.1875f, -0.3125f },
				Math::VecF2{ -0.3125f,  0.3125f }, Math::VecF2{ -0.4375f, -0.0625f }, Math::VecF2{  0.1875f,  0.4375f }, Math::VecF2{  0.4375f, -0.4375f }
			};

			return offsets[this->m_FrameIndex % 8];
		}
		case TemporalAntiAliasingJitter::Halton: {
			Uint32 index{ (this->m_FrameIndex % 16) + 1 };
			return Math::VecF2{ VanDerCorput(2, index), VanDerCorput(3, index) } - 0.5f;
		}
		case TemporalAntiAliasingJitter::R2:
			return this->m_R2Jitter - 0.5f;
		case TemporalAntiAliasingJitter::WhiteNoise: {
			std::mt19937 rng{ this->m_FrameIndex };
			std::uniform_real_distribution<float> dist{ -0.5f, 0.5f };
			return Math::VecF2{ dist(rng), dist(rng) };
		}
		default:
			ASSERT(false);
			return Math::VecF2::Zero();
		}
	}

}