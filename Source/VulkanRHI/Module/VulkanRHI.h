#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(D3D12RHI)

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
PARTING_EXPORT PARTING_SUBMODE_IMPORT(CommandList)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Device)

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"
#include "VulkanRHI/Module/VulkanRHI-Format.h"
#include "VulkanRHI/Module/VulkanRHI-Heap.h"
#include "VulkanRHI/Module/VulkanRHI-Buffer.h"
#include "VulkanRHI/Module/VulkanRHI-Texture.h"
#include "VulkanRHI/Module/VulkanRHI-Sampler.h"
#include "VulkanRHI/Module/VulkanRHI-InputLayout.h"
#include "VulkanRHI/Module/VulkanRHI-Shader.h"
#include "VulkanRHI/Module/VulkanRHI-BlendState.h"
#include "VulkanRHI/Module/VulkanRHI-RasterState.h"
#include "VulkanRHI/Module/VulkanRHI-DepthStencilState.h"
#include "VulkanRHI/Module/VulkanRHI-ViewportState.h"
#include "VulkanRHI/Module/VulkanRHI-FrameBuffer.h"
#include "VulkanRHI/Module/VulkanRHI-ShaderBinding.h"
#include "VulkanRHI/Module/VulkanRHI-Pipeline.h"
#include "VulkanRHI/Module/VulkanRHI-CommandList.h"
#include "VulkanRHI/Module/VulkanRHI-Device.h"
#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {

}