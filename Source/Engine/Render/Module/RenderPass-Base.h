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


#include "Engine/Render/Module/SceneTypes.h"
#include "Engine/Engine/Module/SceneGraph.h"

#include "Engine/Render/Module/DrawStrategy.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"
#include "Engine/Render/Module/View.h"

#include "Engine/Render/Module/GBuffer.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {




	class GeometryPassContext {};

	template<RHI::APITagConcept APITag>
	class IGeometryPass {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
	public:
		IGeometryPass(void) = default;
		virtual ~IGeometryPass(void) = default;

	public:
		static void RenderView(
			Imp_CommandList* commandList,
			const IView* view,
			const IView* viewPrev,
			Imp_FrameBuffer* framebuffer,
			IDrawStrategy<APITag>& drawStrategy,
			IGeometryPass<APITag>& pass,
			GeometryPassContext& passContext,
			bool materialEvents = false
		);


		static void RenderCompositeView(
			Imp_CommandList* commandList,
			const ICompositeView* compositeView,
			const ICompositeView* compositeViewPrev,
			FrameBufferFactory<APITag>& framebufferFactory,
			const SharedPtr<SceneGraphNode<APITag>>& rootNode,
			IDrawStrategy<APITag>& drawStrategy,
			IGeometryPass& pass,
			GeometryPassContext& passContext,
			const char* passEvent = nullptr,
			bool materialEvents = false
		);


	public:
		STDNODISCARD virtual ViewType Get_SupportedViewTypes(void) const = 0;
		virtual void SetupView(GeometryPassContext& context, Imp_CommandList* commandList, const IView* view, const IView* viewPrev) = 0;
		virtual bool SetupMaterial(GeometryPassContext& context, const Material<APITag>* material, RHI::RHIRasterCullMode cullMode, RHI::RHIGraphicsState<APITag>& state) = 0;
		virtual void SetupInputBuffers(GeometryPassContext& context, const BufferGroup<APITag>* buffers, RHI::RHIGraphicsState<APITag>& state) = 0;
		virtual void SetPushConstants(GeometryPassContext& context, Imp_CommandList* commandList, RHI::RHIGraphicsState<APITag>& state, RHI::RHIDrawArguments& args) = 0;
	};





	template<RHI::APITagConcept APITag>
	inline void IGeometryPass<APITag>::RenderView(Imp_CommandList* commandList, const IView* view, const IView* viewPrev, Imp_FrameBuffer* framebuffer, IDrawStrategy<APITag>& drawStrategy, IGeometryPass<APITag>& pass, GeometryPassContext& passContext, bool materialEvents) {
		pass.SetupView(passContext, commandList, view, viewPrev);

		const Material<APITag>* lastMaterial{ nullptr };
		const BufferGroup<APITag>* lastBuffers{ nullptr };
		auto lastCullMode{ RHI::RHIRasterCullMode::Back };

		bool drawMaterial{ true };
		bool stateValid{ false };

		const Material<APITag>* eventMaterial{ nullptr };

		RHI::RHIGraphicsState<APITag> graphicsState;
		graphicsState.FrameBuffer = framebuffer;
		graphicsState.Viewport = view->Get_ViewportState();
		graphicsState.ShadingRateState = view->Get_VariableRateShadingState();

		RHI::RHIDrawArguments currentDraw{ .InstanceCount{ 0 } };

		auto flushDraw = [commandList, materialEvents, &graphicsState, &currentDraw, &eventMaterial, &pass, &passContext](const Material<APITag>* material) {
			if (currentDraw.InstanceCount == 0)
				return;

			if (materialEvents && material != eventMaterial) {
				if (eventMaterial)
					commandList->EndMarker();

				if (material->Name.empty())
					eventMaterial = nullptr;
				else {
					commandList->BeginMarker(material->Name.c_str());
					eventMaterial = material;
				}
			}

			pass.SetPushConstants(passContext, commandList, graphicsState, currentDraw);

			commandList->DrawIndexed(currentDraw);
			currentDraw.InstanceCount = 0;
			};

		while (const DrawItem<APITag>* item = drawStrategy.Get_NextItem()) {
			if (nullptr == item->Material)
				continue;

			bool newBuffers{ item->Buffers != lastBuffers };
			bool newMaterial{ item->Material != lastMaterial || item->CullMode != lastCullMode };

			if (newBuffers || newMaterial)
				flushDraw(lastMaterial);

			if (newBuffers) {
				pass.SetupInputBuffers(passContext, item->Buffers, graphicsState);

				lastBuffers = item->Buffers;
				stateValid = false;
			}

			if (newMaterial) {
				drawMaterial = pass.SetupMaterial(passContext, item->Material, item->CullMode, graphicsState);

				lastMaterial = item->Material;
				lastCullMode = item->CullMode;
				stateValid = false;
			}

			if (drawMaterial) {
				if (!stateValid) {
					commandList->SetGraphicsState(graphicsState);
					stateValid = true;
				}

				RHI::RHIDrawArguments args{
					.VertexCount{ item->Geometry->NumIndices },
					.InstanceCount{ 1 },
					.StartIndexLocation{ item->Mesh->IndexOffset + item->Geometry->IndexOffsetInMesh },
					.StartVertexLocation{ item->Mesh->VertexOffset + item->Geometry->VertexOffsetInMesh },
					.StartInstanceLocation{ static_cast<Uint32>(item->Instance->Get_InstanceIndex()) },/*TODO*/
				};

				if (currentDraw.InstanceCount > 0 &&
					currentDraw.StartIndexLocation == args.StartIndexLocation &&
					currentDraw.StartInstanceLocation + currentDraw.InstanceCount == args.StartInstanceLocation) {
					currentDraw.InstanceCount += 1;
				}
				else {
					flushDraw(item->Material);

					currentDraw = args;
				}
			}
		}

		flushDraw(lastMaterial);

		if (materialEvents && nullptr != eventMaterial)
			commandList->EndMarker();

	}

	template<RHI::APITagConcept APITag>
	inline void IGeometryPass<APITag>::RenderCompositeView(Imp_CommandList* commandList, const ICompositeView* compositeView, const ICompositeView* compositeViewPrev, FrameBufferFactory<APITag>& framebufferFactory, const SharedPtr<SceneGraphNode<APITag>>& rootNode, IDrawStrategy<APITag>& drawStrategy, IGeometryPass& pass, GeometryPassContext& passContext, const char* passEvent, bool materialEvents) {
		if (nullptr != passEvent)
			commandList->BeginMarker(passEvent);

		ViewType supportedViewTypes{ pass.Get_SupportedViewTypes() };

		if (nullptr != compositeViewPrev)// the views must have the same topology	
			ASSERT(compositeView->Get_NumChildViews(supportedViewTypes) == compositeViewPrev->Get_NumChildViews(supportedViewTypes));

		for (Uint32 viewIndex = 0; viewIndex < compositeView->Get_NumChildViews(supportedViewTypes); ++viewIndex)
		{
			const IView* view{ compositeView->Get_ChildView(supportedViewTypes, viewIndex) };
			const IView* viewPrev{ compositeViewPrev ? compositeViewPrev->Get_ChildView(supportedViewTypes, viewIndex) : nullptr };

			ASSERT(view != nullptr);

			drawStrategy.PrepareForView(rootNode, *view);

			auto framebuffer{ framebufferFactory.Get_FrameBuffer(*view) };

			IGeometryPass<APITag>::RenderView(commandList, view, viewPrev, framebuffer, drawStrategy, pass, passContext, materialEvents);
		}

		if (nullptr != passEvent)
			commandList->EndMarker();
	}

}

