#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(RHI)

PARTING_EXPORT PARTING_SUBMODE_IMPORT(Resource)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Traits)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Common)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Format)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Heap)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Buffer)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Texture)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Sampler)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(InputLayout)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Shader)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(BlendState)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(RasterState)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(DepthStencilState)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(ViewportState)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(FrameBuffer)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(ShaderBinding)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Pipeline)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Draw)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(CommandList)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Device)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Utility)

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Heap.h"
#include "RHI/Module/RHI-Buffer.h"
#include "RHI/Module/RHI-Texture.h"
#include "RHI/Module/RHI-Sampler.h"
#include "RHI/Module/RHI-InputLayout.h"
#include "RHI/Module/RHI-Shader.h"
#include "RHI/Module/RHI-BlendState.h"
#include "RHI/Module/RHI-RasterState.h"
#include "RHI/Module/RHI-DepthStencilState.h"
#include "RHI/Module/RHI-ViewportState.h"
#include "RHI/Module/RHI-FrameBuffer.h"
#include "RHI/Module/RHI-ShaderBinding.h"
#include "RHI/Module/RHI-Pipeline.h"
#include "RHI/Module/RHI-Draw.h"
#include "RHI/Module/RHI-CommandList.h"
#include "RHI/Module/RHI-Device.h"
#include "RHI/Module/RHI-Utility.h"

#endif // PARTING_MODULE_BUILD


