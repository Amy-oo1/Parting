#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(DeviceManager)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

PARTING_IMPORT RHI;

PARTING_EXPORT PARTING_SUBMODE_IMPORT(Base);
PARTING_EXPORT PARTING_SUBMODE_IMPORT(DeviceManager, D3D12)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#include "RHI/Module/RHI.h"

#include "Engine/Application/Module/DeviceManager-Base.h"
#include "Engine/Application/Module/DeviceManager-D3D12.h"

#endif // PARTING_MODULE_BUILD
