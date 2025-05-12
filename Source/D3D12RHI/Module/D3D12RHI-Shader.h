#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(D3D12RHI, Shader)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

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

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"
#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	class Shader final :public RHIShader<Shader> {
		friend class RHIResource<Shader>;
		friend class RHIShader<Shader>;

		friend class ShaderLibrary;
		friend class Device;
	public:
		Shader(void) = default;
		Shader(const char* entryName, RHIShaderType shaderType) {
			this->m_Desc.ShaderType = shaderType;
			this->m_Desc.EntryName = entryName;
		}

		~Shader(void) = default;

	public:

	private:

	private:
		RHIShaderDesc m_Desc;
		Vector<char> m_Bytecode;
	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };

		const RHIShaderDesc& Imp_Get_Desc(void)const { return this->m_Desc; }
		void Imp_Get_Bytecode(const void** ppbytecode, Uint64* psize)const {
			if (ppbytecode) *ppbytecode = this->m_Bytecode.data();
			if (psize) *psize = this->m_Bytecode.size();
		}

	};

	class ShaderLibrary final :public RHIShaderLibrary<ShaderLibrary, D3D12Tag> {
		friend class RHIResource<ShaderLibrary>;
		friend class RHIShaderLibrary<ShaderLibrary, D3D12Tag>;

		friend class Shader;
		friend class Device;
	public:
		ShaderLibrary(void) = default;
		~ShaderLibrary(void) = default;
	public:

	private:
		Vector<char> m_Bytecode;
	private:

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };

		void Imp_Get_Bytecode(const void** ppbytecode, Uint64* psize)const {
			if (ppbytecode) *ppbytecode = this->m_Bytecode.data();
			if (psize) *psize = this->m_Bytecode.size();
		}

		RefCountPtr<Shader> Imp_Get_Shader(const char* EntryName, RHIShaderType shaderType)const;
	};

	RefCountPtr<Shader> ShaderLibrary::Imp_Get_Shader(const char* EntryName, RHIShaderType shaderType) const{
		return RefCountPtr<Shader>::Create(new Shader{ EntryName, shaderType });
	}
		
}