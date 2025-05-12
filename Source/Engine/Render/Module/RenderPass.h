#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(RenderPass)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

PARTING_EXPORT PARTING_SUBMODE_IMPORT(SSAOPass)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#include "Engine/Render/Module/SSAOPass.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

}