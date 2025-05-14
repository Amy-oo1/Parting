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

#endif // PARTING_MODULE_BUILD

namespace Parting {

	enum class MaterialResource :Uint8 {
		ConstantBuffer,
		Sampler,
		DiffuseTexture,
		SpecularTexture,
		NormalTexture,
		EmissiveTexture,
		OcclusionTexture,
		TransmissionTexture,
		OpacityTexture
	};



	struct MaterialResourceBinding final {
		MaterialResource Resource;
		Uint32 Slot; // type depends on resource
	};

	template<RHI::APITagConcept APITag>
	class MaterialBindingCache final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_BindLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_SamplerFeedbackTexture = typename RHI::RHITypeTraits<APITag>::Imp_SamplerFeedbackTexture;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
	public:
		MaterialBindingCache(
			Imp_Device* device,
			RHI::RHIShaderType shaderType,
			Uint32 RegisterSpace,
			bool registerSpaceIsDescriptorSet,
			const Vector<MaterialResourceBinding>& bindings,
			Imp_Sampler* sampler,
			Imp_Texture* fallbackTexture,
			bool trackLiveness = true
		);
		~MaterialBindingCache(void) = default;

	private:

		RHI::RefCountPtr<Imp_Device> m_Device;
		Vector<MaterialResourceBinding> m_BindingDesc;
		RHI::RefCountPtr<Imp_Texture> m_FallbackTexture;
		RHI::RefCountPtr<Imp_Sampler> m_Sampler;
		bool m_TrackLiveness;

		RHI::RefCountPtr<Imp_BindLayout> m_BindingLayout;
		UnorderedMap<const Material<APITag>*, RHI::RefCountPtr<Imp_BindingSet>> m_BindingSets;



	};



	template<RHI::APITagConcept APITag>
	inline MaterialBindingCache<APITag>::MaterialBindingCache(Imp_Device* device, RHI::RHIShaderType shaderType, Uint32 RegisterSpace, bool registerSpaceIsDescriptorSet, const Vector<MaterialResourceBinding>& bindings, Imp_Sampler* sampler, Imp_Texture* fallbackTexture, bool trackLiveness) :
		m_Device{ device },
		m_BindingDesc{ bindings },
		m_FallbackTexture{ fallbackTexture },
		m_Sampler{ sampler },
		m_TrackLiveness{ trackLiveness } {
		RHI::RHIBindingLayoutDescBuilder layoutDescBuilder;
		layoutDescBuilder.Set_Visibility(shaderType).Set_RegisterSpace(RegisterSpace).Set_RegisterSpaceIsDescriptorSet(registerSpaceIsDescriptorSet);

		for (const auto& item : bindings) {
			RHI::RHIBindingLayoutItem layoutItem{ .Slot{ item.Slot } };

			switch (item.Resource) {
				using enum MaterialResource;
			case MaterialResource::ConstantBuffer:
				layoutItem.Type = RHI::RHIResourceType::ConstantBuffer;
				break;
			case DiffuseTexture:case SpecularTexture:case NormalTexture:case EmissiveTexture:case OcclusionTexture:case TransmissionTexture:case OpacityTexture:
				layoutItem.Type = RHI::RHIResourceType::Texture_SRV;
				break;
			case MaterialResource::Sampler:
				layoutItem.Type = RHI::RHIResourceType::Sampler;
				break;
			default:
				ASSERT(false);
				break;
			}
			layoutDescBuilder.AddBinding(layoutItem);
		}

		this ->m_BindingLayout = m_Device->CreateBindingLayout(layoutDescBuilder.Build());
	}



}