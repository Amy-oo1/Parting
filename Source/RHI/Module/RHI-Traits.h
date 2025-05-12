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

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHISamplerFeedbackTexture;


	//NOTE 4 :Sampler
	PARTING_EXPORT template<typename Derived>
	class RHISampler;

	//NOTE 5:InputLayout
	PARTING_EXPORT template<typename Derived>
	class RHIInputLayout;

	//NOTE 6:Shader
	PARTING_EXPORT template<typename Derived>
	class RHIShader;

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIShaderLibrary;

	//NOTE 7:FrameBuffer
	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIFrameBuffer;

	//NOTE 8:ShaderBinding
	PARTING_EXPORT template<typename Derived>
	class RHIBindingLayout;

	PARTING_EXPORT template<typename Derived>
	class RHIBindlessLayout;

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
	class RHIBindingSet;

	PARTING_EXPORT template<typename Derived>
	class RHIDescriptorTable;

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

	template<APITagConcept APITag>
	struct TextureType final {
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;
		using Imp_SamplerFeedbackTexture = typename RHITypeTraits<APITag>::Imp_SamplerFeedbackTexture;
	};


	PARTING_EXPORT template<APITagConcept APITag>
	struct ShaderBindingResourceType final {
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;
		using Imp_SamplerFeedbackTexture = typename RHITypeTraits<APITag>::Imp_SamplerFeedbackTexture;
		using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_Sampler = typename RHITypeTraits<APITag>::Imp_Sampler;
	};

	PARTING_EXPORT template<APITagConcept APITag>
	struct BindLayoutType final {
		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindlessLayout = typename RHITypeTraits<APITag>::Imp_BindlessLayout;
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct ShaderBindingType final {
		using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_DescriptorTable = typename RHITypeTraits<APITag>::Imp_DescriptorTable;
	};

}