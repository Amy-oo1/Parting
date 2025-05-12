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

#include "Engine/Render/Module/ShadowMap-Base.h"
#include "Engine/Render/Module/ShadowMap-Planar.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class CascadedShadowMap final :public IShadowMap<APITag> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
	public:
		CascadedShadowMap(Imp_Device* device, Uint32 resolution, Uint32 numCascades, Uint32 numPerObjectShadows, RHI::RHIFormat format, bool isUAV = false);
		~CascadedShadowMap(void) = default;


	public:


	private:
		RHI::RefCountPtr<Imp_Texture> m_ShadowMapTexture;
		Vector<SharedPtr<PlanarShadowMap<APITag>>> m_Cascades;
		Vector<SharedPtr<PlanarShadowMap<APITag>>> m_PerObjectShadows;

		CompositeView m_CompositeView;

		Uint32 m_CascadeCount{ 0 };


	};
	template<RHI::APITagConcept APITag>
	inline CascadedShadowMap<APITag>::CascadedShadowMap(Imp_Device* device, Uint32 resolution, Uint32 numCascades, Uint32 numPerObjectShadows, RHI::RHIFormat format, bool isUAV) {
		ASSERT(numCascades > 0);
		ASSERT(numCascades <= 4);

		RHI::RHITextureDesc desc{
			.Extent {.Width{ resolution }, .Height{ resolution } },
			.ArrayCount { numCascades + numPerObjectShadows },
			.Format { format },
			.Dimension { RHI::RHITextureDimension::Texture2DArray },
			.IsRenderTarget { true },
			.IsUAV { isUAV },
			.IsTypeless { true },
			.ClearValue {Color{ 1.f } },
			.InitialState { RHI::RHIResourceState::ShaderResource },
			.KeepInitialState { true },
		};
		this->m_ShadowMapTexture = device->CreateTexture(desc);

		auto cascadeViewport{ RHI::RHIViewport::Build(static_cast<float>(resolution), static_cast<float>(resolution)) };

		for (Uint32 cascade = 0; cascade < numCascades; ++cascade) {
			SharedPtr<PlanarShadowMap<APITag>> planarShadowMap{ MakeShared<PlanarShadowMap<APITag>>(device, this->m_ShadowMapTexture.Get(), cascade , cascadeViewport) };
			this->m_Cascades.push_back(planarShadowMap);

			this->m_CompositeView.AddView(planarShadowMap->Get_PlanarView());
		}


		for (Uint32 Object = 0; Object < numPerObjectShadows; ++Object) {
			this->m_PerObjectShadows.emplace_back(MakeShared<PlanarShadowMap<APITag>>(device, this->m_ShadowMapTexture.Get(), numCascades + Object, cascadeViewport));
			this->m_PerObjectShadows.back()->Set_FalloffDistance(0.f); // disable falloff on per-object shadows: their bboxes are short by design
		}
	}
}