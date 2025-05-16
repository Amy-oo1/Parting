#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"


PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;
PARTING_IMPORT Algorithm;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Heap)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Sampler)
PARTING_SUBMODE_IMPORT(InputLayout)
PARTING_SUBMODE_IMPORT(Shader)
PARTING_SUBMODE_IMPORT(BlendState)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(RasterState)
PARTING_SUBMODE_IMPORT(DepthStencilState)
PARTING_SUBMODE_IMPORT(ViewportState)
PARTING_SUBMODE_IMPORT(FrameBuffer)
PARTING_SUBMODE_IMPORT(ShaderBinding)


#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Heap.h"
#include "RHI/Module/RHI-Buffer.h"
#include "RHI/Module/RHI-Texture.h"
#include "RHI/Module/RHI-Sampler.h"
#include "RHI/Module/RHI-InputLayout.h"
#include "RHI/Module/RHI-Shader.h"
#include "RHI/Module/RHI-BlendState.h"
#include "RHI/Module/RHI-RasterState.h"
#include "RHI/Module/RHI-DepthStencilState.h"
#include "RHI/Module/RHI-ViewportState.h"
#include "RHI/Module/RHI-FrameBuffer.h"
#include "RHI/Module/RHI-ShaderBinding.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {

	PARTING_EXPORT enum class RHIPrimitiveType : Uint8 {
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		TriangleListWithAdjacency,
		TriangleStripWithAdjacency,
		PatchList
	};

	PARTING_EXPORT struct RHISinglePassStereoState final {
		bool Enabled{ false };
		bool IndependentViewportMask{ false };
		Uint16 RenderTargetIndexOffset{ 0 };

		STDNODISCARD constexpr bool operator==(const RHISinglePassStereoState&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHISinglePassStereoState&) const noexcept = default;
	};

	PARTING_EXPORT struct RHIRenderState final {
		RHIBlendState BlendState;
		RHIDepthStencilState DepthStencilState;
		RHIRasterState RasterState;
		RHISinglePassStereoState SinglePassStereo;
	};

	PARTING_EXPORT enum class RHIVariableShadingRate : Uint8 {
		e1x1,
		e1x2,
		e2x1,
		e2x2,
		e2x4,
		e4x2,
		e4x4
	};

	PARTING_EXPORT enum class RHIShadingRateCombiner : Uint8 {
		Passthrough,
		Override,
		Min,
		Max,
		ApplyRelative
	};

	PARTING_EXPORT struct RHIVariableRateShadingState final {
		bool Enabled{ false };
		RHIVariableShadingRate ShadingRate{ RHIVariableShadingRate::e1x1 };
		RHIShadingRateCombiner PipelinePrimitiveCombiner{ RHIShadingRateCombiner::Passthrough };
		RHIShadingRateCombiner ImageCombiner{ RHIShadingRateCombiner::Passthrough };

		STDNODISCARD constexpr bool operator==(const RHIVariableRateShadingState&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIVariableRateShadingState&) const noexcept = default;
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIGraphicsPipelineDesc final {
		using Imp_InputLayout = typename RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_Shader = typename RHITypeTraits<APITag>::Imp_Shader;
		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;
		RHIPrimitiveType PrimType{ RHIPrimitiveType::TriangleList };
		Uint32 PatchControlPoints{ 0 };
		RefCountPtr<Imp_InputLayout> InputLayout{ nullptr };

		RefCountPtr<Imp_Shader> VS{ nullptr };
		RefCountPtr<Imp_Shader> HS{ nullptr };
		RefCountPtr<Imp_Shader> DS{ nullptr };
		RefCountPtr<Imp_Shader> GS{ nullptr };
		RefCountPtr<Imp_Shader> PS{ nullptr };

		RHIRenderState RenderState;
		RHIVariableRateShadingState ShadingRateState;

		Array<RefCountPtr<Imp_BindingLayout>, g_MaxBindingLayoutCount> BindingLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type BindingLayoutCount{ 0 };

	};

	PARTING_EXPORT template<APITagConcept APITag>
		class RHIGraphicsPipelineDescBuilder final {
		using Imp_InputLayout = typename RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_Shader = typename RHITypeTraits<APITag>::Imp_Shader;
		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;
		public:
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Reset(void) { this->m_Desc = RHIGraphicsPipelineDesc{}; return *this; }

			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_PrimType(RHIPrimitiveType primType) { this->m_Desc.PrimType = primType; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_PatchControlPoints(Uint32 patchControlPoints) { this->m_Desc.PatchControlPoints = patchControlPoints; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_InputLayout(Imp_InputLayout* inputLayout) { this->m_Desc.InputLayout = inputLayout; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_VS(Imp_Shader* shader) { this->m_Desc.VS = shader; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_HS(Imp_Shader* shader) { this->m_Desc.HS = shader; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_DS(Imp_Shader* shader) { this->m_Desc.DS = shader; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_GS(Imp_Shader* shader) { this->m_Desc.GS = shader; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_PS(Imp_Shader* shader) { this->m_Desc.PS = shader; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_RenderState(const RHIRenderState& renderState) { this->m_Desc.RenderState = renderState; return *this; }
			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& Set_ShadingRateState(const RHIVariableRateShadingState& shadingRateState) { this->m_Desc.ShadingRateState = shadingRateState; return *this; }

			STDNODISCARD constexpr RHIGraphicsPipelineDescBuilder& AddBindingLayout(Imp_BindingLayout* bindingLayout) { this->m_Desc.BindingLayouts[this->m_Desc.BindingLayoutCount++] = bindingLayout; return *this; }

			STDNODISCARD constexpr const RHIGraphicsPipelineDesc<APITag>& Build(void) { return this->m_Desc; }
		private:
			RHIGraphicsPipelineDesc<APITag> m_Desc{};
	};

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
		class RHIGraphicsPipeline :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHIGraphicsPipeline(void) = default;
			virtual ~RHIGraphicsPipeline(void) = default;

		public:
			STDNODISCARD const RHIGraphicsPipelineDesc<APITag>& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }
			STDNODISCARD const RHIFrameBufferInfo<APITag>& Get_FrameBufferInfo(void)const { return this->Get_Derived()->Imp_Get_FrameBufferInfo(); }

		private:
			STDNODISCARD constexpr Derived* Get_Derived(void)const noexcept { return static_cast<Derived*>(this); }
		private:
			const RHIGraphicsPipelineDesc<APITag>& Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return RHIGraphicsPipelineDesc<APITag>{}; }
			const RHIFrameBufferInfo<APITag>& Imp_Get_FrameBufferInfo(void)const { LOG_ERROR("No Imp"); return RHIFrameBufferInfo<APITag>{}; }
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIComputePipelineDesc final {
		using Imp_Shader = typename RHITypeTraits<APITag>::Imp_Shader;
		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;

		RefCountPtr<Imp_Shader> CS;

		Array<RefCountPtr<Imp_BindingLayout>, g_MaxBindingLayoutCount> BindingLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type BindingLayoutCount{ 0 };
	};

	template<APITagConcept APITag>
	struct RHIComputePipelineDescBuilder final {
		using Imp_Shader = typename RHITypeTraits<APITag>::Imp_Shader;
		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;
	public:
		constexpr RHIComputePipelineDescBuilder& Reset(void) { this->m_Desc = RHIComputePipelineDesc{}; return *this; }

		constexpr RHIComputePipelineDescBuilder& Set_CS(RefCountPtr<Imp_Shader> shader) { this->m_Desc.CS = shader; return *this; }

		constexpr RHIComputePipelineDescBuilder& AddBindingLayout(RefCountPtr<Imp_BindingLayout> bindingLayout) { this->m_Desc.BindingLayouts[this->m_Desc.BindingLayoutCount++] = bindingLayout; return *this; }

		STDNODISCARD constexpr const RHIComputePipelineDesc<APITag>& Build(void) { return this->m_Desc; }

	private:
		RHIComputePipelineDesc<APITag> m_Desc{};
	};

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
		class RHIComputePipeline :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHIComputePipeline(void) = default;
			virtual ~RHIComputePipeline(void) = default;

		public:
			STDNODISCARD const RHIComputePipelineDesc<APITag>& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }

		private:
			STDNODISCARD constexpr Derived* Get_Derived(void)const noexcept { return static_cast<Derived*>(this); }
		private:
			const RHIComputePipelineDesc<APITag>& Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return RHIComputePipelineDesc<APITag>{}; }
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIMeshletPipelineDesc final {
		using Imp_Shader = typename RHITypeTraits<APITag>::Imp_Shader;
		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;

		RHIPrimitiveType PrimType{ RHIPrimitiveType::TriangleList };

		Imp_Shader* AS{ nullptr };
		Imp_Shader* MS{ nullptr };
		Imp_Shader* PS{ nullptr };

		RHIRenderState RenderState;

		Array<Imp_BindingLayout*, g_MaxBindingLayoutCount> BindingLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type BindingLayoutCount{ 0 };
	};


	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
		class RHIMeshletPipeline :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHIMeshletPipeline(void) = default;
			PARTING_VIRTUAL ~RHIMeshletPipeline(void) = default;

		public:
			STDNODISCARD const RHIMeshletPipelineDesc<APITag>& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }
			STDNODISCARD const RHIFrameBufferInfo<APITag>& Get_FrameBufferInfo(void)const { return this->Get_Derived()->Imp_Get_FrameBufferInfo(); }

		private:
			STDNODISCARD constexpr Derived* Get_Derived(void)const noexcept { return static_cast<Derived*>(this); }
		private:
			const RHIMeshletPipelineDesc<APITag>& Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return RHIMeshletPipelineDesc<APITag>{}; }
			const RHIFrameBufferInfo<APITag>& Imp_Get_FrameBufferInfo(void)const { LOG_ERROR("No Imp"); return RHIFrameBufferInfo<APITag>{}; }
	};

}