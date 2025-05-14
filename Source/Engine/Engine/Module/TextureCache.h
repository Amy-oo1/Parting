#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(TextureCahe)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/SceneTypes.h"
#include "Engine/Engine/Module/CommonRenderPasses.h"

#include "Engine/Engine/Module/DescriptorTableManager.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {


	struct TextureSubresourceData final {
		Uint64 RowPitch{ 0 };
		Uint64 DepthPitch{ 0 };
		Int64 DataOffset{ 0 };
		Uint64 DataSize{ 0 };
	};

	template<RHI::APITagConcept APITag>
	struct TextureData final {
		LoadedTexture<APITag> Texture;
		SharedPtr<IBlob> Data;

		RHI::RHIFormat Format{ RHI::RHIFormat::UNKNOWN };
		RHI::RHIExtent3D Extent{ .Width{ 0 }, .Height{ 0 }, .Depth{ 1 } };
		Uint32 ArrayCount{ 1 };
		Uint32 MipLevelCount{ 1 };
		RHI::RHITextureDimension Dimension{ RHI::RHITextureDimension::Texture2D };
		bool IsRenderTarget{ false };
		bool ForceSRGB{ false };

		// ArraySlice -> MipLevel -> TextureSubresourceData
		Vector<Vector<TextureSubresourceData>> DataLayout;
	};








	template<RHI::APITagConcept APITag>
	class TextureCache final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
	public:
		TextureCache(RHI::RefCountPtr<Imp_Device> device, SharedPtr<IFileSystem> fs, SharedPtr<DescriptorTableManager<APITag>> descriptorTableManager) :
			m_Device{ device },
			m_FS{ fs },
			m_DescriptorTableManager{ descriptorTableManager }
		{
		}
		~TextureCache(void) = default;


	public:
		void Reset(void);

		void ProcessRenderingThreadCommands(CommonRenderPasses<APITag>& passes, float timeLimitMilliseconds);

		void FinalizeTexture(SharedPtr<TextureData<APITag>> texture, CommonRenderPasses<APITag>* passes, Imp_CommandList* commandList);

		SharedPtr<LoadedTexture<APITag>> LoadTextureFromMemoryAsync(const SharedPtr<IBlob>& data, const String& name, const String& mimeType, bool sRGB, tf::Executor& executor);

		bool FillTextureData(const SharedPtr<IBlob>& fileData, const SharedPtr<TextureData<APITag>>& texture, const String& extension, const String& mimeType) const;

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_CommandList> m_CommandList;

		SharedPtr<IFileSystem> m_FS;
		SharedPtr<DescriptorTableManager<APITag>> m_DescriptorTableManager;

		UnorderedMap<String, SharedPtr<TextureData<APITag>>> m_LoadedTextures;

		mutable SharedMutex m_LoadedTexturesMutex;
		Queue<SharedPtr<TextureData<APITag>>> m_TexturesToFinalize;

		Atomic<Uint32> m_TexturesRequested{ 0 };
		Atomic<Uint32> m_TexturesLoaded{ 0 };

	};



	template<RHI::APITagConcept APITag>
	inline void TextureCache<APITag>::Reset(void) {
		{
			LockGuard lock{ this->m_LoadedTexturesMutex };

			this->m_LoadedTextures.clear();

			this->m_TexturesRequested = 0;
			this->m_TexturesLoaded = 0;
		}
	}

	template<RHI::APITagConcept APITag>
	inline void TextureCache<APITag>::ProcessRenderingThreadCommands(CommonRenderPasses<APITag>& passes, float timeLimitMilliseconds) {
		Uint32 commandsExecuted{ 0 };

		while (true) {
			SharedPtr<TextureData<APITag>> texturedata{ nullptr };

			{
				LockGuard lock{ this->m_LoadedTexturesMutex };
				if (this->m_TexturesToFinalize.empty())
					break;

				texturedata = this->m_TexturesToFinalize.front();
				this->m_TexturesToFinalize.pop();
			}

			if (nullptr != texturedata->Data) {
				++commandsExecuted;

				this->m_CommandList->Open();

			}
		}

	}

	template<RHI::APITagConcept APITag>
	inline void TextureCache<APITag>::FinalizeTexture(SharedPtr<TextureData<APITag>> texture, CommonRenderPasses<APITag>* passes, Imp_CommandList* commandList) {

	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<LoadedTexture<APITag>> TextureCache<APITag>::LoadTextureFromMemoryAsync(const SharedPtr<IBlob>& data, const String& name, const String& mimeType, bool sRGB, tf::Executor& executor) {


	}

	template<RHI::APITagConcept APITag>
	inline bool TextureCache<APITag>::FillTextureData(const SharedPtr<IBlob>& fileData, const SharedPtr<TextureData<APITag>>& texture, const String& extension, const String& mimeType) const{
		if (extension == ".dds" || extension == ".DDS" || mimeType == "image/vnd-ms.dds")
		{
			texture->data = fileData;
			if (!LoadDDSTextureFromMemory(*texture))
			{
				texture->data = nullptr;
				log::message(m_ErrorLogSeverity, "Couldn't load DDS texture '%s'", texture->path.c_str());
				return false;
			}
		}
		else if (extension == ".exr" || extension == ".EXR" || mimeType == "image/aces")
		{
			float* data = nullptr;
			int width = 0, height = 0;
			char const* err = nullptr;

			// This reads only 1 or 4 channel images and duplicates channels
			// Should rewrite w/ lower level EXR functions
			if (LoadEXRFromMemory(&data, &width, &height, (uint8_t*)fileData->data(), fileData->size(), &err) == TINYEXR_SUCCESS)
			{
				uint32_t channels = 4;
				uint32_t bytesPerPixel = channels * 4;

				texture->data = std::make_shared<Blob>(data, bytesPerPixel * width * height);
				texture->width = static_cast<uint32_t>(width);
				texture->height = static_cast<uint32_t>(height);
				texture->format = nvrhi::Format::RGBA32_FLOAT;

				texture->originalBitsPerPixel = channels * 32;
				texture->isRenderTarget = true;
				texture->mipLevels = 1;
				texture->dimension = nvrhi::TextureDimension::Texture2D;

				texture->dataLayout.resize(1);
				texture->dataLayout[0].resize(1);
				texture->dataLayout[0][0].dataOffset = 0;
				texture->dataLayout[0][0].rowPitch = static_cast<size_t>(width * bytesPerPixel);
				texture->dataLayout[0][0].dataSize = static_cast<size_t>(width * height * bytesPerPixel);

				return true;
			}
			else
			{
				log::warning("Couldn't load EXR texture '%s'", texture->path.c_str());
				return false;
			}
		}
		else
		{
			int width = 0, height = 0, originalChannels = 0, channels = 0;

			if (!stbi_info_from_memory(
				static_cast<const stbi_uc*>(fileData->data()),
				static_cast<int>(fileData->size()),
				&width, &height, &originalChannels))
			{
				log::message(m_ErrorLogSeverity, "Couldn't process image header for texture '%s'", texture->path.c_str());
				return false;
			}

			bool is_hdr = stbi_is_hdr_from_memory(
				static_cast<const stbi_uc*>(fileData->data()),
				static_cast<int>(fileData->size()));

			if (originalChannels == 3)
			{
				channels = 4;
			}
			else {
				channels = originalChannels;
			}

			unsigned char* bitmap;
			int bytesPerPixel = channels * (is_hdr ? 4 : 1);

			if (is_hdr)
			{
				float* floatmap = stbi_loadf_from_memory(
					static_cast<const stbi_uc*>(fileData->data()),
					static_cast<int>(fileData->size()),
					&width, &height, &originalChannels, channels);

				bitmap = reinterpret_cast<unsigned char*>(floatmap);
			}
			else
			{
				bitmap = stbi_load_from_memory(
					static_cast<const stbi_uc*>(fileData->data()),
					static_cast<int>(fileData->size()),
					&width, &height, &originalChannels, channels);
			}

			if (!bitmap)
			{
				log::message(m_ErrorLogSeverity, "Couldn't load generic texture '%s'", texture->path.c_str());
				return false;
			}

			texture->originalBitsPerPixel = static_cast<uint32_t>(originalChannels) * (is_hdr ? 32 : 8);
			texture->width = static_cast<uint32_t>(width);
			texture->height = static_cast<uint32_t>(height);
			texture->isRenderTarget = true;
			texture->mipLevels = 1;
			texture->dimension = nvrhi::TextureDimension::Texture2D;

			texture->dataLayout.resize(1);
			texture->dataLayout[0].resize(1);
			texture->dataLayout[0][0].dataOffset = 0;
			texture->dataLayout[0][0].rowPitch = static_cast<size_t>(width * bytesPerPixel);
			texture->dataLayout[0][0].dataSize = static_cast<size_t>(width * height * bytesPerPixel);

			texture->data = std::make_shared<StbImageBlob>(bitmap);
			bitmap = nullptr; // ownership transferred to the blob

			switch (channels)
			{
			case 1:
				texture->format = is_hdr ? nvrhi::Format::R32_FLOAT : nvrhi::Format::R8_UNORM;
				break;
			case 2:
				texture->format = is_hdr ? nvrhi::Format::RG32_FLOAT : nvrhi::Format::RG8_UNORM;
				break;
			case 4:
				texture->format = is_hdr ? nvrhi::Format::RGBA32_FLOAT :
					(texture->forceSRGB ? nvrhi::Format::SRGBA8_UNORM : nvrhi::Format::RGBA8_UNORM);
				break;
			default:
				texture->data.reset(); // release the bitmap data

				log::message(m_ErrorLogSeverity, "Unsupported number of components (%d) for texture '%s'", channels, texture->path.c_str());
				return false;
			}
		}

		return true;
	}

}