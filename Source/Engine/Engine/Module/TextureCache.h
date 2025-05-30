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

#include "ThirdParty/taskflow/taskflow.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

#if defined (_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4018) // Silence warning from tinyEXR
#endif

#define TINYEXR_IMPLEMENTATION
#include "ThirdParty/tinyexr/tinyexr.h"

#if defined (_MSC_VER)
#pragma warning(pop)
#endif

#include "ThirdParty/Windows/DDS.h"
#define D3D11_RESOURCE_MISC_TEXTURECUBE 0x4

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

#endif // PARTING_MODULE_BUILD

namespace Parting {

	class StbImageBlob final : public IBlob {
	public:
		StbImageBlob(unsigned char* _data) :
			IBlob{},
			m_Data{ _data } {
		}

		virtual ~StbImageBlob(void) {
			if (nullptr != this->m_Data) {
				stbi_image_free(this->m_Data);
				m_Data = nullptr;
			}
		}

		virtual const void* Get_Data(void) const override { return this->m_Data; }

		virtual Uint64 Get_Size(void) const override { return 0; /*nobody cares*/ }//TODO :maybe this func be call can push a log info

	private:
		unsigned char* m_Data{ nullptr };

	};


	struct TextureSubresourceData final {
		Uint64 RowPitch{ 0 };
		Uint64 DepthPitch{ 0 };
		Int64 DataOffset{ 0 };
		Uint64 DataSize{ 0 };
	};

	template<RHI::APITagConcept APITag>
	struct TextureData final : public LoadedTexture<APITag> {
		SharedPtr<IBlob> Data;

		RHI::RHIFormat Format{ RHI::RHIFormat::UNKNOWN };
		RHI::RHIExtent3D Extent{ .Width{ 0 }, .Height{ 0 }, .Depth{ 1 } };
		Uint32 ArrayCount{ 1 };
		Uint32 MipLevelCount{ 1 };
		RHI::RHITextureDimension Dimension{ RHI::RHITextureDimension::Texture2D };
		bool IsRenderTarget{ false };
		bool ForceSRGB{ false };

		// ArraySlice -> MipLevel -> TextureSubresourceData
		Vector<Vector<TextureSubresourceData>> DataLayout;//NOTE : Size equal arraycount miplevelcount...
	};








	template<RHI::APITagConcept APITag>
	class TextureCache final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_StagingTexture = typename RHI::RHITypeTraits<APITag>::Imp_StagingTexture;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
	public:
		TextureCache(Imp_Device* device, SharedPtr<IFileSystem> fs) :
			m_Device{ device },
			m_FS{ ::MoveTemp(fs) }
		{
		}
		~TextureCache(void) = default;


	public:
		void Reset(void);

		bool FindTextureInCache(const Path& path, SharedPtr<TextureData<APITag>>& texture);

		SharedPtr<IBlob> ReadTextureFile(const Path& path) const;

		bool FillTextureData(const SharedPtr<IBlob>& fileData, const SharedPtr<TextureData<APITag>>& texture, const String& extension, const String& mimeType) const;

		void FinalizeTexture(SharedPtr<TextureData<APITag>> texture, CommonRenderPasses<APITag>* passes, Imp_CommandList* commandList);


		bool ProcessRenderingThreadCommands(CommonRenderPasses<APITag>& passes, float timeLimitMilliseconds);

		void TextureLoaded(SharedPtr<TextureData<APITag>> texture);

		SharedPtr<LoadedTexture<APITag>> LoadTextureFromMemoryAsync(const SharedPtr<IBlob>& data, const String& name, const String& mimeType, bool sRGB, tf::Executor& executor);

		SharedPtr<LoadedTexture<APITag>> LoadTextureFromMemoryDeferred(const SharedPtr<IBlob>& data, const String& name, const String& mimeType, bool sRGB);

		SharedPtr<LoadedTexture<APITag>> LoadTextureFromFile(const Path& path, bool sRGB, CommonRenderPasses<APITag>* passes, Imp_CommandList* commandList);

		SharedPtr<LoadedTexture<APITag>> LoadTextureFromFileAsync(const Path& path, bool sRGB, tf::Executor& executor);

		SharedPtr<LoadedTexture<APITag>> LoadTextureFromFileDeferred(const Path& path, bool sRGB);

		void LoadingFinished(void) { this->m_CommandList.Reset(); }

	private:
		static bool LoadDDSTextureFromMemory(TextureData<APITag>& textureInfo);

		static auto CreateDDSTextureFromMemory(Imp_Device* device, Imp_CommandList* commandList, SharedPtr<IBlob> data, const char* debugName = nullptr, bool forceSRGB = false) -> RHI::RefCountPtr<Imp_Texture>;

		static SharedPtr<IBlob> SaveStagingTextureAsDDS(Imp_Device* device, Imp_StagingTexture* stagingTexture);

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_CommandList> m_CommandList;

		SharedPtr<IFileSystem> m_FS;

		UnorderedMap<String, SharedPtr<TextureData<APITag>>> m_LoadedTextures;
		mutable SharedMutex m_LoadedTexturesMutex;

		Queue<SharedPtr<TextureData<APITag>>> m_TexturesToFinalize;
		Uint32 m_TexturesFinalized{ 0 };
		Mutex m_TexturesToFinalizeMutex;

		Uint32 m_MaxTextureSize{ 0 };//NOTE : Limit Texture Size

		bool m_GenerateMipmaps{ true };

		Atomic<Uint32> m_TexturesRequested{ 0 };
		Atomic<Uint32> m_TexturesLoaded{ 0 };
	};


	Uint32 GetMipLevelsNum(Uint32 width, Uint32 height) {//TODO Into Utility
		Uint32 size{ Math::Min(width, height) };
		return static_cast<Uint32>(Math::Log2f(static_cast<float>(size)) / Math::Log2f(2.0f)) + 1;
	}

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
	inline bool TextureCache<APITag>::FindTextureInCache(const Path& path, SharedPtr<TextureData<APITag>>& texture) {
		{
			LockGuard guard(this->m_LoadedTexturesMutex);

			// First see if this texture is already loaded (or being loaded).
			texture = this->m_LoadedTextures[path.generic_string()];
			if (nullptr != texture)
				return true;

			// Allocate a new texture slot for this file name and return it. Load the file later in a thread pool.
			// LoadTextureFromFileAsync function for a given scene is only called from one thread, so there is no 
			// chance of loading the same texture twice.

			texture = MakeShared<TextureData<APITag>>();
			this->m_LoadedTextures[path.generic_string()] = texture;
		}

		++this->m_TexturesRequested;

		return false;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<IBlob> TextureCache<APITag>::ReadTextureFile(const Path& path) const {
		auto Filedata{ this->m_FS->ReadFile(path) };

		if (nullptr == Filedata)
			LOG_ERROR("Couldn't load texture file {1}"/*, path.generic_string().c_str()*/);

		return Filedata;
	}

	template<RHI::APITagConcept APITag>
	inline bool TextureCache<APITag>::ProcessRenderingThreadCommands(CommonRenderPasses<APITag>& passes, float timeLimitMilliseconds) {
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

				if (nullptr == this->m_CommandList)
					this->m_CommandList = this->m_Device->CreateCommandList();

				this->m_CommandList->Open();

				this->FinalizeTexture(texturedata, &passes, this->m_CommandList.Get());

				this->m_CommandList->Close();
				this->m_Device->ExecuteCommandList(this->m_CommandList);
				this->m_Device->RunGarbageCollection();

			}
		}

		return commandsExecuted > 0;

	}

	template<RHI::APITagConcept APITag>
	inline void TextureCache<APITag>::FinalizeTexture(SharedPtr<TextureData<APITag>> texture, CommonRenderPasses<APITag>* passes, Imp_CommandList* commandList) {
		ASSERT(nullptr != texture->Data);
		ASSERT(nullptr != commandList);

		Uint32 originalWidth{ texture->Extent.Width };
		Uint32 originalHeight{ texture->Extent.Height };

		bool isBlockCompressed{//TODO :or use compression format ,but not directly use the format
			(texture->Format == RHI::RHIFormat::BC1_UNORM) ||
			(texture->Format == RHI::RHIFormat::BC1_UNORM_SRGB) ||
			(texture->Format == RHI::RHIFormat::BC2_UNORM) ||
			(texture->Format == RHI::RHIFormat::BC2_UNORM_SRGB) ||
			(texture->Format == RHI::RHIFormat::BC3_UNORM) ||
			(texture->Format == RHI::RHIFormat::BC3_UNORM_SRGB) ||
			(texture->Format == RHI::RHIFormat::BC4_SNORM) ||
			(texture->Format == RHI::RHIFormat::BC4_UNORM) ||
			(texture->Format == RHI::RHIFormat::BC5_SNORM) ||
			(texture->Format == RHI::RHIFormat::BC5_UNORM) ||
			(texture->Format == RHI::RHIFormat::BC6H_SFLOAT) ||
			(texture->Format == RHI::RHIFormat::BC6H_UFLOAT) ||
			(texture->Format == RHI::RHIFormat::BC7_UNORM) ||
			(texture->Format == RHI::RHIFormat::BC7_UNORM_SRGB)
		};

		if (isBlockCompressed) {
			originalWidth = (originalWidth + 3) & ~3;
			originalHeight = (originalHeight + 3) & ~3;
		}

		Uint32 scaledWidth{ originalWidth };
		Uint32 scaledHeight{ originalHeight };

		if (this->m_MaxTextureSize > 0 &&
			Math::Max(originalWidth, originalHeight) > this->m_MaxTextureSize &&
			texture->IsRenderTarget &&
			texture->Dimension == RHI::RHITextureDimension::Texture2D) {
			if (originalWidth >= originalHeight) {
				scaledHeight = originalHeight * this->m_MaxTextureSize / originalWidth;
				scaledWidth = this->m_MaxTextureSize;
			}
			else {
				scaledWidth = originalWidth * this->m_MaxTextureSize / originalHeight;
				scaledHeight = this->m_MaxTextureSize;
			}
		}

		const char* dataPointer{ static_cast<const char*>(texture->Data->Get_Data()) };

		RHI::RHITextureDesc textureDesc{
			.Extent{.Width{ scaledWidth }, .Height{ scaledHeight }, .Depth{ texture->Extent.Depth } },
			.ArrayCount{ texture->ArrayCount },
			.MipLevelCount{ this->m_GenerateMipmaps && texture->IsRenderTarget && nullptr != passes ? GetMipLevelsNum(scaledWidth, scaledHeight) : texture->MipLevelCount },
			.Format { texture->Format },
			.Dimension{ texture->Dimension },
			.DebugName{ texture->FilePath },
			.IsRenderTarget{ texture->IsRenderTarget },
		};
		texture->Texture = m_Device->CreateTexture(textureDesc);

		commandList->BeginTrackingTextureState(texture->Texture, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);

		if (scaledWidth != originalWidth || scaledHeight != originalHeight) {
			RHI::RHITextureDesc tempTextureDesc{
				.Extent{.Width{ originalWidth }, .Height{ originalHeight }, .Depth{ textureDesc.Extent.Depth } },
				.ArrayCount{ textureDesc.ArrayCount },
				.Format{ textureDesc.Format },
				.Dimension{ textureDesc.Dimension }
			};

			RHI::RefCountPtr<Imp_Texture> tempTexture{ m_Device->CreateTexture(tempTextureDesc) };
			ASSERT(nullptr != tempTexture);

			commandList->BeginTrackingTextureState(tempTexture, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);

			for (Uint32 arraySlice = 0; arraySlice < texture->ArrayCount; ++arraySlice) {
				const TextureSubresourceData& layout{ texture->DataLayout[arraySlice][0] };

				commandList->WriteTexture(tempTexture, arraySlice, 0, dataPointer + layout.DataOffset, layout.RowPitch, layout.DepthPitch);
			}

			RHI::RefCountPtr<Imp_FrameBuffer> framebuffer{ this->m_Device->CreateFrameBuffer(RHI::RHIFrameBufferDescBuilder<APITag>{}
				.AddColorAttachment(texture->Texture)
				.Build()
			) };

			passes->BLITTexture(commandList, framebuffer, tempTexture);
		}
		else {
			for (Uint32 arraySlice = 0; arraySlice < texture->ArrayCount; ++arraySlice)
				for (Uint32 mipLevel = 0; mipLevel < texture->MipLevelCount; ++mipLevel) {
					const TextureSubresourceData& layout = texture->DataLayout[arraySlice][mipLevel];
					commandList->WriteTexture(texture->Texture, arraySlice, mipLevel, dataPointer + layout.DataOffset, layout.RowPitch, layout.DepthPitch);
				}
		}

		texture->Data.reset();

		for (Uint32 mipLevel = texture->MipLevelCount; mipLevel < textureDesc.MipLevelCount; ++mipLevel) {
			RHI::RefCountPtr<Imp_FrameBuffer> framebuffer{ this->m_Device->CreateFrameBuffer(RHI::RHIFrameBufferDescBuilder<APITag>{}
				.AddColorAttachment(RHI::RHIFrameBufferAttachment<APITag>{.Texture{ texture->Texture }, .Subresources{.BaseMipLevel{ mipLevel } } })
				.Build()
			) };

			passes->BLITTexture(commandList, BLITParameters<APITag>{
				.TargetFrameBuffer{ framebuffer },
					.SourceTexture{ texture->Texture },
					.SourceMip{ mipLevel - 1 }
			});
		}

		commandList->SetPermanentTextureState(texture->Texture, RHI::RHIResourceState::ShaderResource);
		commandList->CommitBarriers();

		++this->m_TexturesFinalized;
	}

	template<RHI::APITagConcept APITag>
	inline void TextureCache<APITag>::TextureLoaded(SharedPtr<TextureData<APITag>> texture) {
		{
			LockGuard guard(this->m_TexturesToFinalizeMutex);
			if (texture->MimeType.empty())
				LOG_INFO("F,TODO");
			else
				LOG_INFO("S,TODO");
		}
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<LoadedTexture<APITag>> TextureCache<APITag>::LoadTextureFromMemoryAsync(const SharedPtr<IBlob>& data, const String& name, const String& mimeType, bool sRGB, tf::Executor& executor) {
		auto texture{ MakeShared<TextureData<APITag>>() };

		texture->ForceSRGB = sRGB;
		texture->FilePath = name;
		texture->MimeType = mimeType;

		executor.async(
			[this, &texture, &data, mimeType](void) {
				if (this->FillTextureData(data, texture, "", mimeType)) {
					this->TextureLoaded(texture);

					LockGuard guard{ this->m_TexturesToFinalizeMutex };
					this->m_TexturesToFinalize.push(texture);
				}

				++this->m_TexturesLoaded;
			}
		);

		return texture;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<LoadedTexture<APITag>> TextureCache<APITag>::LoadTextureFromMemoryDeferred(const SharedPtr<IBlob>& data, const String& name, const String& mimeType, bool sRGB) {
		auto texture{ MakeShared<TextureData<APITag>>() };

		texture->ForceSRGB = sRGB;
		texture->FilePath = name;
		texture->MimeType = mimeType;

		if (this->FillTextureData(data, texture, "", mimeType)) {
			this->TextureLoaded(texture);

			LockGuard guard{ this->m_TexturesToFinalizeMutex };
			this->m_TexturesToFinalize.push(texture);
		}

		++this->m_TexturesLoaded;

		return texture;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<LoadedTexture<APITag>> TextureCache<APITag>::LoadTextureFromFile(const Path& path, bool sRGB, CommonRenderPasses<APITag>* passes, Imp_CommandList* commandList) {
		SharedPtr<TextureData<APITag>> texture;

		if (this->FindTextureInCache(path, texture))
			return texture;

		texture->ForceSRGB = sRGB;
		texture->FilePath = path.generic_string();

		auto fileData{ this->ReadTextureFile(path) };
		if (nullptr != fileData)
			if (this->FillTextureData(fileData, texture, path.extension().generic_string(), "")) {
				this->TextureLoaded(texture);

				this->FinalizeTexture(texture, passes, commandList);
			}

		++m_TexturesLoaded;

		return texture;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<LoadedTexture<APITag>> TextureCache<APITag>::LoadTextureFromFileAsync(const Path& path, bool sRGB, tf::Executor& executor) {
		return nullptr;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<LoadedTexture<APITag>> TextureCache<APITag>::LoadTextureFromFileDeferred(const Path& path, bool sRGB) {
		SharedPtr<TextureData<APITag>> texture;
		if (this->FindTextureInCache(path, texture))
			return texture;

		texture->ForceSRGB = sRGB;
		texture->FilePath = path.generic_string();

		if (this->FillTextureData(this->ReadTextureFile(path), texture, path.extension().generic_string(), "")) {
			this->TextureLoaded(texture);

			LockGuard guard{ this->m_TexturesToFinalizeMutex };
			this->m_TexturesToFinalize.push(texture);
		}

		++this->m_TexturesLoaded;

		return texture;
	}

	template<RHI::APITagConcept APITag>
	inline bool TextureCache<APITag>::FillTextureData(const SharedPtr<IBlob>& fileData, const SharedPtr<TextureData<APITag>>& texture, const String& extension, const String& mimeType) const {
		if (extension == ".dds" || extension == ".DDS" || mimeType == "image/vnd-ms.dds") {
			texture->Data = fileData;
			if (!TextureCache<APITag>::LoadDDSTextureFromMemory(*texture)) {
				texture->Data = nullptr;
				LOG_ERROR("Couldn't load DDS texture {1}"/*, texture->Texture.path.c_str()*/);
				return false;
			}
		}
		else if (extension == ".exr" || extension == ".EXR" || mimeType == "image/aces") {
			float* data{ nullptr };
			Int32 width{ 0 };
			Int32 height{ 0 };
			char const* err{ nullptr };

			// This reads only 1 or 4 channel images and duplicates channels
			// Should rewrite w/ lower level EXR functions
			if (LoadEXRFromMemory(&data, &width, &height, static_cast<const Uint8*>(fileData->Get_Data()), fileData->Get_Size(), &err) == TINYEXR_SUCCESS) {
				Uint32 channels{ 4 };
				Uint32 bytesPerPixel{ channels * 4 };

				texture->Data = MakeShared<Blob>(data, bytesPerPixel * width * height);
				texture->Extent.Width = static_cast<Uint32>(width);
				texture->Extent.Height = static_cast<Uint32>(height);
				texture->Format = RHI::RHIFormat::RGBA32_FLOAT;

				texture->OriginalBitsPerPixel = channels * 32;
				texture->IsRenderTarget = true;
				texture->MipLevelCount = 1;
				texture->Dimension = RHI::RHITextureDimension::Texture2D;

				texture->DataLayout.resize(1);
				texture->DataLayout[0].resize(1);
				texture->DataLayout[0][0].DataOffset = 0;
				texture->DataLayout[0][0].RowPitch = static_cast<size_t>(width * bytesPerPixel);
				texture->DataLayout[0][0].DataSize = static_cast<size_t>(width * height * bytesPerPixel);

				return true;
			}
			else {
				LOG_ERROR("Couldn't load EXR texture '%s'"/*, texture->path.c_str()*/);
				return false;
			}
		}
		else {
			Int32
				width{ 0 },
				height{ 0 },
				originalChannels{ 0 };

			if (!stbi_info_from_memory(static_cast<const stbi_uc*>(fileData->Get_Data()), static_cast<Int32>(fileData->Get_Size()), &width, &height, &originalChannels)) {
				LOG_ERROR("Couldn't process image header for texture "/*texture->path.c_str()*/);
				return false;
			}

			bool is_hdr{ static_cast<bool>(stbi_is_hdr_from_memory(static_cast<const stbi_uc*>(fileData->Get_Data()),static_cast<Int32>(fileData->Get_Size()))) };

			Int32 channels{ originalChannels };

			if (originalChannels == 3)
				channels = 4;

			Int32 bytesPerPixel{ channels * (is_hdr ? 4 : 1) };

			unsigned char* bitmap;
			if (is_hdr)
				bitmap = reinterpret_cast<unsigned char*>(stbi_loadf_from_memory(static_cast<const stbi_uc*>(fileData->Get_Data()), static_cast<Int32>(fileData->Get_Size()), &width, &height, &originalChannels, channels));
			else
				bitmap = stbi_load_from_memory(static_cast<const stbi_uc*>(fileData->Get_Data()), static_cast<Int32>(fileData->Get_Size()), &width, &height, &originalChannels, channels);

			if (nullptr == bitmap) {
				LOG_ERROR("Couldn't load generic texture "/*, texture->path.c_str()*/);
				return false;
			}

			texture->OriginalBitsPerPixel = static_cast<Uint32>(originalChannels) * (is_hdr ? 32 : 8);
			texture->Extent.Width = static_cast<Uint32>(width);
			texture->Extent.Height = static_cast<Uint32>(height);
			texture->IsRenderTarget = true;
			/*texture->MipLevelCount = 1;
			texture->Dimension = RHI::RHITextureDimension::Texture2D;*/

			texture->DataLayout.resize(1);
			texture->DataLayout[0].resize(1);
			texture->DataLayout[0][0].DataOffset = 0;
			texture->DataLayout[0][0].RowPitch = static_cast<Uint64>(width * bytesPerPixel);
			texture->DataLayout[0][0].DataSize = static_cast<Uint64>(width * height * bytesPerPixel);

			texture->Data = MakeShared<StbImageBlob>(bitmap);
			bitmap = nullptr; // ownership transferred to the blob

			switch (channels) {
			case 1:
				texture->Format = is_hdr ? RHI::RHIFormat::R32_FLOAT : RHI::RHIFormat::R8_UNORM;
				break;

			case 2:
				texture->Format = is_hdr ? RHI::RHIFormat::RG32_FLOAT : RHI::RHIFormat::RG8_UNORM;
				break;

			case 4:
				texture->Format = is_hdr ?
					RHI::RHIFormat::RGBA32_FLOAT :
					(texture->ForceSRGB ? RHI::RHIFormat::SRGBA8_UNORM : RHI::RHIFormat::RGBA8_UNORM);
				break;

			default:
				texture->Data.reset(); // release the bitmap data

				LOG_ERROR("Unsupported number of components  for texture "/*, channels, texture->path.c_str()*/);
				return false;
			}
		}

		return true;
	}











	struct FormatMapping final {
		RHI::RHIFormat Format;
		DXGI_FORMAT DXGIFormat;
		Uint32 BitsPerPixel;
	};

	constexpr FormatMapping g_FormatMappings[] = {
		FormatMapping{ RHI::RHIFormat::UNKNOWN,              DXGI_FORMAT_UNKNOWN,                0 },
		FormatMapping{ RHI::RHIFormat::R8_UINT,              DXGI_FORMAT_R8_UINT,                8 },
		FormatMapping{ RHI::RHIFormat::R8_SINT,              DXGI_FORMAT_R8_SINT,                8 },
		FormatMapping{ RHI::RHIFormat::R8_UNORM,             DXGI_FORMAT_R8_UNORM,               8 },
		FormatMapping{ RHI::RHIFormat::R8_SNORM,             DXGI_FORMAT_R8_SNORM,               8 },
		FormatMapping{ RHI::RHIFormat::RG8_UINT,             DXGI_FORMAT_R8G8_UINT,              16 },
		FormatMapping{ RHI::RHIFormat::RG8_SINT,             DXGI_FORMAT_R8G8_SINT,              16 },
		FormatMapping{ RHI::RHIFormat::RG8_UNORM,            DXGI_FORMAT_R8G8_UNORM,             16 },
		FormatMapping{ RHI::RHIFormat::RG8_SNORM,            DXGI_FORMAT_R8G8_SNORM,             16 },
		FormatMapping{ RHI::RHIFormat::R16_UINT,             DXGI_FORMAT_R16_UINT,               16 },
		FormatMapping{ RHI::RHIFormat::R16_SINT,             DXGI_FORMAT_R16_SINT,               16 },
		FormatMapping{ RHI::RHIFormat::R16_UNORM,            DXGI_FORMAT_R16_UNORM,              16 },
		FormatMapping{ RHI::RHIFormat::R16_SNORM,            DXGI_FORMAT_R16_SNORM,              16 },
		FormatMapping{ RHI::RHIFormat::R16_FLOAT,            DXGI_FORMAT_R16_FLOAT,              16 },
		FormatMapping{ RHI::RHIFormat::BGRA4_UNORM,          DXGI_FORMAT_B4G4R4A4_UNORM,         16 },
		FormatMapping{ RHI::RHIFormat::B5G6R5_UNORM,         DXGI_FORMAT_B5G6R5_UNORM,           16 },
		FormatMapping{ RHI::RHIFormat::B5G5R5A1_UNORM,       DXGI_FORMAT_B5G5R5A1_UNORM,         16 },
		FormatMapping{ RHI::RHIFormat::RGBA8_UINT,           DXGI_FORMAT_R8G8B8A8_UINT,          32 },
		FormatMapping{ RHI::RHIFormat::RGBA8_SINT,           DXGI_FORMAT_R8G8B8A8_SINT,          32 },
		FormatMapping{ RHI::RHIFormat::RGBA8_UNORM,          DXGI_FORMAT_R8G8B8A8_UNORM,         32 },
		FormatMapping{ RHI::RHIFormat::RGBA8_SNORM,          DXGI_FORMAT_R8G8B8A8_SNORM,         32 },
		FormatMapping{ RHI::RHIFormat::BGRA8_UNORM,          DXGI_FORMAT_B8G8R8A8_UNORM,         32 },
		FormatMapping{ RHI::RHIFormat::SRGBA8_UNORM,         DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,    32 },
		FormatMapping{ RHI::RHIFormat::SBGRA8_UNORM,         DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,    32 },
		FormatMapping{ RHI::RHIFormat::R10G10B10A2_UNORM,    DXGI_FORMAT_R10G10B10A2_UNORM,      32 },
		FormatMapping{ RHI::RHIFormat::R11G11B10_FLOAT,      DXGI_FORMAT_R11G11B10_FLOAT,        32 },
		FormatMapping{ RHI::RHIFormat::RG16_UINT,            DXGI_FORMAT_R16G16_UINT,            32 },
		FormatMapping{ RHI::RHIFormat::RG16_SINT,            DXGI_FORMAT_R16G16_SINT,            32 },
		FormatMapping{ RHI::RHIFormat::RG16_UNORM,           DXGI_FORMAT_R16G16_UNORM,           32 },
		FormatMapping{ RHI::RHIFormat::RG16_SNORM,           DXGI_FORMAT_R16G16_SNORM,           32 },
		FormatMapping{ RHI::RHIFormat::RG16_FLOAT,           DXGI_FORMAT_R16G16_FLOAT,           32 },
		FormatMapping{ RHI::RHIFormat::R32_UINT,             DXGI_FORMAT_R32_UINT,               32 },
		FormatMapping{ RHI::RHIFormat::R32_SINT,             DXGI_FORMAT_R32_SINT,               32 },
		FormatMapping{ RHI::RHIFormat::R32_FLOAT,            DXGI_FORMAT_R32_FLOAT,              32 },
		FormatMapping{ RHI::RHIFormat::RGBA16_UINT,          DXGI_FORMAT_R16G16B16A16_UINT,      64 },
		FormatMapping{ RHI::RHIFormat::RGBA16_SINT,          DXGI_FORMAT_R16G16B16A16_SINT,      64 },
		FormatMapping{ RHI::RHIFormat::RGBA16_FLOAT,         DXGI_FORMAT_R16G16B16A16_FLOAT,     64 },
		FormatMapping{ RHI::RHIFormat::RGBA16_UNORM,         DXGI_FORMAT_R16G16B16A16_UNORM,     64 },
		FormatMapping{ RHI::RHIFormat::RGBA16_SNORM,         DXGI_FORMAT_R16G16B16A16_SNORM,     64 },
		FormatMapping{ RHI::RHIFormat::RG32_UINT,            DXGI_FORMAT_R32G32_UINT,            64 },
		FormatMapping{ RHI::RHIFormat::RG32_SINT,            DXGI_FORMAT_R32G32_SINT,            64 },
		FormatMapping{ RHI::RHIFormat::RG32_FLOAT,           DXGI_FORMAT_R32G32_FLOAT,           64 },
		FormatMapping{ RHI::RHIFormat::RGB32_UINT,           DXGI_FORMAT_R32G32B32_UINT,         96 },
		FormatMapping{ RHI::RHIFormat::RGB32_SINT,           DXGI_FORMAT_R32G32B32_SINT,         96 },
		FormatMapping{ RHI::RHIFormat::RGB32_FLOAT,          DXGI_FORMAT_R32G32B32_FLOAT,        96 },
		FormatMapping{ RHI::RHIFormat::RGBA32_UINT,          DXGI_FORMAT_R32G32B32A32_UINT,      128 },
		FormatMapping{ RHI::RHIFormat::RGBA32_SINT,          DXGI_FORMAT_R32G32B32A32_SINT,      128 },
		FormatMapping{ RHI::RHIFormat::RGBA32_FLOAT,         DXGI_FORMAT_R32G32B32A32_FLOAT,     128 },
		FormatMapping{ RHI::RHIFormat::D16,                  DXGI_FORMAT_R16_UNORM,              16 },
		FormatMapping{ RHI::RHIFormat::D24S8,                DXGI_FORMAT_R24_UNORM_X8_TYPELESS,  32 },
		FormatMapping{ RHI::RHIFormat::X24G8_UINT,           DXGI_FORMAT_X24_TYPELESS_G8_UINT,   32 },
		FormatMapping{ RHI::RHIFormat::D32,                  DXGI_FORMAT_R32_FLOAT,              32 },
		FormatMapping{ RHI::RHIFormat::D32S8,                DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, 64 },
		FormatMapping{ RHI::RHIFormat::X32G8_UINT,           DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,  64 },
		FormatMapping{ RHI::RHIFormat::BC1_UNORM,            DXGI_FORMAT_BC1_UNORM,              4 },
		FormatMapping{ RHI::RHIFormat::BC1_UNORM_SRGB,       DXGI_FORMAT_BC1_UNORM_SRGB,         4 },
		FormatMapping{ RHI::RHIFormat::BC2_UNORM,            DXGI_FORMAT_BC2_UNORM,              8 },
		FormatMapping{ RHI::RHIFormat::BC2_UNORM_SRGB,       DXGI_FORMAT_BC2_UNORM_SRGB,         8 },
		FormatMapping{ RHI::RHIFormat::BC3_UNORM,            DXGI_FORMAT_BC3_UNORM,              8 },
		FormatMapping{ RHI::RHIFormat::BC3_UNORM_SRGB,       DXGI_FORMAT_BC3_UNORM_SRGB,         8 },
		FormatMapping{ RHI::RHIFormat::BC4_UNORM,            DXGI_FORMAT_BC4_UNORM,              4 },
		FormatMapping{ RHI::RHIFormat::BC4_SNORM,            DXGI_FORMAT_BC4_SNORM,              4 },
		FormatMapping{ RHI::RHIFormat::BC5_UNORM,            DXGI_FORMAT_BC5_UNORM,              8 },
		FormatMapping{ RHI::RHIFormat::BC5_SNORM,            DXGI_FORMAT_BC5_SNORM,              8 },
		FormatMapping{ RHI::RHIFormat::BC6H_UFLOAT,          DXGI_FORMAT_BC6H_UF16,              8 },
		FormatMapping{ RHI::RHIFormat::BC6H_SFLOAT,          DXGI_FORMAT_BC6H_SF16,              8 },
		FormatMapping{ RHI::RHIFormat::BC7_UNORM,            DXGI_FORMAT_BC7_UNORM,              8 },
		FormatMapping{ RHI::RHIFormat::BC7_UNORM_SRGB,       DXGI_FORMAT_BC7_UNORM_SRGB,         8 },
	};

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

	RHI::RHIFormat ConvertDDSFormat(const DirectX::DDS_PIXELFORMAT& ddpf, bool forceSRGB) {
		if (ddpf.flags & DDS_RGB) {
			// Note that sRGB formats are written using the "DX10" extended header

			switch (ddpf.RGBBitCount) {
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
					return forceSRGB ? RHI::RHIFormat::RGBA8_UNORM : RHI::RHIFormat::SRGBA8_UNORM;

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
					return forceSRGB ? RHI::RHIFormat::BGRA8_UNORM : RHI::RHIFormat::SBGRA8_UNORM;

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
					return forceSRGB ? RHI::RHIFormat::BGRA8_UNORM : RHI::RHIFormat::SBGRA8_UNORM; // actually BGRX8, but there's no such format in NVRHI

				// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

				// Note that many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assume
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

				// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
					return RHI::RHIFormat::R10G10B10A2_UNORM;

				// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
					return RHI::RHIFormat::RG16_UNORM;

				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
					return RHI::RHIFormat::R32_FLOAT;// Only 32-bit color channel format in D3D9 was R32F
				break;

			case 24: break;// No 24bpp DXGI formats aka D3DFMT_R8G8B8

			case 16:
				if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
					return RHI::RHIFormat::B5G5R5A1_UNORM;
				if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
					return RHI::RHIFormat::B5G6R5_UNORM;

				// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

				if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
					return RHI::RHIFormat::BGRA4_UNORM;

				// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

				// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
				break;
			}
		}
		else if (ddpf.flags & DDS_LUMINANCE) {
			if (8 == ddpf.RGBBitCount) {
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
					return RHI::RHIFormat::R8_UNORM;

				// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
					return RHI::RHIFormat::RG8_UNORM;
			}

			if (16 == ddpf.RGBBitCount) {
				if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
					return RHI::RHIFormat::R16_UNORM;
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
					return RHI::RHIFormat::RG8_UNORM;
			}
		}
		else if (ddpf.flags & DDS_ALPHA) {
			if (8 == ddpf.RGBBitCount)
				return RHI::RHIFormat::R8_UNORM; // we don't support A8 in RHI
		}
		else if (ddpf.flags & DDS_BUMPDUDV) {
			if (16 == ddpf.RGBBitCount) {
				if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
					return RHI::RHIFormat::RG8_SNORM;
			}

			if (32 == ddpf.RGBBitCount) {
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
					return RHI::RHIFormat::RGBA8_SNORM;
				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
					return RHI::RHIFormat::RG16_SNORM;

				// No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
			}
		}
		else if (ddpf.flags & DDS_FOURCC) {
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
				return forceSRGB ? RHI::RHIFormat::BC1_UNORM_SRGB : RHI::RHIFormat::BC1_UNORM;
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
				return forceSRGB ? RHI::RHIFormat::BC2_UNORM_SRGB : RHI::RHIFormat::BC2_UNORM;
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
				return forceSRGB ? RHI::RHIFormat::BC3_UNORM_SRGB : RHI::RHIFormat::BC3_UNORM;

			// While pre-multiplied alpha isn't directly supported by the DXGI formats,
			// they are basically the same as these BC formats so they can be mapped
			if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
				return RHI::RHIFormat::BC2_UNORM;
			if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
				return RHI::RHIFormat::BC3_UNORM;

			if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
				return RHI::RHIFormat::BC4_UNORM;
			if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
				return RHI::RHIFormat::BC4_UNORM;
			if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
				return RHI::RHIFormat::BC4_SNORM;

			if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
				return RHI::RHIFormat::BC5_UNORM;
			if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
				return RHI::RHIFormat::BC5_UNORM;
			if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
				return RHI::RHIFormat::BC5_SNORM;

			// BC6H and BC7 are written using the "DX10" extended header
			/*
			if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
			{
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
			}
			if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
			{
				return DXGI_FORMAT_G8R8_G8B8_UNORM;
			}

			if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
			{
				return DXGI_FORMAT_YUY2;
			}
			*/

			// Check for D3DFORMAT enums being set here
			switch (ddpf.fourCC) {
			case 36:return RHI::RHIFormat::RGBA16_UNORM; // D3DFMT_A16B16G16R16
			case 110: return RHI::RHIFormat::RGBA16_SNORM;// D3DFMT_Q16W16V16U16
			case 111:return RHI::RHIFormat::R16_FLOAT; // D3DFMT_R16F
			case 112: return RHI::RHIFormat::RG16_FLOAT;// D3DFMT_G16R16F
			case 113: return RHI::RHIFormat::RGBA16_FLOAT;// D3DFMT_A16B16G16R16F
			case 114: return RHI::RHIFormat::R32_FLOAT;// D3DFMT_R32F
			case 115: return RHI::RHIFormat::RG32_FLOAT;// D3DFMT_G32R32F
			case 116:return RHI::RHIFormat::RGBA32_FLOAT; // D3DFMT_A32B32G32R32F
			}
		}

		return RHI::RHIFormat::UNKNOWN;
	}

	Uint32 BitsPerPixel(RHI::RHIFormat format) {
		const FormatMapping& mapping = g_FormatMappings[Tounderlying(format)];
		ASSERT(mapping.Format == format);

		return mapping.BitsPerPixel;
	}

	void GetSurfaceInfo(Uint64 width, Uint64 height, RHI::RHIFormat fmt, Uint32 bitsPerPixel, Uint64* outNumBytes, Uint64* outRowBytes, Uint64* outNumRows) {
		Uint64 numBytes{ 0 };
		Uint64 rowBytes{ 0 };
		Uint64 numRows{ 0 };

		bool bc{ false };
		Uint64 bpe{ 0 };
		switch (fmt) {
			using enum RHI::RHIFormat;
		case BC1_UNORM:case BC1_UNORM_SRGB:case BC4_UNORM:case BC4_SNORM:
			bc = true;
			bpe = 8;
			break;

		case BC2_UNORM:case BC2_UNORM_SRGB:case BC3_UNORM:case BC3_UNORM_SRGB:case BC5_UNORM:case BC5_SNORM:case BC6H_UFLOAT:case BC6H_SFLOAT:case BC7_UNORM:case BC7_UNORM_SRGB:
			bc = true;
			bpe = 16;
			break;

		default:break;
		}

		if (bc) {
			Uint64 numBlocksWide{ 0 };
			if (width > 0)
				numBlocksWide = Math::Max<Uint64>(1, (width + 3) / 4);
			Uint64 numBlocksHigh{ 0 };
			if (height > 0)
				numBlocksHigh = Math::Max<Uint64>(1, (height + 3) / 4);
			rowBytes = numBlocksWide * bpe;
			numRows = numBlocksHigh;
			numBytes = rowBytes * numBlocksHigh;
		}
		else {
			rowBytes = (width * bitsPerPixel + 7) / 8; // round up to nearest byte
			numRows = height;
			numBytes = rowBytes * height;
		}

		if (nullptr != outNumBytes)
			*outNumBytes = numBytes;
		if (nullptr != outRowBytes)
			*outRowBytes = rowBytes;
		if (nullptr != outNumRows)
			*outNumRows = numRows;
	}

#undef ISBITMASK

	TextureAlphaMode GetAlphaMode(const DirectX::DDS_HEADER* header) {
		if (header->ddspf.flags & DDS_FOURCC) {
			if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC) {
				auto d3d10ext{ reinterpret_cast<const DirectX::DDS_HEADER_DXT10*>(reinterpret_cast<const Uint8*>(header) + sizeof(DirectX::DDS_HEADER)) };
				auto mode{ static_cast<TextureAlphaMode>(d3d10ext->miscFlags2 & DirectX::DDS_MISC_FLAGS2_ALPHA_MODE_MASK) };
				switch (mode) {
					using enum TextureAlphaMode;
				case STRAIGHT:case PREMULTIPLIED:case OPAQUE:case CUSTOM:return mode;
				default:break;
				}
			}
			else if ((MAKEFOURCC('D', 'X', 'T', '2') == header->ddspf.fourCC)
				|| (MAKEFOURCC('D', 'X', 'T', '4') == header->ddspf.fourCC))
				return TextureAlphaMode::PREMULTIPLIED;
		}

		return TextureAlphaMode::UNKNOWN;
	}

	template<RHI::APITagConcept APITag>
	Uint64 FillTextureInfoOffsets(TextureData<APITag>& textureInfo, Uint64 dataSize, Int64 dataOffset) {
		textureInfo.OriginalBitsPerPixel = BitsPerPixel(textureInfo.Format);

		textureInfo.DataLayout.resize(textureInfo.ArrayCount);
		for (Uint32 arraySlice = 0; arraySlice < textureInfo.ArrayCount; arraySlice++) {
			Uint64 w = textureInfo.Extent.Width;
			Uint64 h = textureInfo.Extent.Height;
			Uint64 d = textureInfo.Extent.Depth;

			Vector<TextureSubresourceData>& sliceData{ textureInfo.DataLayout[arraySlice] };
			sliceData.resize(textureInfo.MipLevelCount);

			for (Uint32 mipLevel = 0; mipLevel < textureInfo.MipLevelCount; mipLevel++) {
				Uint64 NumBytes{ 0 };
				Uint64 RowBytes{ 0 };
				Uint64 NumRows{ 0 };
				GetSurfaceInfo(w, h, textureInfo.Format, textureInfo.OriginalBitsPerPixel, &NumBytes, &RowBytes, &NumRows);

				TextureSubresourceData& levelData{ sliceData[mipLevel] };
				levelData.DataOffset = dataOffset;
				levelData.DataSize = NumBytes;
				levelData.RowPitch = RowBytes;
				levelData.DepthPitch = RowBytes * NumRows;

				dataOffset += NumBytes * d;

				if (dataSize > 0 && dataOffset > static_cast<Int64>(dataSize))
					return 0;

				w = w >> 1;
				h = h >> 1;
				d = d >> 1;

				if (w == 0) w = 1;
				if (h == 0) h = 1;
				if (d == 0) d = 1;
			}
		}

		return dataOffset;
	}


	template<RHI::APITagConcept APITag>
	inline bool TextureCache<APITag>::LoadDDSTextureFromMemory(TextureData<APITag>& textureInfo) {
		if (textureInfo.Data->Get_Size() < sizeof(Uint32) + sizeof(DirectX::DDS_HEADER))
			return false;

		auto dwMagicNumber{ *reinterpret_cast<const Uint32*>(textureInfo.Data->Get_Data()) };
		if (dwMagicNumber != DirectX::DDS_MAGIC)
			return false;

		auto header{ reinterpret_cast<const DirectX::DDS_HEADER*>(static_cast<const char*>(textureInfo.Data->Get_Data()) + sizeof(Uint32)) };

		// Verify header to validate DDS file
		if (header->size != sizeof(DirectX::DDS_HEADER) ||
			header->ddspf.size != sizeof(DirectX::DDS_PIXELFORMAT))
			return false;

		// Check for DX10 extension
		bool bDXT10Header{ false };
		if ((header->ddspf.flags & DDS_FOURCC) &&
			(MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)) {
			// Must be long enough for both headers and magic value
			if (textureInfo.Data->Get_Size() < (sizeof(DirectX::DDS_HEADER) + sizeof(Uint32) + sizeof(DirectX::DDS_HEADER_DXT10)))
				return false;

			bDXT10Header = true;
		}

		Int64 dataOffset{
			sizeof(Uint32) +
			sizeof(DirectX::DDS_HEADER) +
			(bDXT10Header ? sizeof(DirectX::DDS_HEADER_DXT10) : 0)
		};

		textureInfo.Extent.Width = header->width;
		textureInfo.Extent.Height = header->height;
		textureInfo.MipLevelCount = header->mipMapCount ? header->mipMapCount : 1;
		textureInfo.Extent.Depth = 1;
		textureInfo.ArrayCount = 1;
		textureInfo.AlphaMode = GetAlphaMode(header);

		if ((header->ddspf.flags & DDS_FOURCC) &&
			(MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)) {
			auto d3d10ext{ reinterpret_cast<const DirectX::DDS_HEADER_DXT10*>(reinterpret_cast<const char*>(header) + sizeof(DirectX::DDS_HEADER)) };

			if (d3d10ext->arraySize == 0)
				return false;

			for (const auto& [format, dxgiformat, _bitper] : g_FormatMappings)
				if (dxgiformat == d3d10ext->dxgiFormat) {
					textureInfo.Format = format;
					break;
				}

			if (textureInfo.Format == RHI::RHIFormat::UNKNOWN)
				return false;

			// Apply the forceSRGB flag and promote various compatible formats to sRGB
			if (textureInfo.ForceSRGB) {
				switch (textureInfo.Format) {
				case(RHI::RHIFormat::RGBA8_UNORM):
					textureInfo.Format = RHI::RHIFormat::SRGBA8_UNORM;
					break;

				case(RHI::RHIFormat::BGRA8_UNORM):
					textureInfo.Format = RHI::RHIFormat::SBGRA8_UNORM;
					break;

				case(RHI::RHIFormat::BC1_UNORM):
					textureInfo.Format = RHI::RHIFormat::BC1_UNORM_SRGB;
					break;

				case(RHI::RHIFormat::BC2_UNORM):
					textureInfo.Format = RHI::RHIFormat::BC2_UNORM_SRGB;
					break;

				case(RHI::RHIFormat::BC3_UNORM):
					textureInfo.Format = RHI::RHIFormat::BC3_UNORM_SRGB;
					break;

				case(RHI::RHIFormat::BC7_UNORM):
					textureInfo.Format = RHI::RHIFormat::BC7_UNORM_SRGB;
					break;

				default:
					break;
				}
			}

			switch (d3d10ext->resourceDimension) {
			case DirectX::DDS_DIMENSION_TEXTURE1D:
				// D3DX writes 1D textures with a fixed Height of 1
				if ((header->flags & DDS_HEIGHT) && textureInfo.Extent.Height != 1)
					return false;
				textureInfo.Extent.Height = 1;
				textureInfo.Dimension = d3d10ext->arraySize > 1 ? RHI::RHITextureDimension::Texture1DArray : RHI::RHITextureDimension::Texture1D;
				break;

			case DirectX::DDS_DIMENSION_TEXTURE2D:
				if (d3d10ext->miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE) {
					textureInfo.ArrayCount = d3d10ext->arraySize * 6;
					textureInfo.Dimension = d3d10ext->arraySize > 1 ? RHI::RHITextureDimension::TextureCubeArray : RHI::RHITextureDimension::TextureCube;
				}
				else {
					textureInfo.ArrayCount = d3d10ext->arraySize;
					textureInfo.Dimension = d3d10ext->arraySize > 1 ? RHI::RHITextureDimension::Texture2DArray : RHI::RHITextureDimension::Texture2D;
				}

				break;

			case DirectX::DDS_DIMENSION_TEXTURE3D:
				if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
					return false;
				textureInfo.Extent.Depth = header->depth;
				textureInfo.Dimension = RHI::RHITextureDimension::Texture3D;
				break;

			default:
				return false;
			}
		}
		else {
			textureInfo.Format = ConvertDDSFormat(header->ddspf, textureInfo.ForceSRGB);

			if (textureInfo.Format == RHI::RHIFormat::UNKNOWN)
				return false;

			if (header->flags & DDS_HEADER_FLAGS_VOLUME) {
				textureInfo.Extent.Depth = header->depth;
				textureInfo.Dimension = RHI::RHITextureDimension::Texture3D;
			}
			else {
				if (header->caps2 & DDS_CUBEMAP) {
					// We require all six faces to be defined
					if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
						return false;

					textureInfo.ArrayCount = 6;
					textureInfo.Dimension = RHI::RHITextureDimension::TextureCube;
				}
				else
					textureInfo.Dimension = RHI::RHITextureDimension::Texture2D;
			}
		}

		if (FillTextureInfoOffsets(textureInfo, textureInfo.Data->Get_Size(), dataOffset) == 0)
			return false;

		return true;
	}

	template<RHI::APITagConcept APITag>
	inline auto TextureCache<APITag>::CreateDDSTextureFromMemory(Imp_Device* device, Imp_CommandList* commandList, SharedPtr<IBlob> data, const char* debugName, bool forceSRGB) -> RHI::RefCountPtr<Imp_Texture> {
		TextureData<APITag> info{
			.Data { data },
			.ForceSRGB { forceSRGB },
		};

		if (!LoadDDSTextureFromMemory(info))
			return nullptr;

		RHI::RHITextureDesc desc{
			.Extent { info.Extent },
			.ArrayCount { info.ArrayCount },
			.MipLevelCount { info.MipLevelCount },
			.Format { info.Format },
			.Dimension { info.Dimension },
			.DebugName { debugName },
		};

		RHI::RefCountPtr<Imp_Texture> texture{ device->CreateTexture(desc) };

		if (nullptr == texture)
			return nullptr;

		commandList->BeginTrackingTextureState(texture, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);

		for (Uint32 arraySlice = 0; arraySlice < info.ArrayCount; ++arraySlice)
			for (Uint32 mipLevel = 0; mipLevel < info.MipLevelCount; ++mipLevel) {
				const TextureSubresourceData& layout{ info.DataLayout[arraySlice][mipLevel] };

				commandList->WriteTexture(texture, arraySlice, mipLevel, static_cast<const char*>(info.Data->Get_Data()) + layout.DataOffset, layout.RowPitch);
			}

		commandList->SetPermanentTextureState(texture, RHI::RHIResourceState::ShaderResource);
		commandList->CommitBarriers();

		return texture;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<IBlob> TextureCache<APITag>::SaveStagingTextureAsDDS(Imp_Device* device, Imp_StagingTexture* stagingTexture) {
		const RHI::RHITextureDesc& textureDesc{ stagingTexture->Get_Desc() };

		DirectX::DDS_HEADER header{
			.size { sizeof(DirectX::DDS_HEADER) },
			.flags { DDS_HEADER_FLAGS_TEXTURE },
			.width { textureDesc.Extent.Width },
			.height { textureDesc.Extent.Height },
			.depth { textureDesc.Extent.Depth },
			.mipMapCount { textureDesc.MipLevelCount },
			.ddspf {
				.size { sizeof(DirectX::DDS_PIXELFORMAT) },
				.flags { DDS_FOURCC },
				.fourCC { MAKEFOURCC('D', 'X', 'T', '1') } // This will be overridden by the DX10 header
			}
		};
		DirectX::DDS_HEADER_DXT10 dx10header{};

		switch (textureDesc.Dimension) {
			using enum RHI::RHITextureDimension;
		case Texture1D:
		case Texture1DArray:
			dx10header.resourceDimension = DirectX::DDS_DIMENSION_TEXTURE1D;
			break;

		case Texture2D:case Texture2DArray:case TextureCube:case TextureCubeArray:
			dx10header.resourceDimension = DirectX::DDS_DIMENSION_TEXTURE2D;
			break;

		case Texture3D:
			// Unsupported
			return nullptr;
			/*header.flags |= DirectX::DDS_HEADER_FLAGS_VOLUME;
			dx10header.resourceDimension = DirectX::DDS_DIMENSION_TEXTURE3D;
			break;*/

		case Texture2DMS:case Texture2DMSArray:case Unknown:
			// Unsupported
			return nullptr;
		}

		dx10header.arraySize = textureDesc.ArrayCount;
		if (textureDesc.Dimension == RHI::RHITextureDimension::TextureCube || textureDesc.Dimension == RHI::RHITextureDimension::TextureCubeArray) {
			dx10header.arraySize /= 6;
			dx10header.miscFlag |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		for (const auto& [format, dxgiformat, _p] : g_FormatMappings)
			if (format == textureDesc.Format) {
				dx10header.dxgiFormat = dxgiformat;
				break;
			}

		if (dx10header.dxgiFormat == DXGI_FORMAT_UNKNOWN)
			return nullptr;// Unsupported

		TextureData<APITag> textureInfo{
			.Format { textureDesc.Format },
			.Extent { textureDesc.Extent },
			.ArrayCount { textureDesc.ArrayCount },
			.MipLevelCount { textureDesc.MipLevelCount },
			.Dimension { textureDesc.Dimension }
		};

		Int64 dataOffset{
			sizeof(Uint32) +
			sizeof(DirectX::DDS_HEADER) +
			sizeof(DirectX::DDS_HEADER_DXT10)
		};

		Uint64 dataSize{ FillTextureInfoOffsets(textureInfo, 0, dataOffset) };

		char* data{ reinterpret_cast<char*>(malloc(dataSize)) };
		ASSERT(nullptr != data);

		*reinterpret_cast<Uint32*>(data) = DirectX::DDS_MAGIC;
		*reinterpret_cast<DirectX::DDS_HEADER*>(data + sizeof(Uint32)) = header;
		*reinterpret_cast<DirectX::DDS_HEADER_DXT10*>(data + sizeof(Uint32) + sizeof(DirectX::DDS_HEADER)) = dx10header;


		for (Uint32 arraySlice = 0; arraySlice < textureDesc.ArrayCount; ++arraySlice) {
			Uint32 width{ textureInfo.Extent.Width };
			Uint32 height{ textureInfo.Extent.Height };

			for (Uint32 mipLevel = 0; mipLevel < textureDesc.MipLevelCount; ++mipLevel) {
				RHI::RHITextureSlice slice{
					.MipLevel { mipLevel },
					.ArraySlice { arraySlice }
				};

				Uint64 rowPitch{ 0 };
				const char* sliceData{ reinterpret_cast<const char*>(device->MapStagingTexture(stagingTexture, slice, RHI::RHICPUAccessMode::Read, &rowPitch)) };

				const TextureSubresourceData& subresourceData{ textureInfo.DataLayout[arraySlice][mipLevel] };

				for (Uint32 row = 0; row < height; ++row) {
					Int64 destOffset{ subresourceData.DataOffset + subresourceData.RowPitch * row };
					Int64 srcOffset{ rowPitch * row };

					ASSERT(destOffset + subresourceData.RowPitch <= dataSize);

					memcpy(data + destOffset, sliceData + srcOffset, subresourceData.RowPitch);
				}

				device->UnmapStagingTexture(stagingTexture);

				width = width >> 1;
				height = height >> 1;

				if (width == 0)
					width = 1;
				if (height == 0)
					height = 1;
			}
		}

		return MakeShared<Blob>(data, dataSize);
	}

}