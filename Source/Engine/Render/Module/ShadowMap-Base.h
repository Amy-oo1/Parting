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


PARTING_MODULE(ShadowMap)


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

#include "Shader/light_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class IShadowMap {
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
	public:
		IShadowMap(void) = default;
		virtual ~IShadowMap(void) = default;

	public:
		virtual Math::MatF44 Get_WorldToUvzwMatrix(void) const = 0;

		virtual const class ICompositeView& Get_View(void) const = 0;

		virtual auto Get_Texture(void)->Imp_Texture* const = 0;

		virtual Uint32 Get_NumberOfCascades(void) const = 0;

		virtual const IShadowMap<APITag>* Get_Cascade(Uint32 index) const = 0;

		virtual Uint32 Get_NumberOfPerObjectShadows(void) const = 0;

		virtual const IShadowMap<APITag>* Get_PerObjectShadow(Uint32 index) const = 0;

		virtual Math::VecU2 Get_TextureSize(void) const = 0;

		virtual Math::BoxF2 Get_UVRange(void) const = 0;

		virtual Math::VecF2 Get_FadeRangeInTexels(void) const = 0;

		virtual bool Is_LitOutOfBounds(void) const = 0;

		virtual void FillShadowConstants(ShadowConstants& constants) const = 0;

	};
}

