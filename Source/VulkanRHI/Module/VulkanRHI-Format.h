#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(D3D12RHI, Format)

PARTING_IMPORT DirectX12Wrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;

PARTING_IMPORT RHI;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	struct FormatMapping final{
		RHIFormat AbstractFormat;
		VkFormat ResourceFormat;
	};

	constexpr Array<FormatMapping, Tounderlying(RHIFormat::COUNT)> c_FormatMap{
		FormatMapping{ RHIFormat::UNKNOWN,				VK_FORMAT_UNDEFINED					},
		FormatMapping{ RHIFormat::R8_UINT,				VK_FORMAT_R8_UINT					},
		FormatMapping{ RHIFormat::R8_SINT,				VK_FORMAT_R8_SINT					},
		FormatMapping{ RHIFormat::R8_UNORM,				VK_FORMAT_R8_UNORM					},
		FormatMapping{ RHIFormat::R8_SNORM,				VK_FORMAT_R8_SNORM					},
		FormatMapping{ RHIFormat::RG8_UINT,				VK_FORMAT_R8G8_UINT					},
		FormatMapping{ RHIFormat::RG8_SINT,				VK_FORMAT_R8G8_SINT					},
		FormatMapping{ RHIFormat::RG8_UNORM,			VK_FORMAT_R8G8_UNORM				},
		FormatMapping{ RHIFormat::RG8_SNORM,			VK_FORMAT_R8G8_SNORM				},
		FormatMapping{ RHIFormat::R16_UINT,				VK_FORMAT_R16_UINT					},
		FormatMapping{ RHIFormat::R16_SINT,				VK_FORMAT_R16_SINT					},
		FormatMapping{ RHIFormat::R16_UNORM,			VK_FORMAT_R16_UNORM					},
		FormatMapping{ RHIFormat::R16_SNORM,			VK_FORMAT_R16_SNORM					},
		FormatMapping{ RHIFormat::R16_FLOAT,			VK_FORMAT_R16_SFLOAT				},
		FormatMapping{ RHIFormat::BGRA4_UNORM,			VK_FORMAT_B4G4R4A4_UNORM_PACK16		},
		FormatMapping{ RHIFormat::B5G6R5_UNORM,			VK_FORMAT_B5G6R5_UNORM_PACK16		},
		FormatMapping{ RHIFormat::B5G5R5A1_UNORM,		VK_FORMAT_B5G5R5A1_UNORM_PACK16		},
		FormatMapping{ RHIFormat::RGBA8_UINT,			VK_FORMAT_R8G8B8A8_UINT				},
		FormatMapping{ RHIFormat::RGBA8_SINT,			VK_FORMAT_R8G8B8A8_SINT				},
		FormatMapping{ RHIFormat::RGBA8_UNORM,			VK_FORMAT_R8G8B8A8_UNORM			},
		FormatMapping{ RHIFormat::RGBA8_SNORM,			VK_FORMAT_R8G8B8A8_SNORM			},
		FormatMapping{ RHIFormat::BGRA8_UNORM,			VK_FORMAT_B8G8R8A8_UNORM			},
		FormatMapping{ RHIFormat::SRGBA8_UNORM,			VK_FORMAT_R8G8B8A8_SRGB				},
		FormatMapping{ RHIFormat::SBGRA8_UNORM,			VK_FORMAT_B8G8R8A8_SRGB				},
		FormatMapping{ RHIFormat::R10G10B10A2_UNORM,	VK_FORMAT_A2B10G10R10_UNORM_PACK32	},
		FormatMapping{ RHIFormat::R11G11B10_FLOAT,		VK_FORMAT_B10G11R11_UFLOAT_PACK32	},
		FormatMapping{ RHIFormat::RG16_UINT,			VK_FORMAT_R16G16_UINT				},
		FormatMapping{ RHIFormat::RG16_SINT,			VK_FORMAT_R16G16_SINT				},
		FormatMapping{ RHIFormat::RG16_UNORM,			VK_FORMAT_R16G16_UNORM				},
		FormatMapping{ RHIFormat::RG16_SNORM,			VK_FORMAT_R16G16_SNORM				},
		FormatMapping{ RHIFormat::RG16_FLOAT,			VK_FORMAT_R16G16_SFLOAT				},
		FormatMapping{ RHIFormat::R32_UINT,				VK_FORMAT_R32_UINT					},
		FormatMapping{ RHIFormat::R32_SINT,				VK_FORMAT_R32_SINT					},
		FormatMapping{ RHIFormat::R32_FLOAT,			VK_FORMAT_R32_SFLOAT				},
		FormatMapping{ RHIFormat::RGBA16_UINT,			VK_FORMAT_R16G16B16A16_UINT			},
		FormatMapping{ RHIFormat::RGBA16_SINT,			VK_FORMAT_R16G16B16A16_SINT			},
		FormatMapping{ RHIFormat::RGBA16_FLOAT,			VK_FORMAT_R16G16B16A16_SFLOAT		},
		FormatMapping{ RHIFormat::RGBA16_UNORM,			VK_FORMAT_R16G16B16A16_UNORM		},
		FormatMapping{ RHIFormat::RGBA16_SNORM,			VK_FORMAT_R16G16B16A16_SNORM		},
		FormatMapping{ RHIFormat::RG32_UINT,			VK_FORMAT_R32G32_UINT				},
		FormatMapping{ RHIFormat::RG32_SINT,			VK_FORMAT_R32G32_SINT				},
		FormatMapping{ RHIFormat::RG32_FLOAT,			VK_FORMAT_R32G32_SFLOAT				},
		FormatMapping{ RHIFormat::RGB32_UINT,			VK_FORMAT_R32G32B32_UINT			},
		FormatMapping{ RHIFormat::RGB32_SINT,			VK_FORMAT_R32G32B32_SINT			},
		FormatMapping{ RHIFormat::RGB32_FLOAT,			VK_FORMAT_R32G32B32_SFLOAT			},
		FormatMapping{ RHIFormat::RGBA32_UINT,			VK_FORMAT_R32G32B32A32_UINT			},
		FormatMapping{ RHIFormat::RGBA32_SINT,			VK_FORMAT_R32G32B32A32_SINT			},
		FormatMapping{ RHIFormat::RGBA32_FLOAT,			VK_FORMAT_R32G32B32A32_SFLOAT		},
		FormatMapping{ RHIFormat::D16,					VK_FORMAT_D16_UNORM					},
		FormatMapping{ RHIFormat::D24S8,				VK_FORMAT_D24_UNORM_S8_UINT			},
		FormatMapping{ RHIFormat::X24G8_UINT,			VK_FORMAT_D24_UNORM_S8_UINT			},
		FormatMapping{ RHIFormat::D32,					VK_FORMAT_D32_SFLOAT				},
		FormatMapping{ RHIFormat::D32S8,				VK_FORMAT_D32_SFLOAT_S8_UINT		},
		FormatMapping{ RHIFormat::X32G8_UINT,			VK_FORMAT_D32_SFLOAT_S8_UINT		},
		FormatMapping{ RHIFormat::BC1_UNORM,			VK_FORMAT_BC1_RGBA_UNORM_BLOCK		},
		FormatMapping{ RHIFormat::BC1_UNORM_SRGB,		VK_FORMAT_BC1_RGBA_SRGB_BLOCK		},
		FormatMapping{ RHIFormat::BC2_UNORM,			VK_FORMAT_BC2_UNORM_BLOCK			},
		FormatMapping{ RHIFormat::BC2_UNORM_SRGB,		VK_FORMAT_BC2_SRGB_BLOCK			},
		FormatMapping{ RHIFormat::BC3_UNORM,			VK_FORMAT_BC3_UNORM_BLOCK			},
		FormatMapping{ RHIFormat::BC3_UNORM_SRGB,		VK_FORMAT_BC3_SRGB_BLOCK			},
		FormatMapping{ RHIFormat::BC4_UNORM,			VK_FORMAT_BC4_UNORM_BLOCK			},
		FormatMapping{ RHIFormat::BC4_SNORM,			VK_FORMAT_BC4_SNORM_BLOCK			},
		FormatMapping{ RHIFormat::BC5_UNORM,			VK_FORMAT_BC5_UNORM_BLOCK			},
		FormatMapping{ RHIFormat::BC5_SNORM,			VK_FORMAT_BC5_SNORM_BLOCK			},
		FormatMapping{ RHIFormat::BC6H_UFLOAT,			VK_FORMAT_BC6H_UFLOAT_BLOCK			},
		FormatMapping{ RHIFormat::BC6H_SFLOAT,			VK_FORMAT_BC6H_SFLOAT_BLOCK			},
		FormatMapping{ RHIFormat::BC7_UNORM,			VK_FORMAT_BC7_UNORM_BLOCK			},
		FormatMapping{ RHIFormat::BC7_UNORM_SRGB,		VK_FORMAT_BC7_SRGB_BLOCK			}
		};

	VkFormat ConvertFormat(RHIFormat format) { return c_FormatMap[Tounderlying(format)].ResourceFormat; }
}