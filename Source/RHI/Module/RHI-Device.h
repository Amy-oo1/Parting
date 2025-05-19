#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"


PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(RHI, Device)

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
PARTING_SUBMODE_IMPORT(Draw)
PARTING_SUBMODE_IMPORT(CommandList)


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
#include "RHI/Module/RHI-Draw.h"
#include "RHI/Module/RHI-CommandList.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace RHI {

	/*PARTING_EXPORT*/ template<typename Derived, APITagConcept APITag>
	class RHIDevice :public RHIResource<Derived> {
		friend class RHIResource<Derived>;

		using Imp_MessageCallback = typename RHITypeTraits<APITag>::Imp_MessageCallback;
		using Imp_Heap = typename RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;
		using Imp_StagingTexture = typename RHITypeTraits<APITag>::Imp_StagingTexture;
		using Imp_Sampler = typename RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_SamplerFeedbackTexture = typename RHITypeTraits<APITag>::Imp_SamplerFeedbackTexture;
		using Imp_FrameBuffer = typename RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_TimerQuery = typename RHITypeTraits<APITag>::Imp_TimerQuery;
		using Imp_EventQuery = typename RHITypeTraits<APITag>::Imp_EventQuery;
		using Imp_Shader = typename RHITypeTraits<APITag>::Imp_Shader;
		using Imp_ShaderLibrary = typename RHITypeTraits<APITag>::Imp_ShaderLibrary;
		using Imp_InputLayout = typename RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_GraphicsPipeline = typename RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_ComputePipeline = typename RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_MeshletPipeline = typename RHITypeTraits<APITag>::Imp_MeshletPipeline;
		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		//TODO :using Imp_DescriptorSet = typename RHITypeTraits<APITag>::Imp_DescriptorSet;
		using Imp_CommandList = typename RHITypeTraits<APITag>::Imp_CommandList;

	protected:
		RHIDevice(void) = default;
		PARTING_VIRTUAL ~RHIDevice(void) = default;


	public:
		RHIFormat ChooseFormat(RHIFormatSupport requiredFeatures, const RHIFormat* requestedFormats, Uint32 requestedFormatCount) {
			ASSERT(requestedFormats != nullptr);
			ASSERT(requestedFormatCount > 0);

			for (Uint32 Index = 0; Index < requestedFormatCount; ++Index)
				if (requiredFeatures == (this->QueryFormatSupport(requestedFormats[Index]) & requiredFeatures))
					return requestedFormats[Index];

			return RHIFormat::UNKNOWN;
		}

	public:
		STDNODISCARD RefCountPtr<Imp_Heap> CreateHeap(const RHIHeapDesc& desc) { return this->Get_Derived()->Imp_CreateHeap(desc); }

		STDNODISCARD RefCountPtr<Imp_Texture> CreateTexture(const RHITextureDesc& desc) { return this->Get_Derived()->Imp_CreateTexture(desc); }

		STDNODISCARD RHIMemoryRequirements Get_TextureMemoryRequirements(Imp_Texture* texture) { return this->Get_Derived()->Imp_Get_TextureMemoryRequirements(texture); }

		bool BindTextureMemory(Imp_Texture* texture, Imp_Heap* heap, Uint64 offset) { return this->Get_Derived()->Imp_BindTextureMemory(texture, heap, offset); }

		STDNODISCARD RefCountPtr<Imp_Texture> CreateHandleForNativeTexture(RHIObjectType type, RHIObject texture, const RHITextureDesc& desc) { return this->Get_Derived()->Imp_CreateHandleForNativeTexture(type, texture, desc); }

		STDNODISCARD RefCountPtr<Imp_StagingTexture> CreateStagingTexture(const RHITextureDesc& desc, RHICPUAccessMode CPUaccess) { return this->Get_Derived()->Imp_CreateStagingTexture(desc, CPUaccess); }

		STDNODISCARD void* MapStagingTexture(Imp_StagingTexture* texture, const RHITextureSlice& slice, RHICPUAccessMode CPUaccess, Uint64* OutRowPitch) { return this->Get_Derived()->Imp_MapStagingTexture(texture, slice, CPUaccess, OutRowPitch); }

		void UnmapStagingTexture(Imp_StagingTexture* texture) { this->Get_Derived()->Imp_UnmapStagingTexture(texture); }

		//TODO us re to set output
		void Get_TextureTiling(Imp_Texture* texture, Uint32* TileCount, RHIPackedMipDesc* desc, RHITileShape* tileshadpe, Uint32* SubresourceTilingCount, RHISubresourceTiling* SubresourceTilings) { this->Get_Derived()->Imp_Get_TextureTiling(texture, TileCount, desc, tileshadpe, SubresourceTilingCount, SubresourceTilings); }

		void UpdateTextureTileMappings(Imp_Texture* texture, const RHITextureTilesMapping<APITag>* TileMappings, Uint32 TileMappingCount, RHICommandQueue executionQueue = RHICommandQueue::Graphics) { this->Get_Derived()->Imp_UpdateTextureTileMappings(texture, TileMappings, TileMappingCount, executionQueue); }

		STDNODISCARD RefCountPtr<Imp_SamplerFeedbackTexture> CreateSamplerFeedbackTexture(Imp_Texture* texture, const RHISamplerFeedbackTextureDesc& desc) { return this->Get_Derived()->Imp_CreateSamplerFeedbackTexture(texture, desc); }

		STDNODISCARD RefCountPtr<Imp_SamplerFeedbackTexture> CreateSamplerFeedbackForNativeTexture(RHIObjectType type, RHIObject texture, Imp_Texture* pairedtexture) { return this->Get_Derived()->Imp_CreateSamplerFeedbackForNativeTexture(type, texture, pairedtexture); }

		STDNODISCARD RefCountPtr<Imp_Buffer> CreateBuffer(const RHIBufferDesc& desc) { return this->Get_Derived()->Imp_CreateBuffer(desc); }

		STDNODISCARD void* MapBuffer(Imp_Buffer* buffer, RHICPUAccessMode CPUaccess) { return this->Get_Derived()->Imp_MapBuffer(buffer, CPUaccess); }

		void UnmapBuffer(Imp_Buffer* buffer) { this->Get_Derived()->Imp_UnmapBuffer(buffer); }

		STDNODISCARD RHIMemoryRequirements Get_BufferMemoryRequirements(Imp_Buffer* buffer) { return this->Get_Derived()->Imp_Get_BufferMemoryRequirements(buffer); }

		bool BindBufferMemory(Imp_Buffer* buffer, Imp_Heap* heap, Uint64 offset) { return this->Get_Derived()->Imp_BindBufferMemory(buffer, heap, offset); }

		STDNODISCARD RefCountPtr<Imp_Buffer> CreateHandleForNativeBuffer(RHIObjectType type, RHIObject buffer, const RHIBufferDesc& desc) { return this->Get_Derived()->Imp_CreateHandleForNativeBuffer(type, buffer, desc); }

		STDNODISCARD RefCountPtr<Imp_Shader> CreateShader(const RHIShaderDesc& desc, const void* binary, Uint64 binarySize) { return this->Get_Derived()->Imp_CreateShader(desc, binary, binarySize); }

		STDNODISCARD RefCountPtr<Imp_Shader> CreateShaderSpecialization(Imp_Shader* baseshader, const RHIShaderSpecialization* Constants, Uint32 ConstantCount) { return this->Get_Derived()->Imp_CreateShaderSpecialization(baseshader, Constants, ConstantCount); }

		STDNODISCARD RefCountPtr<Imp_ShaderLibrary> CreateShaderLibrary(const void* binary, Uint64 binarySize) { return this->Get_Derived()->Imp_CreateShaderLibrary(binary, binarySize); }

		STDNODISCARD RefCountPtr<Imp_Sampler> CreateSampler(const RHISamplerDesc& desc) { return this->Get_Derived()->Imp_CreateSampler(desc); }
		//TODO :constexpr if to do d3d11
		STDNODISCARD RefCountPtr<Imp_InputLayout> CreateInputLayout(const RHIVertexAttributeDesc* attributes, Uint32 attributeCount, Imp_Shader* Vertexshader = nullptr) { return this->Get_Derived()->Imp_CreateInputLayout(attributes, attributeCount, Vertexshader); }

		STDNODISCARD RefCountPtr<Imp_EventQuery> CreateEventQuery(void) { return this->Get_Derived()->Imp_CreateEventQuery(); }

		void SetEventQuery(Imp_EventQuery* query, RHICommandQueue queue) { this->Get_Derived()->Imp_SetEventQuery(query, queue); }

		bool PollEventQuery(Imp_EventQuery* query) { return this->Get_Derived()->Imp_PollEventQuery(query); }

		void WaitEventQuery(Imp_EventQuery* query) { this->Get_Derived()->Imp_WaitEventQuery(query); }

		void ResetEventQuery(Imp_EventQuery* query) { this->Get_Derived()->Imp_ResetEventQuery(query); }

		RefCountPtr<Imp_TimerQuery> CreateTimerQuery(void) { return this->Get_Derived()->Imp_CreateTimerQuery(); }

		bool PollTimerQuery(Imp_TimerQuery* query) { return this->Get_Derived()->Imp_PollTimerQuery(query); }

		float Get_TimerQueryTime(Imp_TimerQuery* query) { return this->Get_Derived()->Imp_Get_TimerQueryTime(query); }

		void ResetTimerQuery(Imp_TimerQuery* query) { this->Get_Derived()->Imp_ResetTimerQuery(query); }

		STDNODISCARD RefCountPtr<Imp_FrameBuffer> CreateFrameBuffer(const RHIFrameBufferDesc<APITag>& desc) { return this->Get_Derived()->Imp_CreateFrameBuffer(desc); }

		STDNODISCARD RefCountPtr<Imp_GraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineDesc<APITag>& desc, Imp_FrameBuffer* framebuffer) { return this->Get_Derived()->Imp_CreateGraphicsPipeline(desc, framebuffer); }

		STDNODISCARD RefCountPtr<Imp_ComputePipeline> CreateComputePipeline(const RHIComputePipelineDesc<APITag>& desc) { return this->Get_Derived()->Imp_CreateComputePipeline(desc); }

		STDNODISCARD RefCountPtr<Imp_MeshletPipeline> CreateMeshletPipeline(const RHIMeshletPipelineDesc<APITag>& desc, Imp_FrameBuffer* framebuffer) { return this->Get_Derived()->Imp_CreateMeshletPipeline(desc, framebuffer); }

		STDNODISCARD RefCountPtr<Imp_BindingLayout> CreateBindingLayout(const RHIBindingLayoutDesc& desc) { return this->Get_Derived()->Imp_CreateBindingLayout(desc); }

		STDNODISCARD RefCountPtr<Imp_BindingLayout> CreateBindingLayout(const RHIBindlessLayoutDesc& desc) { return this->Get_Derived()->Imp_CreateBindingLayout(desc); }

		STDNODISCARD RefCountPtr<Imp_BindingSet> CreateBindingSet(const RHIBindingSetDesc<APITag>& desc, Imp_BindingLayout* layout) { return this->Get_Derived()->Imp_CreateBindingSet(desc, layout); }

		//TODO :

		//TODO :

		//TODO :

		STDNODISCARD RefCountPtr<Imp_CommandList> CreateCommandList(const RHICommandListParameters& desc = RHICommandListParameters{}) { return this->Get_Derived()->Imp_CreateCommandList(desc); }

		/*STDNODISCARD*/ Uint64 ExecuteCommandLists(Imp_CommandList* const* commandLists, Uint32 commandListCount, RHICommandQueue queue = RHICommandQueue::Graphics) { return this->Get_Derived()->Imp_ExecuteCommandLists(commandLists, commandListCount, queue); }

		void QueueWaitForCommandList(RHICommandQueue WaitQueue, RHICommandQueue executeQueue, Uint64 instance) { this->Get_Derived()->Imp_QueueWaitForCommandList(WaitQueue, executeQueue, instance); }

		/*STDNODISCARD*/ bool WaitForIdle(void) { return this->Get_Derived()->Imp_WaitForIdle(); }

		void RunGarbageCollection(void) { this->Get_Derived()->Imp_RunGarbageCollection(); }

		STDNODISCARD bool QueryFeatureSupport(RHIFeature feature, void* outData, Uint64 outDataSize) { return this->Get_Derived()->Imp_QueryFeatureSupport(feature, outData, outDataSize); }

		STDNODISCARD RHIFormatSupport QueryFormatSupport(RHIFormat format) { return this->Get_Derived()->Imp_QueryFormatSupport(format); }

		STDNODISCARD RHIObject Get_NativeQueue(RHIObjectType type, RHICommandQueue queue) { return this->Get_Derived()->Imp_Get_NativeQueue(type, queue); }

		STDNODISCARD Imp_MessageCallback* Get_MessageCallback(void) { return this->Get_Derived()->Imp_Get_MessageCallback(); }

		//TODO 

		//TODO

	public:
		/*STDNODISCARD*/ Uint64 ExecuteCommandList(Imp_CommandList* commandList, RHICommandQueue queue = RHICommandQueue::Graphics) { return this->ExecuteCommandLists(&commandList, 1u, queue); }


	private:
		STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
		STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
	private:
		RefCountPtr<Imp_Heap> Imp_CreateHeap(const RHIHeapDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_Texture> Imp_CreateTexture(const RHITextureDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RHIMemoryRequirements Imp_Get_TextureMemoryRequirements(Imp_Texture*) { LOG_ERROR("No Imp"); return RHIMemoryRequirements{}; }
		bool Imp_BindTextureMemory(Imp_Texture*, Imp_Heap*, Uint64) { LOG_ERROR("No Imp"); return false; }
		RefCountPtr<Imp_Texture> Imp_CreateHandleForNativeTexture(RHIObjectType, RHIObject, const RHITextureDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_StagingTexture> Imp_CreateStagingTexture(const RHITextureDesc&, RHICPUAccessMode) { LOG_ERROR("No Imp"); return nullptr; }
		void* Imp_MapStagingTexture(Imp_StagingTexture*, const RHITextureSlice&, RHICPUAccessMode, Uint64*) { LOG_ERROR("No Imp"); return nullptr; }
		void Imp_UnmapStagingTexture(Imp_StagingTexture*) { LOG_ERROR("No Imp"); }
		void Imp_Get_TextureTiling(Imp_Texture*, Uint32*, RHIPackedMipDesc*, RHITileShape*, Uint32*, RHISubresourceTiling*) { LOG_ERROR("No Imp"); }
		void Imp_UpdateTextureTileMappings(Imp_Texture*, const RHITextureTilesMapping<APITag>*, Uint32, RHICommandQueue) { LOG_ERROR("No Imp"); }
		RefCountPtr<Imp_SamplerFeedbackTexture> Imp_CreateSamplerFeedbackTexture(Imp_Texture*, const RHISamplerFeedbackTextureDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_SamplerFeedbackTexture> Imp_CreateSamplerFeedbackForNativeTexture(RHIObjectType, RHIObject, Imp_Texture*) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_Buffer> Imp_CreateBuffer(const RHIBufferDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		void* Imp_MapBuffer(Imp_Buffer*, RHICPUAccessMode) { LOG_ERROR("No Imp"); return nullptr; }
		void Imp_UnmapBuffer(Imp_Buffer*) { LOG_ERROR("No Imp"); }
		RHIMemoryRequirements Imp_Get_BufferMemoryRequirements(Imp_Buffer*) { LOG_ERROR("No Imp"); return RHIMemoryRequirements{}; }
		bool Imp_BindBufferMemory(Imp_Buffer*, Imp_Heap*, Uint64) { LOG_ERROR("No Imp"); return false; }
		RefCountPtr<Imp_Buffer> Imp_CreateHandleForNativeBuffer(RHIObjectType, RHIObject, const RHIBufferDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_Shader> Imp_CreateShader(const RHIShaderDesc&, const void*, Uint64) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_Shader> Imp_CreateShaderSpecialization(Imp_Shader*, const RHIShaderSpecialization*, Uint32) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_ShaderLibrary> Imp_CreateShaderLibrary(const void*, Uint64) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_Sampler> Imp_CreateSampler(const RHISamplerDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_InputLayout> Imp_CreateInputLayout(const RHIVertexAttributeDesc*, Uint32, Imp_Shader*) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_EventQuery> Imp_CreateEventQuery(void) { LOG_ERROR("No Imp"); return nullptr; }
		void Imp_SetEventQuery(Imp_EventQuery*, RHICommandQueue) { LOG_ERROR("No Imp"); }
		bool Imp_PollEventQuery(Imp_EventQuery*) { LOG_ERROR("No Imp"); return false; }
		void Imp_WaitEventQuery(Imp_EventQuery*) { LOG_ERROR("No Imp"); }
		void Imp_ResetEventQuery(Imp_EventQuery*) { LOG_ERROR("No Imp"); }
		RefCountPtr<Imp_TimerQuery> Imp_CreateTimerQuery(void) { LOG_ERROR("No Imp"); return nullptr; }
		bool Imp_PollTimerQuery(Imp_TimerQuery*) { LOG_ERROR("No Imp"); return false; }
		float Imp_Get_TimerQueryTime(Imp_TimerQuery*) { LOG_ERROR("No Imp"); return 0.0f; }
		void Imp_ResetTimerQuery(Imp_TimerQuery*) { LOG_ERROR("No Imp"); }
		RefCountPtr<Imp_FrameBuffer> Imp_CreateFrameBuffer(const RHIFrameBufferDesc<APITag>&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_GraphicsPipeline> Imp_CreateGraphicsPipeline(const RHIGraphicsPipelineDesc<APITag>&, Imp_FrameBuffer*) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_ComputePipeline> Imp_CreateComputePipeline(const RHIComputePipelineDesc<APITag>&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_MeshletPipeline> Imp_CreateMeshletPipeline(const RHIMeshletPipelineDesc<APITag>&, Imp_FrameBuffer*) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_BindingLayout> Imp_CreateBindingLayout(const RHIBindingLayoutDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_BindingLayout> Imp_CreateBindingLayout(const RHIBindlessLayoutDesc&) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_BindingSet> Imp_CreateBindingSet(const RHIBindingSetDesc<APITag>&, Imp_BindingLayout*) { LOG_ERROR("No Imp"); return nullptr; }
		RefCountPtr<Imp_CommandList> Imp_CreateCommandList(const RHICommandListParameters&) { LOG_ERROR("No Imp"); return nullptr; }
		Uint64 Imp_ExecuteCommandLists(Imp_CommandList* const*, Uint32, RHICommandQueue) { LOG_ERROR("No Imp"); return 0; }
		void Imp_QueueWaitForCommandList(RHICommandQueue, RHICommandQueue, Uint64) { LOG_ERROR("No Imp"); }
		bool Imp_WaitForIdle(void) { LOG_ERROR("No Imp"); return false; }
		void Imp_RunGarbageCollection(void) { LOG_ERROR("No Imp"); }
		bool Imp_QueryFeatureSupport(RHIFeature, void*, Uint64) { LOG_ERROR("No Imp"); return false; }
		RHIFormatSupport Imp_QueryFormatSupport(RHIFormat) { LOG_ERROR("No Imp"); return RHIFormatSupport::None; }
		RHIObject Imp_Get_NativeQueue(RHIObjectType, RHICommandQueue) { LOG_ERROR("No Imp"); return nullptr; }
		Imp_MessageCallback* Imp_Get_MessageCallback(void) { LOG_ERROR("No Imp"); return nullptr; }
	};

}