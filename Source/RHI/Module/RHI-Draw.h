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

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(BlendState)
PARTING_SUBMODE_IMPORT(Heap)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(InputLayout)
PARTING_SUBMODE_IMPORT(Shader)
PARTING_SUBMODE_IMPORT(RasterState)
PARTING_SUBMODE_IMPORT(DepthStencilState)
PARTING_SUBMODE_IMPORT(ViewportState)
PARTING_SUBMODE_IMPORT(FrameBuffer)
PARTING_SUBMODE_IMPORT(Sampler)
PARTING_SUBMODE_IMPORT(ShaderBinding)
PARTING_SUBMODE_IMPORT(Pipeline)


#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Container/Module/Container.h"
#include "Core/Logger/Module/Logger.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-BlendState.h"
#include "RHI/Module/RHI-Heap.h"
#include "RHI/Module/RHI-Texture.h"
#include "RHI/Module/RHI-Buffer.h"
#include "RHI/Module/RHI-InputLayout.h"
#include "RHI/Module/RHI-Shader.h"
#include "RHI/Module/RHI-RasterState.h"
#include "RHI/Module/RHI-DepthStencilState.h"
#include "RHI/Module/RHI-ViewportState.h"
#include "RHI/Module/RHI-FrameBuffer.h"
#include "RHI/Module/RHI-Sampler.h"

#include "RHI/Module/RHI-ShaderBinding.h"
#include "RHI/Module/RHI-Pipeline.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace RHI {

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIVertexBufferBinding final {
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;

		Imp_Buffer* Buffer{ nullptr };
		Uint32 Slot{ 0 };
		Uint64 Offset{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIVertexBufferBinding& other) const noexcept {
			return
				this->Buffer == other.Buffer &&
				this->Slot == other.Slot &&
				this->Offset == other.Offset;
		};
		STDNODISCARD constexpr bool operator!=(const RHIVertexBufferBinding& other) const noexcept { return !(*this == other); }
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIIndexBufferBinding final {
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;

		Imp_Buffer* Buffer{ nullptr };
		RHIFormat Format{ RHIFormat::UNKNOWN };
		Uint32 Offset{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIIndexBufferBinding&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIIndexBufferBinding&) const noexcept = default;
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIGraphicsState final {
		using Imp_GraphicsPipeline = typename RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_FrameBuffer = typename RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;

		Imp_GraphicsPipeline* Pipeline{ nullptr };
		Imp_FrameBuffer* FrameBuffer{ nullptr };
		RHIViewportState Viewport;
		RHIVariableRateShadingState ShadingRateState;
		Color BlendConstantColor;
		Uint8 DynamicStencilRefValue{ 0 };

		Array<Imp_BindingSet*, g_MaxBindingLayoutCount> BindingSets{};
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type BindingSetCount{ 0 };

		Array<RHIVertexBufferBinding<APITag>, g_MaxVertexAttributeCount> VertexBuffers{};
		RemoveCV<decltype(g_MaxVertexAttributeCount)>::type VertexBufferCount{ 0 };

		RHIIndexBufferBinding<APITag> IndexBuffer{};

		Imp_Buffer* IndirectParams{ nullptr };
	};

	PARTING_EXPORT template<APITagConcept APITag>
		class RHIGraphicsStateBuilder final {
		using Imp_GraphicsPipeline = typename RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_FrameBuffer = typename RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;
		public:
			constexpr RHIGraphicsStateBuilder& Reset(void) { this->m_State = RHIGraphicsState<APITag>{}; return *this; }

			constexpr RHIGraphicsStateBuilder& Set_Pipeline(Imp_GraphicsPipeline* pipeline) { this->m_State.Pipeline = pipeline; return *this; }
			constexpr RHIGraphicsStateBuilder& Set_FrameBuffer(Imp_FrameBuffer* framebuffer) { this->m_State.FrameBuffer = framebuffer; return *this; }
			constexpr RHIGraphicsStateBuilder& Set_ViewportState(const RHIViewportState& viewport) { this->m_State.Viewport = viewport; return *this; }
			constexpr RHIGraphicsStateBuilder& Set_ShadingRateState(const RHIVariableRateShadingState& shadingRateState) { this->m_State.ShadingRateState = shadingRateState; return *this; }
			constexpr RHIGraphicsStateBuilder& Set_BlendConstantColor(const Color& color) { this->m_State.BlendConstantColor = color; return *this; }
			constexpr RHIGraphicsStateBuilder& Set_DynamicStencilRefValue(Uint8 stencilRefValue) { this->m_State.DynamicStencilRefValue = stencilRefValue; return *this; }
			constexpr RHIGraphicsStateBuilder& Set_IndexBuffer(const RHIIndexBufferBinding<APITag>& indexBuffer) { this->m_State.IndexBuffer = indexBuffer; return *this; }
			constexpr RHIGraphicsStateBuilder& Set_IndirectParams(Imp_Buffer* indirectParams) { this->m_State.IndirectParams = indirectParams; return *this; }

			constexpr RHIGraphicsStateBuilder& AddViewport(const RHIViewport& viewport) { this->m_State.Viewport.Viewports[this->m_State.Viewport.ViewportCount++] = viewport; return *this; }
			constexpr RHIGraphicsStateBuilder& AddScissorRect(const RHIRect2D& scissor) { this->m_State.Viewport.ScissorRects[this->m_State.Viewport.ScissorCount++] = scissor; return *this; }
			constexpr RHIGraphicsStateBuilder& AddViewportAndScissorRect(const RHIViewport& viewport) {
				this->AddViewport(viewport);
				this->AddScissorRect(BuildScissorRect(viewport));

				return *this;
			}

			constexpr RHIGraphicsStateBuilder& SubScissorRect(void) { --this->m_State.Viewport.ScissorCount; return *this; }
			constexpr RHIGraphicsStateBuilder& SubViewport(void) { --this->m_State.Viewport.ViewportCount; return *this; }

			constexpr RHIGraphicsStateBuilder& AddBindingSet(Imp_BindingSet* bindingSet) { this->m_State.BindingSets[this->m_State.BindingSetCount++] = bindingSet; return *this; }
			constexpr RHIGraphicsStateBuilder& SubBindingSet(void) { --this->m_State.BindingSetCount; return *this; }

			constexpr RHIGraphicsStateBuilder& AddVertexBuffer(const RHIVertexBufferBinding<APITag>& vertexBuffer) { this->m_State.VertexBuffers[this->m_State.VertexBufferCount++] = vertexBuffer; return *this; }
			constexpr RHIGraphicsStateBuilder& SubVertexBuffer(void) { --this->m_State.VertexBufferCount; return *this; }

			STDNODISCARD constexpr const RHIGraphicsState<APITag>& Build(void) { return this->m_State; }
		private:
			RHIGraphicsState<APITag> m_State{};
	};

	PARTING_EXPORT struct RHIDrawArguments final {
		Uint32 VertexCount{ 0 };
		Uint32 InstanceCount{ 1 };
		Uint32 StartIndexLocation{ 0 };
		Uint32 StartVertexLocation{ 0 };
		Uint32 StartInstanceLocation{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIDrawArguments&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIDrawArguments&) const noexcept = default;
	};

	PARTING_EXPORT class RHIDrawArgumentsBuilder final {
	public:
		STDNODISCARD constexpr RHIDrawArgumentsBuilder& Reset(void) { this->m_Desc = RHIDrawArguments{}; return *this; }

		STDNODISCARD constexpr RHIDrawArgumentsBuilder& Set_VertexCount(Uint32 vertexCount) { this->m_Desc.VertexCount = vertexCount; return *this; }
		STDNODISCARD constexpr RHIDrawArgumentsBuilder& Set_InstanceCount(Uint32 instanceCount) { this->m_Desc.InstanceCount = instanceCount; return *this; }
		STDNODISCARD constexpr RHIDrawArgumentsBuilder& Set_StartIndexLocation(Uint32 startIndexLocation) { this->m_Desc.StartIndexLocation = startIndexLocation; return *this; }
		STDNODISCARD constexpr RHIDrawArgumentsBuilder& Set_StartVertexLocation(Uint32 startVertexLocation) { this->m_Desc.StartVertexLocation = startVertexLocation; return *this; }
		STDNODISCARD constexpr RHIDrawArgumentsBuilder& Set_StartInstanceLocation(Uint32 startInstanceLocation) { this->m_Desc.StartInstanceLocation = startInstanceLocation; return *this; }

		STDNODISCARD constexpr const RHIDrawArguments& Build(void) { return this->m_Desc; }
	private:
		RHIDrawArguments m_Desc{};
	};

	PARTING_EXPORT struct RHIDrawIndirectArguments final {
		Uint32 VertexCount{ 0 };
		Uint32 InstanceCount{ 1 };
		Uint32 StartVertexLocation{ 0 };
		Uint32 StartInstanceLocation{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIDrawIndirectArguments& other) const noexcept {
			return
				this->VertexCount == other.VertexCount &&
				this->InstanceCount == other.InstanceCount &&
				this->StartVertexLocation == other.StartVertexLocation &&
				this->StartInstanceLocation == other.StartInstanceLocation;
		};
		STDNODISCARD constexpr bool operator!=(const RHIDrawIndirectArguments& other) const noexcept { return !(*this == other); }
	};

	//NOTE :this class just for see ,dout use ,becase bebuild class has just 4 member
	PARTING_EXPORT class RHIDrawIndirectArgumentsBuilder final {
	public:
		STDNODISCARD constexpr RHIDrawIndirectArgumentsBuilder& Reset(void) { this->m_Desc = RHIDrawIndirectArguments{}; return *this; }

		STDNODISCARD constexpr RHIDrawIndirectArgumentsBuilder& Set_VertexCount(Uint32 vertexCount) { this->m_Desc.VertexCount = vertexCount; return *this; }
		STDNODISCARD constexpr RHIDrawIndirectArgumentsBuilder& Set_InstanceCount(Uint32 instanceCount) { this->m_Desc.InstanceCount = instanceCount; return *this; }
		STDNODISCARD constexpr RHIDrawIndirectArgumentsBuilder& Set_StartVertexLocation(Uint32 startVertexLocation) { this->m_Desc.StartVertexLocation = startVertexLocation; return *this; }
		STDNODISCARD constexpr RHIDrawIndirectArgumentsBuilder& Set_StartInstanceLocation(Uint32 startInstanceLocation) { this->m_Desc.StartInstanceLocation = startInstanceLocation; return *this; }

		STDNODISCARD constexpr const RHIDrawIndirectArguments& Build(void) { return this->m_Desc; }
	private:
		RHIDrawIndirectArguments m_Desc{};
	};

	PARTING_EXPORT struct RHIDrawIndexedIndirectArguments final {
		Uint32 IndexCount{ 0 };
		Uint32 InstanceCount{ 1 };
		Uint32 StartIndexLocation{ 0 };
		Int32  BaseVertexLocation{ 0 };//NOTE Int32
		Uint32 StartInstanceLocation{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIDrawIndexedIndirectArguments&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIDrawIndexedIndirectArguments&) const noexcept = default;
	};

	PARTING_EXPORT class RHIDrawIndexedIndirectArgumentsBuilder final {
	public:
		STDNODISCARD constexpr RHIDrawIndexedIndirectArgumentsBuilder& Reset(void) { this->m_Desc = RHIDrawIndexedIndirectArguments{}; return *this; }

		STDNODISCARD constexpr RHIDrawIndexedIndirectArgumentsBuilder& Set_IndexCount(Uint32 indexCount) { this->m_Desc.IndexCount = indexCount; return *this; }
		STDNODISCARD constexpr RHIDrawIndexedIndirectArgumentsBuilder& Set_InstanceCount(Uint32 instanceCount) { this->m_Desc.InstanceCount = instanceCount; return *this; }
		STDNODISCARD constexpr RHIDrawIndexedIndirectArgumentsBuilder& Set_StartIndexLocation(Uint32 startIndexLocation) { this->m_Desc.StartIndexLocation = startIndexLocation; return *this; }
		STDNODISCARD constexpr RHIDrawIndexedIndirectArgumentsBuilder& Set_BaseVertexLocation(Int32 baseVertexLocation) { this->m_Desc.BaseVertexLocation = baseVertexLocation; return *this; }
		STDNODISCARD constexpr RHIDrawIndexedIndirectArgumentsBuilder& Set_StartInstanceLocation(Uint32 startInstanceLocation) { this->m_Desc.StartInstanceLocation = startInstanceLocation; return *this; }

		STDNODISCARD constexpr const RHIDrawIndexedIndirectArguments& Build(void) { return this->m_Desc; }
	private:
		RHIDrawIndexedIndirectArguments m_Desc{};
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIComputeState final {
		using Imp_ComputePipeline = typename RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;

		Imp_ComputePipeline* Pipeline{ nullptr };

		Array<Imp_BindingSet*, g_MaxBindingLayoutCount> BindingSets;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type BindingSetCount{ 0 };

		Imp_Buffer* IndirectParams{ nullptr };

		STDNODISCARD constexpr bool operator==(const RHIComputeState&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIComputeState&) const noexcept = default;//NOTE :Complie not set NODISCARD
	};

	PARTING_EXPORT template<APITagConcept APITag>
		class RHIComputeStateBuilder final {
		using Imp_ComputePipeline = typename RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;
		public:
			constexpr RHIComputeStateBuilder& Reset(void) { this->m_State = RHIComputeState<APITag>{}; return *this; }

			constexpr RHIComputeStateBuilder& Set_Pipeline(Imp_ComputePipeline* pipeline) { this->m_State.Pipeline = pipeline; return *this; }
			constexpr RHIComputeStateBuilder& Set_IndirectParams(Imp_Buffer* indirectParams) { this->m_State.IndirectParams = indirectParams; return *this; }

			constexpr RHIComputeStateBuilder& AddBindingSet(Imp_BindingSet* bindingSet) { this->m_State.BindingSets[this->m_State.BindingSetCount++] = bindingSet; return *this; }

			STDNODISCARD constexpr const RHIComputeState<APITag>& Build(void) { return this->m_State; }
		private:
			RHIComputeState<APITag> m_State{};
	};

	PARTING_EXPORT struct RHIDispatchIndirectArguments final {
		Uint32 GroupsX{ 0 };
		Uint32 GroupsY{ 0 };
		Uint32 GroupsZ{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIDispatchIndirectArguments&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIDispatchIndirectArguments&) const noexcept = default;
	};

	PARTING_EXPORT class RHIDispatchIndirectArgumentsBuilder final {
	public:
		STDNODISCARD constexpr RHIDispatchIndirectArgumentsBuilder& Reset(void) { this->m_Desc = RHIDispatchIndirectArguments{}; return *this; }

		STDNODISCARD constexpr RHIDispatchIndirectArgumentsBuilder& Set_GroupsX(Uint32 groupsX) { this->m_Desc.GroupsX = groupsX; return *this; }
		STDNODISCARD constexpr RHIDispatchIndirectArgumentsBuilder& Set_GroupsY(Uint32 groupsY) { this->m_Desc.GroupsY = groupsY; return *this; }
		STDNODISCARD constexpr RHIDispatchIndirectArgumentsBuilder& Set_GroupsZ(Uint32 groupsZ) { this->m_Desc.GroupsZ = groupsZ; return *this; }

		STDNODISCARD constexpr RHIDispatchIndirectArgumentsBuilder& Set_Groups2D(Uint32 groupsX, Uint32 groupsY) { this->m_Desc.GroupsX = groupsX; this->m_Desc.GroupsY = groupsY; return *this; }
		STDNODISCARD constexpr RHIDispatchIndirectArgumentsBuilder& Set_Groups3D(Uint32 groupsX, Uint32 groupsY, Uint32 groupsZ) { this->m_Desc.GroupsX = groupsX; this->m_Desc.GroupsY = groupsY; this->m_Desc.GroupsZ = groupsZ; return *this; }

		STDNODISCARD constexpr const RHIDispatchIndirectArguments& Build(void) { return this->m_Desc; }
	private:
		RHIDispatchIndirectArguments m_Desc{};
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIMeshletState final {
		using Imp_MeshletPipeline = typename RHITypeTraits<APITag>::Imp_MeshletPipeline;
		using Imp_FrameBuffer = typename RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;

		Imp_MeshletPipeline* Pipeline{ nullptr };
		Imp_FrameBuffer* FrameBuffer{ nullptr };
		RHIViewportState Viewport;
		Color BlendConstantColor{};
		Uint8 DynamicStencilRefValue{ 0 };

		Array<Imp_BindingSet*, g_MaxBindingLayoutCount> BindingSets;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type BindingSetCount{ 0 };

		Imp_Buffer* IndirectParams{ nullptr };

		STDNODISCARD constexpr bool operator==(const RHIMeshletState&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIMeshletState&) const noexcept = default;
	};

	PARTING_EXPORT template<APITagConcept APITag>
		class RHIMeshletStateBuilder final {
		using Imp_MeshletPipeline = typename RHITypeTraits<APITag>::Imp_MeshletPipeline;
		using Imp_FrameBuffer = typename RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;
		public:
			constexpr RHIMeshletStateBuilder& Reset(void) { this->m_State = RHIMeshletState<APITag>{}; return *this; }

			constexpr RHIMeshletStateBuilder& Set_Pipeline(Imp_MeshletPipeline* pipeline) { this->m_State.Pipeline = pipeline; return *this; }
			constexpr RHIMeshletStateBuilder& Set_FrameBuffer(Imp_FrameBuffer* framebuffer) { this->m_State.FrameBuffer = framebuffer; return *this; }
			constexpr RHIMeshletStateBuilder& Set_ViewportState(const RHIViewportState& viewport) { this->m_State.Viewport = viewport; return *this; }
			constexpr RHIMeshletStateBuilder& Set_BlendConstantColor(const Color& color) { this->m_State.BlendConstantColor = color; return *this; }
			constexpr RHIMeshletStateBuilder& Set_DynamicStencilRefValue(Uint8 stencilRefValue) { this->m_State.DynamicStencilRefValue = stencilRefValue; return *this; }
			constexpr RHIMeshletStateBuilder& Set_IndirectParams(Imp_Buffer* indirectParams) { this->m_State.IndirectParams = indirectParams; return *this; }

			STDNODISCARD constexpr RHIMeshletStateBuilder& AddBindingSet(Imp_BindingSet* bindingSet) { this->m_State.BindingSets[this->m_State.BindingSetCount++] = bindingSet; return *this; }

			STDNODISCARD constexpr const RHIMeshletState<APITag>& Build(void) { return this->m_State; }
		private:
			RHIMeshletState<APITag> m_State{};
	};

}