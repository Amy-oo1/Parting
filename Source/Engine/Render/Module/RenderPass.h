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

#include "Engine/Render/Module/RenderPass-Base.h"
#include "Engine/Render/Module/RenderPass-DepthPass.h"
#include "Engine/Render/Module/RenderPass-ToneMappingPass.h"
#include "Engine/Render/Module/RenderPass-TemporalAntiAliasingPass.h"
#include "Engine/Render/Module/RenderPass-ForwardShadingPass.h"
#include "Engine/Render/Module/RenderPass-GBufferFillPass.h"
#include "Engine/Render/Module/RenderPass-PixelReadbackPass.h"
#include "Engine/Render/Module/RenderPass-MipMapGenPass.h"
#include "Engine/Render/Module/RenderPass-DeferredLightingPass.h"
#include "Engine/Render/Module/RenderPass-SkyPass.h"
#include "Engine/Render/Module/RenderPass-SSAOPass.h"
#include "Engine/Render/Module/RenderPass-LightProbeProcessingPass.h"
#include "Engine/Render/Module/RenderPass-BloomPass.h"


#endif // PARTING_MODULE_BUILD

namespace Parting {

}