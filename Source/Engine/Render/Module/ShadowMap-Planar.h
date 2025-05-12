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

#include "Engine/Render/Module/View.h"

#include "Engine/Render/Module/ShadowMap-Base.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class PlanarShadowMap final :public IShadowMap<APITag> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;

	public:
		PlanarShadowMap(Imp_Device* device, Imp_Texture* texture, Uint32 arraySlice, const RHI::RHIViewport& viewport);
		~PlanarShadowMap(void) = default;

	public:
		SharedPtr<PlanarView> Get_PlanarView(void) { return this->m_View; }

		void Set_FalloffDistance(float distance) { this->m_FalloffDistance = distance; }

		void SetupProxyView(void);

	private:
		RHI::RefCountPtr<Imp_Texture> m_ShadowMapTexture;

		Math::VecF2 m_ShadowMapSize;
		Math::VecF2 m_TextureSize;

		SharedPtr<PlanarView> m_View;

		float m_FalloffDistance{ 1.f };
	};


	template<RHI::APITagConcept APITag>
	inline PlanarShadowMap<APITag>::PlanarShadowMap(Imp_Device* device, Imp_Texture* texture, Uint32 arraySlice, const RHI::RHIViewport& viewport) :m_ShadowMapTexture{ texture } {
		const RHI::RHITextureDesc& Desc{ this->m_ShadowMapTexture->Get_Desc() };

		this->m_TextureSize = Math::VecF2{ static_cast<float>(Desc.Extent.Width), static_cast<float>(Desc.Extent.Height) };
		this->m_ShadowMapSize = Math::VecF2{ viewport.MaxX - viewport.MinX, viewport.MaxY - viewport.MinY };

		this->m_View = MakeShared<PlanarView>();
		this->m_View->Set_Viewport(viewport);
		this->m_View->Set_ArraySlice(arraySlice);
	}
}