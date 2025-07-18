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


PARTING_MODULE(DrawStrategy)


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
#include "Core/VFS/Module/VFS.h"
#include "Core/Json/Module/Json.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Engine/Module/SceneGraph.h"
#include "Engine/Render/Module/ShadowMap.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace Parting {

	template<RHI::APITagConcept APITag>
	inline void Light<APITag>::FillLightConstants(Shader::LightConstants& lightConstants) const {
		lightConstants.Color = this->LightColor;
		lightConstants.ShadowCascades = Math::VecI4{ -1 };
		lightConstants.PerObjectShadows = Math::VecI4{ -1 };
		lightConstants.ShadowChannel = Math::VecI4{ this->ShadowChannel, -1, -1, -1 };
		if (nullptr != this->ShadowMap)
			lightConstants.OutOfBoundsShadow = this->ShadowMap->Is_LitOutOfBounds() ? 1.f : 0.f;
		else
			lightConstants.OutOfBoundsShadow = 1.f;
	}

}