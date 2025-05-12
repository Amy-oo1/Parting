#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"


PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(RHI, Device)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;
PARTING_IMPORT Algorithm;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(BlendState)
PARTING_SUBMODE_IMPORT(Heap)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(InputLayout)
PARTING_SUBMODE_IMPORT(Shader)
PARTING_SUBMODE_IMPORT(RasterState)
PARTING_SUBMODE_IMPORT(DepthStencilState)
PARTING_SUBMODE_IMPORT(ViewportState)
PARTING_SUBMODE_IMPORT(FrameBuffer)
PARTING_SUBMODE_IMPORT(Sampler)
PARTING_SUBMODE_IMPORT(ShaderBinding)
PARTING_SUBMODE_IMPORT(Pipeline)
PARTING_SUBMODE_IMPORT(Draw)
PARTING_SUBMODE_IMPORT(CommandList)


#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Container/Module/Container.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-BlendState.h"
#include "RHI/Module/RHI-Heap.h"
#include "RHI/Module/RHI-Texture.h"
#include "RHI/Module/RHI-Buffer.h"
#include "RHI/Module/RHI-InputLayout.h"
#include "RHI/Module/RHI-Shader.h"
#include "RHI/Module/RHI-RasterState.h"
#include "RHI/Module/RHI-DepthStencilState.h"
#include "RHI/Module/RHI-ViewportState.h"
#include "RHI/Module/RHI-FrameBuffer.h"
#include "RHI/Module/RHI-Sampler.h"

#include "RHI/Module/RHI-ShaderBinding.h"
#include "RHI/Module/RHI-Pipeline.h"
#include "RHI/Module/RHI-Draw.h"
#include "RHI/Module/RHI-CommandList.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace RHI {
	
}