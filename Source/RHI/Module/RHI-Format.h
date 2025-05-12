#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(RHI, Format)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {
	//NOTE : Here enum is big specific to the platform
	PARTING_EXPORT enum class RHIFormat :Uint8 {
		UNKNOWN,

		R8_UINT,
		R8_SINT,
		R8_UNORM,
		R8_SNORM,
		RG8_UINT,
		RG8_SINT,
		RG8_UNORM,
		RG8_SNORM,
		R16_UINT,
		R16_SINT,
		R16_UNORM,
		R16_SNORM,
		R16_FLOAT,
		BGRA4_UNORM,
		B5G6R5_UNORM,
		B5G5R5A1_UNORM,
		RGBA8_UINT,
		RGBA8_SINT,
		RGBA8_UNORM,
		RGBA8_SNORM,
		BGRA8_UNORM,
		SRGBA8_UNORM,
		SBGRA8_UNORM,
		R10G10B10A2_UNORM,
		R11G11B10_FLOAT,
		RG16_UINT,
		RG16_SINT,
		RG16_UNORM,
		RG16_SNORM,
		RG16_FLOAT,
		R32_UINT,
		R32_SINT,
		R32_FLOAT,
		RGBA16_UINT,
		RGBA16_SINT,
		RGBA16_FLOAT,
		RGBA16_UNORM,
		RGBA16_SNORM,
		RG32_UINT,
		RG32_SINT,
		RG32_FLOAT,
		RGB32_UINT,
		RGB32_SINT,
		RGB32_FLOAT,
		RGBA32_UINT,
		RGBA32_SINT,
		RGBA32_FLOAT,

		D16,
		D24S8,
		X24G8_UINT,
		D32,
		D32S8,
		X32G8_UINT,

		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC2_UNORM,
		BC2_UNORM_SRGB,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_UNORM,
		BC4_SNORM,
		BC5_UNORM,
		BC5_SNORM,
		BC6H_UFLOAT,
		BC6H_SFLOAT,
		BC7_UNORM,
		BC7_UNORM_SRGB,

		COUNT,
	};

	PARTING_EXPORT enum class RHIFormatKind : Uint32 {
		Integer,
		Normalized,
		Float,
		DepthStencil
	};

	PARTING_EXPORT struct RHIFormatInfo final {
		RHIFormat Format;
		String Name;
		Uint8 BytesPerBlock;
		Uint8 BlockSize;
		RHIFormatKind Kind;
		bool HasRed : 1;
		bool HasGreen : 1;
		bool HasBlue : 1;
		bool HasAlpha : 1;
		bool HasDepth : 1;
		bool HasStencil : 1;
		bool IsSigned : 1;
		bool IsSRGB : 1;
	};

	//TODO :Remove this
	/*using enum RHIFormat;*/
	using enum RHIFormatKind;

	//TODO   space to tab
	PARTING_EXPORT HEADER_INLINE const Array<RHIFormatInfo, Tounderlying(RHIFormat::COUNT)> g_RHIFormats {
		RHIFormatInfo { RHIFormat::UNKNOWN,           "UNKNOWN",           0,   0, Integer,      false, false, false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::R8_UINT,           "R8_UINT",           1,   1, Integer,      true,  false, false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::R8_SINT,           "R8_SINT",           1,   1, Integer,      true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::R8_UNORM,          "R8_UNORM",          1,   1, Normalized,   true,  false, false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::R8_SNORM,          "R8_SNORM",          1,   1, Normalized,   true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RG8_UINT,          "RG8_UINT",          2,   1, Integer,      true,  true,  false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::RG8_SINT,          "RG8_SINT",          2,   1, Integer,      true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RG8_UNORM,         "RG8_UNORM",         2,   1, Normalized,   true,  true,  false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::RG8_SNORM,         "RG8_SNORM",         2,   1, Normalized,   true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::R16_UINT,          "R16_UINT",          2,   1, Integer,      true,  false, false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::R16_SINT,          "R16_SINT",          2,   1, Integer,      true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::R16_UNORM,         "R16_UNORM",         2,   1, Normalized,   true,  false, false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::R16_SNORM,         "R16_SNORM",         2,   1, Normalized,   true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::R16_FLOAT,         "R16_FLOAT",         2,   1, Float,        true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::BGRA4_UNORM,       "BGRA4_UNORM",       2,   1, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::B5G6R5_UNORM,      "B5G6R5_UNORM",      2,   1, Normalized,  true,  true,  true,  false, false, false, false, false },
		RHIFormatInfo { RHIFormat::B5G5R5A1_UNORM,    "B5G5R5A1_UNORM",    2,   1, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::RGBA8_UINT,        "RGBA8_UINT",        4,   1, Integer,      true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::RGBA8_SINT,        "RGBA8_SINT",        4,   1, Integer,      true,  true,  true,  true,  false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGBA8_UNORM,       "RGBA8_UNORM",       4,   1, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::RGBA8_SNORM,       "RGBA8_SNORM",       4,   1, Normalized,   true,  true,  true,  true,  false, false, true,  false },
		RHIFormatInfo { RHIFormat::BGRA8_UNORM,       "BGRA8_UNORM",       4,   1, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::SRGBA8_UNORM,      "SRGBA8_UNORM",      4,   1, Normalized,   true,  true,  true,  true,  false, false, false, true  },
		RHIFormatInfo { RHIFormat::SBGRA8_UNORM,      "SBGRA8_UNORM",      4,   1, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::R10G10B10A2_UNORM, "R10G10B10A2_UNORM", 4,   1, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::R11G11B10_FLOAT,   "R11G11B10_FLOAT",   4,   1, Float,        true,  true,  true,  false, false, false, false, false },
		RHIFormatInfo { RHIFormat::RG16_UINT,         "RG16_UINT",         4,   1, Integer,      true,  true,  false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::RG16_SINT,         "RG16_SINT",         4,   1, Integer,      true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RG16_UNORM,        "RG16_UNORM",        4,   1, Normalized,   true,  true,  false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::RG16_SNORM,        "RG16_SNORM",        4,   1, Normalized,   true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RG16_FLOAT,        "RG16_FLOAT",        4,   1, Float,        true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::R32_UINT,          "R32_UINT",          4,   1, Integer,      true,  false, false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::R32_SINT,          "R32_SINT",          4,   1, Integer,      true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::R32_FLOAT,         "R32_FLOAT",         4,   1, Float,        true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGBA16_UINT,       "RGBA16_UINT",       8,   1, Integer,      true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::RGBA16_SINT,       "RGBA16_SINT",       8,   1, Integer,      true,  true,  true,  true,  false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGBA16_FLOAT,      "RGBA16_FLOAT",      8,   1, Float,        true,  true,  true,  true,  false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGBA16_UNORM,      "RGBA16_UNORM",      8,   1, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::RGBA16_SNORM,      "RGBA16_SNORM",      8,   1, Normalized,   true,  true,  true,  true,  false, false, true,  false },
		RHIFormatInfo { RHIFormat::RG32_UINT,         "RG32_UINT",         8,   1, Integer,      true,  true,  false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::RG32_SINT,         "RG32_SINT",         8,   1, Integer,      true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RG32_FLOAT,        "RG32_FLOAT",        8,   1, Float,        true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGB32_UINT,        "RGB32_UINT",        12,  1, Integer,      true,  true,  true,  false, false, false, false, false },
		RHIFormatInfo { RHIFormat::RGB32_SINT,        "RGB32_SINT",        12,  1, Integer,      true,  true,  true,  false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGB32_FLOAT,       "RGB32_FLOAT",       12,  1, Float,        true,  true,  true,  false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGBA32_UINT,       "RGBA32_UINT",       16,  1, Integer,      true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::RGBA32_SINT,       "RGBA32_SINT",       16,  1, Integer,      true,  true,  true,  true,  false, false, true,  false },
		RHIFormatInfo { RHIFormat::RGBA32_FLOAT,      "RGBA32_FLOAT",      16,  1, Float,        true,  true,  true,  true,  false, false, true,  false },
		RHIFormatInfo { RHIFormat::D16,               "D16",               2,   1, DepthStencil, false, false, false, false, true,  false, false, false },
		RHIFormatInfo { RHIFormat::D24S8,             "D24S8",             4,   1, DepthStencil, false, false, false, false, true,  true,  false, false },
		RHIFormatInfo { RHIFormat::X24G8_UINT,        "X24G8_UINT",        4,   1, Integer,      false, false, false, false, false, true,  false, false },
		RHIFormatInfo { RHIFormat::D32,               "D32",               4,   1, DepthStencil, false, false, false, false, true,  false, false, false },
		RHIFormatInfo { RHIFormat::D32S8,             "D32S8",             8,   1, DepthStencil, false, false, false, false, true,  true,  false, false },
		RHIFormatInfo { RHIFormat::X32G8_UINT,        "X32G8_UINT",        8,   1, Integer,      false, false, false, false, false, true,  false, false },
		RHIFormatInfo { RHIFormat::BC1_UNORM,         "BC1_UNORM",         8,   4, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::BC1_UNORM_SRGB,    "BC1_UNORM_SRGB",    8,   4, Normalized,   true,  true,  true,  true,  false, false, false, true  },
		RHIFormatInfo { RHIFormat::BC2_UNORM,         "BC2_UNORM",         16,  4, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::BC2_UNORM_SRGB,    "BC2_UNORM_SRGB",    16,  4, Normalized,   true,  true,  true,  true,  false, false, false, true  },
		RHIFormatInfo { RHIFormat::BC3_UNORM,         "BC3_UNORM",         16,  4, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::BC3_UNORM_SRGB,    "BC3_UNORM_SRGB",    16,  4, Normalized,   true,  true,  true,  true,  false, false, false, true  },
		RHIFormatInfo { RHIFormat::BC4_UNORM,         "BC4_UNORM",         8,   4, Normalized,   true,  false, false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::BC4_SNORM,         "BC4_SNORM",         8,   4, Normalized,   true,  false, false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::BC5_UNORM,         "BC5_UNORM",         16,  4, Normalized,   true,  true,  false, false, false, false, false, false },
		RHIFormatInfo { RHIFormat::BC5_SNORM,         "BC5_SNORM",         16,  4, Normalized,   true,  true,  false, false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::BC6H_UFLOAT,       "BC6H_UFLOAT",       16,  4, Float,        true,  true,  true,  false, false, false, false, false },
		RHIFormatInfo { RHIFormat::BC6H_SFLOAT,       "BC6H_SFLOAT",       16,  4, Float,        true,  true,  true,  false, false, false, true,  false },
		RHIFormatInfo { RHIFormat::BC7_UNORM,         "BC7_UNORM",         16,  4, Normalized,   true,  true,  true,  true,  false, false, false, false },
		RHIFormatInfo { RHIFormat::BC7_UNORM_SRGB,    "BC7_UNORM_SRGB",    16,  4, Normalized,   true,  true,  true,  true,  false, false, false, true  }
	};

	PARTING_EXPORT constexpr const RHIFormatInfo& Get_RHIFormatInfo(RHIFormat format)noexcept {
		//TODO :
		return g_RHIFormats[Tounderlying(format)];
	}


	PARTING_EXPORT enum class RHIFormatSupport : Uint32{
		None = 0,

		Buffer = 0x00000001,
		IndexBuffer = 0x00000002,
		VertexBuffer = 0x00000004,

		Texture = 0x00000008,
		DepthStencil = 0x00000010,
		RenderTarget = 0x00000020,
		Blendable = 0x00000040,

		ShaderLoad = 0x00000080,
		ShaderSample = 0x00000100,
		ShaderUavLoad = 0x00000200,
		ShaderUavStore = 0x00000400,
		ShaderAtomic = 0x00000800,
	};
	EXPORT_ENUM_CLASS_OPERATORS(RHIFormatSupport);
}