#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(RHI,Traits)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"


#endif // PARTING_MODULE_BUILD

namespace RHI {
	struct D3D12Tag {};

	struct VulkanTag {};

	PARTING_EXPORT template<typename Tag>
		concept APITagConcept = requires{sizeof(Tag) == 1; };


	//NOTE :Forward declaration this be sorted althoug maybe equal

	//NOTE 1:Heap
	PARTING_EXPORT template<typename Derived>
	class RHIHeap;

	//NOTE 2:Buffer
	PARTING_EXPORT template<typename Derived>
	class RHIBuffer;

	//NOTE 3:Texture
	PARTING_EXPORT template<typename Derived>
	class RHITexture;

	PARTING_EXPORT template<typename Derived>
	class RHIStagingTexture;

	//NOTE 4 :Sampler
	PARTING_EXPORT template<typename Derived>
	class RHISampler;

	//NOTE 5:InputLayout
	PARTING_EXPORT template<typename Derived>
	class RHIInputLayout;

	//NOTE 6:Shader
	PARTING_EXPORT template<typename Derived>
	class RHIShader;

	//NOTE 7:FrameBuffer
	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIFrameBuffer;

	//NOTE 8:ShaderBinding
	PARTING_EXPORT template<typename Derived>
	class RHIBindingLayout;

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIBindingSet;

	//NOTE 9:Pipeline
	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIGraphicsPipeline;

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIComputePipeline;

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIMeshletPipeline;

	//NOTE !0 :Draw


	//NOTE 11 CommandList
	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHICommandList;

	//NOTE 12 Device
	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIDevice;
	

	PARTING_EXPORT template<typename APITag>
	struct RHITypeTraits;


	PARTING_EXPORT template<APITagConcept APITag>
	struct ShaderBindingResourceType final {
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_Sampler = typename RHITypeTraits<APITag>::Imp_Sampler;
	};
}