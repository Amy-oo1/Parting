#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(RHI, Shader)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {
	PARTING_EXPORT enum class RHIShaderType : Uint16{
		None = 0x0000,

		Compute = 0x0020,

		Vertex = 0x0001,
		Hull = 0x0002,
		Domain = 0x0004,
		Geometry = 0x0008,
		Pixel = 0x0010,
		Amplification = 0x0040,
		Mesh = 0x0080,
		AllGraphics = 0x00DF,

		All = 0x3FFF,
	};
	EXPORT_ENUM_CLASS_OPERATORS(RHIShaderType);

	PARTING_EXPORT struct RHIShaderDesc final {
		RHIShaderType ShaderType{ RHIShaderType::None };
		String DebugName;
		String EntryName{ "main" };
	};

	PARTING_EXPORT class RHIShaderDescBuilder final {
	public:
		STDNODISCARD constexpr RHIShaderDescBuilder& Reset(void) { this->m_Desc = RHIShaderDesc{}; return *this; }

		STDNODISCARD constexpr RHIShaderDescBuilder& Set_ShaderType(const RHIShaderType shaderType) { this->m_Desc.ShaderType = shaderType; return *this; }
		STDNODISCARD constexpr RHIShaderDescBuilder& Set_DebugName(const String& debugName) { this->m_Desc.DebugName = debugName; return *this; }
		STDNODISCARD constexpr RHIShaderDescBuilder& Set_EntryName(const String& entryName) { this->m_Desc.EntryName = entryName; return *this; }
	
		STDNODISCARD constexpr const RHIShaderDesc& Build(void)const { return this->m_Desc; }
	private:
		RHIShaderDesc m_Desc{};

	};

	PARTING_EXPORT template<typename Derived>
	class RHIShader :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
	protected:
		RHIShader(void) = default;
		~RHIShader(void) = default;

	public:
		STDNODISCARD const RHIShaderDesc& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }

		void Get_Bytecode(const void** ppbytecode, Uint64* psize)const { return this->Get_Derived()->Imp_Get_Bytecode(ppbytecode, psize); }

	private:
		STDNODISCARD constexpr Derived* Get_Derived(void) const noexcept { return static_cast<Derived*>(this); }
	private:
		const RHIShaderDesc& Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return this->m_Desc; }
		void Imp_Get_Bytecode(const void** ppbytecode, Uint64* psize)const { LOG_ERROR("No Imp"); *ppbytecode = nullptr; *psize = 0;  return; }

	};
}