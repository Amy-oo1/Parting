#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(BindingCache)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "ThirdParty/ShaderMake/include/ShaderMake/ShaderBlob.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	struct ShaderMacro final {
		String Name;
		String Definition;
	};

	struct StaticShader final {
		const void* pBytecode{ nullptr };
		Uint64 Size{ 0 };
	};


	template<RHI::APITagConcept APITag>
	class ShaderFactory final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
	public:
		ShaderFactory(Imp_Device* device, SharedPtr<IFileSystem> fs, const Path& basePath) :
			m_Device{ device },
			m_FS{ ::MoveTemp(fs) },
			m_BasePath{ basePath } {
		}

		~ShaderFactory(void) = default;

	public:
		SharedPtr<IBlob> Get_Bytecode(const String& FileName, String EntryName);

		// Creates a shader from binary file.
		auto CreateShader(const String& fileName, const String& entryName, const Vector<ShaderMacro>* pDefines, RHI::RHIShaderType type) -> RHI::RefCountPtr<Imp_Shader>;


		// Creates a shader from the bytecode array.
		auto CreateStaticShader(StaticShader shader, const Vector<ShaderMacro>* pDefines, const RHI::RHIShaderDesc& desc) -> RHI::RefCountPtr<Imp_Shader>;


		void ClearCache(void) { this->m_BytecodeCache.clear(); }




	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		SharedPtr<IFileSystem> m_FS;
		Path m_BasePath;

		UnorderedMap<String, SharedPtr<IBlob>> m_BytecodeCache;

	};



	template<RHI::APITagConcept APITag>
	inline SharedPtr<IBlob> ShaderFactory<APITag>::Get_Bytecode(const String& fileName, String entryName) {
		ASSERT(nullptr != this->m_FS);
		if (entryName.empty()) {
			LOG_ERROR("Shader entry name is null");

			entryName = String{ "main" };
		}

		String adjustedName{ fileName };
		if (auto pos{ adjustedName.find(".hlsl") };	pos != String::npos)
			adjustedName.erase(pos, 5);

		if (entryName != String{ "main" })
			adjustedName += "_" + String{ entryName };

		Path shaderFilePath{ this->m_BasePath / (adjustedName + ".bin") };

		SharedPtr<IBlob>& data{ this->m_BytecodeCache[shaderFilePath.generic_string()] };

		if (nullptr != data)
			return data;

		data = m_FS->ReadFile(shaderFilePath);

		if (nullptr == data) {
			LOG_ERROR("Couldn't read the binary file for shader  from "/*, fileName, shaderFilePath.generic_string().c_str()*/);
			return nullptr;
		}

		return data;
	}

	template<RHI::APITagConcept APITag>
	inline auto ShaderFactory<APITag>::CreateShader(const String& fileName, const String& entryName, const Vector<ShaderMacro>* pDefines, RHI::RHIShaderType type) -> RHI::RefCountPtr<Imp_Shader> {
		SharedPtr<IBlob> byteCode{ this->Get_Bytecode(fileName, entryName) };

		if (nullptr == byteCode)
		return nullptr;

		RHI::RHIShaderDesc desc{
			.ShaderType {type},
			.DebugName{ fileName },
			.EntryName { entryName },
		};

		//TODO :  i do not know this TODO should,maybe i forget ,oooo Cache 
		return this->CreateStaticShader(StaticShader{ .pBytecode { byteCode->Get_Data() }, .Size{ byteCode->Get_Size() } }, pDefines, desc);
	}

	template<RHI::APITagConcept APITag>
	inline auto ShaderFactory<APITag>::CreateStaticShader(StaticShader shader, const Vector<ShaderMacro>* pDefines, const RHI::RHIShaderDesc& desc) -> RHI::RefCountPtr<Imp_Shader> {
		ASSERT(nullptr != shader.pBytecode);//TODO : Debug
		ASSERT(shader.Size > 0);


		Vector<ShaderMake::ShaderConstant> constants;
		if (nullptr != pDefines)
			for (const auto& define : *pDefines)
				constants.push_back(ShaderMake::ShaderConstant{ .name{ define.Name.c_str() }, .value{ define.Definition.c_str() } });

		const void* permutationBytecode{ nullptr };
		Uint64 permutationSize{ 0 };
		if (!ShaderMake::FindPermutationInBlob(shader.pBytecode, shader.Size, constants.data(), static_cast<Uint32>(constants.size()), &permutationBytecode, &permutationSize)) {
			auto message{ ShaderMake::FormatShaderNotFoundMessage(shader.pBytecode, shader.Size, constants.data(), static_cast<Uint32>(constants.size())) };
			/*LOG_ERROR(message);*/

			return nullptr;
		}

		return this->m_Device->CreateShader(desc, permutationBytecode, permutationSize);
	}
}