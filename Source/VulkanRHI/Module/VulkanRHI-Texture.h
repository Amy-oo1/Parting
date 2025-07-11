#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Heap)
PARTING_SUBMODE_IMPORT(Buffer)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"
#include "VulkanRHI/Module/VulkanRHI-Format.h"
#include "VulkanRHI/Module/VulkanRHI-Heap.h"
#include "VulkanRHI/Module/VulkanRHI-Buffer.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	struct TextureSubresourceView final {
		Texture& Texture;
		RHITextureSubresourceSet Subresource;

		vk::ImageView View{ nullptr };
		vk::ImageSubresourceRange SubresourceRange;

		bool operator==(const TextureSubresourceView&) const noexcept = default;
		bool operator!=(const TextureSubresourceView&) const noexcept = default;
	};

	inline vk::ImageUsageFlags PickImageUsage(const RHITextureDesc& d) {
		const auto& formatInfo{ Get_RHIFormatInfo(d.Format) };

		vk::ImageUsageFlags ret{
			vk::ImageUsageFlagBits::eTransferSrc |
			vk::ImageUsageFlagBits::eTransferDst
		};

		if (d.IsShaderResource)
			ret |= vk::ImageUsageFlagBits::eSampled;

		if (d.IsRenderTarget) {
			if (formatInfo.HasDepth || formatInfo.HasStencil)
				ret |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
			else
				ret |= vk::ImageUsageFlagBits::eColorAttachment;
		}

		if (d.IsUAV)
			ret |= vk::ImageUsageFlagBits::eStorage;

		return ret;
	}

	inline vk::ImageCreateFlags PickImageFlags(const RHITextureDesc& d) {
		vk::ImageCreateFlags flags{};

		if (d.Dimension == RHITextureDimension::TextureCube ||
			d.Dimension == RHITextureDimension::TextureCubeArray)
			flags |= vk::ImageCreateFlagBits::eCubeCompatible;

		if (d.IsTypeless)
			flags |= vk::ImageCreateFlagBits::eMutableFormat | vk::ImageCreateFlagBits::eExtendedUsage;

		if (d.IsTiled)
			flags |= vk::ImageCreateFlagBits::eSparseBinding | vk::ImageCreateFlagBits::eSparseResidency;

		return flags;
	}

	vk::ImageType TextureDimensionToImageType(RHITextureDimension dimension) {
		switch (dimension) {
			using enum RHITextureDimension;
		case Texture1D:case Texture1DArray:return vk::ImageType::e1D;
		case Texture2D:case Texture2DArray:case TextureCube:case TextureCubeArray:case Texture2DMS:case Texture2DMSArray:return vk::ImageType::e2D;
		case Texture3D:return vk::ImageType::e3D;
		case Unknown:default:ASSERT(false); return vk::ImageType::e2D;
		}
	}

	//Misc

	vk::ImageCreateInfo ConvertTextureDesc(const RHITextureDesc& desc) {
		return vk::ImageCreateInfo{}
			.setImageType(TextureDimensionToImageType(desc.Dimension))
			.setExtent(vk::Extent3D{ desc.Extent.Width, desc.Extent.Height, desc.Extent.Depth })
			.setMipLevels(desc.MipLevelCount)
			.setArrayLayers(desc.ArrayCount)
			.setFormat(vk::Format{ ConvertFormat(desc.Format) })
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(PickImageUsage(desc))
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSamples(ConvertSampleCount(desc.SampleCount))
			.setFlags(PickImageFlags(desc));
	}

	class Texture final : public RHITexture<Texture>, public MemoryResource {
		friend class RHIResource<Texture>;
		friend class RHITexture<Texture>;

		friend class CommandList;
		friend class Device;
	public:
		enum class TextureSubresourceViewType {
			AllAspects,
			DepthOnly,
			StencilOnly
		};

		using SubresourceViewKey = Tuple<RHITextureSubresourceSet, TextureSubresourceViewType, RHITextureDimension, RHIFormat, vk::ImageUsageFlags>;

		struct SubresourceViewKeyHash final {
			Uint64 operator()(const SubresourceViewKey& s) const noexcept {
				const auto& [subresources, viewType, dimension, format, usage] { s };

				Uint64 hash{ 0 };

				hash = HashCombine(hash, Hash<Uint32>{}(subresources.BaseMipLevel));
				hash = HashCombine(hash, Hash<Uint32>{}(subresources.MipLevelCount));
				hash = HashCombine(hash, Hash<Uint32>{}(subresources.BaseArraySlice));
				hash = HashCombine(hash, Hash<Uint32>{}(subresources.ArraySliceCount));
				hash = HashCombine(hash, Hash<TextureSubresourceViewType>{}(viewType));
				hash = HashCombine(hash, Hash<RHITextureDimension>{}(dimension));
				hash = HashCombine(hash, Hash<RHIFormat>{}(format));
				hash = HashCombine(hash, Hash<vk::ImageUsageFlags::MaskType>{}(static_cast<vk::ImageUsageFlags::MaskType>(usage)));

				return hash;
			}
		};

		Texture(const Context& context, VulkanAllocator& allocator, RHITextureDesc desc, vk::ImageCreateInfo info) :
			RHITexture<Texture>{},
			MemoryResource{},
			m_Context{ context },
			m_Allocator{ allocator },
			m_Desc{ desc },
			m_ImageInfo{ info }{
		}

		~Texture(void);

		// returns a subresource view for an arbitrary range of mip levels and array layers.
		// 'viewtype' only matters when asking for a depthstencil view; in situations where only depth or stencil can be bound
		// (such as an SRV with ImageLayout::eShaderReadOnlyOptimal), but not both, then this specifies which of the two aspect bits is to be set.
		TextureSubresourceView& Get_SubresourceView(const RHITextureSubresourceSet& subresources, RHITextureDimension dimension, RHIFormat format, vk::ImageUsageFlags usage, TextureSubresourceViewType viewtype = TextureSubresourceViewType::AllAspects);

		Uint32 Get_NumSubresources(void) const { return this->m_Desc.MipLevelCount * this->m_Desc.ArrayCount; }

		Uint32 Get_SubresourceIndex(Uint32 mipLevel, Uint32 arrayLayer) const { return mipLevel * this->m_Desc.ArrayCount + arrayLayer; }

	private:
		const Context& m_Context;
		VulkanAllocator& m_Allocator;

		RHITextureDesc m_Desc;
		RHITextureStateExtension<VulkanTag> m_StateExtension{ .DescRef{ this->m_Desc }, .ParentTextureRef{ this } };

		RefCountPtr<Heap> m_Heap;

		vk::ImageCreateInfo m_ImageInfo;
		vk::Image m_Image;

		// contains subresource views for this texture
		// note that we only create the views that the app uses, and that multiple views may map to the same subresources
		UnorderedMap<SubresourceViewKey, TextureSubresourceView, Texture::SubresourceViewKeyHash> m_SubresourceViews;

		Mutex m_Mutex;

		static constexpr Uint32 TileByteSize{ 65536 };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHITextureDesc& Imp_Get_Desc(void)const { return this->m_Desc; }

		RHIObject Imp_GetNativeView(RHIObjectType objectType, RHIFormat format, RHITextureSubresourceSet subresources, RHITextureDimension dimension, bool isReadOnlyDSV);
	};

	//Misc

	Texture::TextureSubresourceViewType Get_TextureViewType(RHIFormat bindingFormat, RHIFormat textureFormat) {
		RHIFormat format{ (RHIFormat::UNKNOWN == bindingFormat) ? textureFormat : bindingFormat };

		const auto& formatInfo{ Get_RHIFormatInfo(format) };

		if (formatInfo.HasDepth)
			return Texture::TextureSubresourceViewType::DepthOnly;
		else if (formatInfo.HasStencil)
			return Texture::TextureSubresourceViewType::StencilOnly;
		else
			return Texture::TextureSubresourceViewType::AllAspects;
	}

	//Src

	Texture::~Texture(void) {
		for (auto& viewIter : this->m_SubresourceViews) {
			auto& view{ viewIter.second.View };
			this->m_Context.Device.destroyImageView(view, this->m_Context.AllocationCallbacks);
			view = nullptr;
		}
		this->m_SubresourceViews.clear();

		if (this->MemoryResource::Managed) {
			if (nullptr != this->m_Image) {
				this->m_Context.Device.destroyImage(this->m_Image, this->m_Context.AllocationCallbacks);
				this->m_Image = nullptr;
			}

			if (nullptr != this->MemoryResource::Memory) {
				this->m_Allocator.FreeMemory(static_cast<MemoryResource*>(this));
				this->MemoryResource::Memory = nullptr;
			}
		}
	}

	// infer aspect flags for a given image format
	vk::ImageAspectFlags GuessImageAspectFlags(vk::Format format) {
		switch (format) {
		case vk::Format::eD16Unorm:case vk::Format::eX8D24UnormPack32:case vk::Format::eD32Sfloat:
			return vk::ImageAspectFlagBits::eDepth;

		case vk::Format::eS8Uint:
			return vk::ImageAspectFlagBits::eStencil;

		case vk::Format::eD16UnormS8Uint:case vk::Format::eD24UnormS8Uint:case vk::Format::eD32SfloatS8Uint:
			return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

		default:
			return vk::ImageAspectFlagBits::eColor;
		}
	}

	// a subresource usually shouldn't have both stencil and depth aspect flag bits set; this enforces that depending on viewType param
	vk::ImageAspectFlags GuessSubresourceImageAspectFlags(vk::Format format, Texture::TextureSubresourceViewType viewType) {
		vk::ImageAspectFlags flags{ GuessImageAspectFlags(format) };

		if ((flags & (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)) == (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)) {
			if (viewType == Texture::TextureSubresourceViewType::DepthOnly)
				flags = flags & (~vk::ImageAspectFlagBits::eStencil);
			else if (viewType == Texture::TextureSubresourceViewType::StencilOnly)
				flags = flags & (~vk::ImageAspectFlagBits::eDepth);
		}

		return flags;
	}

	vk::ImageViewType TextureDimensionToImageViewType(RHITextureDimension dimension) {
		switch (dimension) {
			using enum RHITextureDimension;
		case Texture1D:
			return vk::ImageViewType::e1D;

		case Texture1DArray:
			return vk::ImageViewType::e1DArray;

		case Texture2D:case Texture2DMS:
			return vk::ImageViewType::e2D;

		case Texture2DArray:
		case Texture2DMSArray:
			return vk::ImageViewType::e2DArray;

		case TextureCube:
			return vk::ImageViewType::eCube;

		case TextureCubeArray:
			return vk::ImageViewType::eCubeArray;

		case Texture3D:
			return vk::ImageViewType::e3D;

		case Unknown:default:
			ASSERT(false);
			return vk::ImageViewType::e2D;
		}
	}

	//Imp

	TextureSubresourceView& Texture::Get_SubresourceView(const RHITextureSubresourceSet& subresource, RHITextureDimension dimension, RHIFormat format, vk::ImageUsageFlags usage, TextureSubresourceViewType viewtype) {
		// This function is called from createBindingSet etc. and therefore free-threaded.
		// It modifies the subresourceViews map associated with the texture.
		LockGuard lockGuard{ this->m_Mutex };//TODO : 

		if (RHITextureDimension::Unknown == dimension)
			dimension = this->m_Desc.Dimension;

		if (RHIFormat::UNKNOWN == format)
			format = this->m_Desc.Format;

		// Only use VkImageViewUsageCreateInfo when the image is typeless, i.e. it was created
		// with the MUTABLE_FORMAT and EXTENDED_USAGE bits.
		if (!this->m_Desc.IsTypeless)
			usage = vk::ImageUsageFlags{};

		auto cachekey{ MakeTuple(subresource, viewtype, dimension, format, usage) };

		if (auto iter = this->m_SubresourceViews.find(cachekey); iter != this->m_SubresourceViews.end())
			return iter->second;

		auto iter_pair{ this->m_SubresourceViews.emplace(cachekey, *this) };
		auto& view{ std::get<0>(iter_pair)->second };

		view.Subresource = subresource;

		auto vkformat{ ConvertFormat(format) };

		vk::ImageAspectFlags aspectflags{ GuessSubresourceImageAspectFlags(vk::Format{ vkformat }, viewtype) };

		view.SubresourceRange = vk::ImageSubresourceRange{}
			.setAspectMask(aspectflags)
			.setBaseMipLevel(subresource.BaseMipLevel)
			.setLevelCount(subresource.MipLevelCount)
			.setBaseArrayLayer(subresource.BaseArraySlice)
			.setLayerCount(subresource.ArraySliceCount);

		vk::ImageViewType imageViewType{ TextureDimensionToImageViewType(dimension) };

		auto viewInfo{ vk::ImageViewCreateInfo{}
			.setImage(this->m_Image)
			.setViewType(imageViewType)
			.setFormat(vk::Format{ vkformat })
			.setSubresourceRange(view.SubresourceRange) };

		auto usageInfo{ vk::ImageViewUsageCreateInfo{}.setUsage(usage) };

		if (0 != static_cast<Uint32>(usage))
			viewInfo.setPNext(&usageInfo);

		if (viewtype == TextureSubresourceViewType::StencilOnly)
			viewInfo.components.setG(vk::ComponentSwizzle::eR);
		// D3D / HLSL puts stencil values in the second component to keep the illusion of combined depth/stencil.
		// Set a component swizzle so we appear to do the same.

		VULKAN_CHECK(this->m_Context.Device.createImageView(&viewInfo, this->m_Context.AllocationCallbacks, &view.View));

		const String debugName{ String("ImageView for: ") + (this->m_Desc.DebugName.empty() ? String{ "UnNamed" } : this->m_Desc.DebugName) };
		this->m_Context.NameVKObject(VkImageView{ view.View }, vk::ObjectType::eImageView, vk::DebugReportObjectTypeEXT::eImageView, debugName.c_str());

		return view;
	}

	RHIObject Texture::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_Image:return RHIObject{ .Pointer{ this->m_Image } };
		case VK_DeviceMemory:
			return RHIObject{ .Pointer{ this->MemoryResource::Memory } };
		case VK_ImageCreateInfo:return RHIObject{ .Pointer { const_cast<vk::ImageCreateInfo*>(&this->m_ImageInfo) } };
		default:return RHIObject{};
		}
	}

	RHIObject Texture::Imp_GetNativeView(RHIObjectType objectType, RHIFormat format, RHITextureSubresourceSet subresources, RHITextureDimension dimension, bool /*isReadOnlyDSV*/) {
		switch (objectType) {
		case RHIObjectType::VK_ImageView: {
			if (format == RHIFormat::UNKNOWN)
				format = this->m_Desc.Format;

			const RHIFormatInfo& formatInfo{ Get_RHIFormatInfo(format) };

			TextureSubresourceViewType viewType{ TextureSubresourceViewType::AllAspects };
			if (formatInfo.HasDepth && !formatInfo.HasStencil)
				viewType = TextureSubresourceViewType::DepthOnly;
			else if (!formatInfo.HasDepth && formatInfo.HasStencil)
				viewType = TextureSubresourceViewType::StencilOnly;

			// Note: we don't have the intended usage information here, so VkImageViewUsageCreateInfo won't be added to the view.
			return RHIObject{ .Pointer{ this->Get_SubresourceView(subresources, dimension, format, vk::ImageUsageFlags{}, viewType).View } };
		}
		default:
			return RHIObject{};
		}
	}

	struct StagingTextureRegion final {
		// offset, size in bytes
		/*off_t*/ long Offset;
		Uint64 Size;
	};

	class StagingTexture final : public RHIStagingTexture<StagingTexture> {
		friend class RHIResource<StagingTexture>;
		friend class RHIStagingTexture<StagingTexture>;

		friend class CommandList;
		friend class Device;
	public:
		StagingTexture(void) = default;
		~StagingTexture(void) = default;

	public:
		Uint64 ComputeSliceSize(Uint32 mipLevel) const;

		const StagingTextureRegion& Get_SliceRegion(Uint32 mipLevel, Uint32 arraySlice, Uint32 z);

		void PopulateSliceRegions(void);

		Uint64 Get_BufferSize(void) const {
			ASSERT(this->m_SliceRegions.size() > 0);

			Uint64 size{ this->m_SliceRegions.back().Offset + this->m_SliceRegions.back().Size };

			ASSERT(size > 0);

			return size;
		}

	private:
		RHITextureDesc m_Desc;
		// backing store for staging texture is a buffer
		RefCountPtr<Buffer> m_Buffer;
		// per-mip, per-slice regions
		// offset = mipLevel * numDepthSlices + depthSlice
		Vector<StagingTextureRegion> m_SliceRegions;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept { LOG_ERROR("Imp But Empty"); return RHIObject{}; };

		const RHITextureDesc& Imp_Get_Desc(void)const { return this->m_Desc; }
	};

	constexpr long AlignBufferOffset(long off) {
		constexpr long bufferAlignmentBytes{ 4 };

		return ((off + (bufferAlignmentBytes - 1)) / bufferAlignmentBytes) * bufferAlignmentBytes;
	}

	inline Uint64 StagingTexture::ComputeSliceSize(Uint32 mipLevel) const {
		const auto& formatInfo{ Get_RHIFormatInfo(this->m_Desc.Format) };

		Uint32 wInBlocks{ Math::Max(((this->m_Desc.Extent.Width >> mipLevel) + formatInfo.BlockSize - 1) / formatInfo.BlockSize, 1u) };
		Uint32 hInBlocks{ Math::Max(((this->m_Desc.Extent.Height >> mipLevel) + formatInfo.BlockSize - 1) / formatInfo.BlockSize, 1u) };

		Uint64 blockPitchBytes{ static_cast<Uint64>(wInBlocks) * formatInfo.BytesPerBlock };

		return blockPitchBytes * hInBlocks;
	}

	inline const StagingTextureRegion& StagingTexture::Get_SliceRegion(Uint32 mipLevel, Uint32 arraySlice, Uint32 z) {
		if (this->m_Desc.Extent.Depth != 1) {
			// Hard case, since each mip level has half the slices as the previous one.
			ASSERT(arraySlice == 0);
			ASSERT(z < this->m_Desc.Extent.Depth);

			Uint32 mipDepth{ this->m_Desc.Extent.Depth };
			Uint64 index{ 0 };
			while (mipLevel-- > 0) {
				index += mipDepth;
				mipDepth = Math::Max(mipDepth, 1u);
			}
			return this->m_SliceRegions[index + z];
		}
		else if (this->m_Desc.ArrayCount != 1) {
			// Easy case, since each mip level has a consistent number of slices.
			ASSERT(z == 0);
			ASSERT(arraySlice < this->m_Desc.ArrayCount);
			ASSERT(this->m_SliceRegions.size() == this->m_Desc.MipLevelCount * this->m_Desc.ArrayCount);

			return this->m_SliceRegions[static_cast<Uint64>(mipLevel) * this->m_Desc.ArrayCount + arraySlice];
		}
		else
		{
			ASSERT(arraySlice == 0);
			ASSERT(z == 0);
			ASSERT(this->m_SliceRegions.size() == this->m_Desc.MipLevelCount);

			return this->m_SliceRegions[mipLevel];
		}
	}

	inline void StagingTexture::PopulateSliceRegions(void) {
		using Off_T = decltype(Declval<decltype(this->m_SliceRegions)::value_type>().Offset);

		Off_T curOffset{ 0 };

		this->m_SliceRegions.clear();

		for (Uint32 mip = 0; mip < this->m_Desc.MipLevelCount; ++mip) {
			auto sliceSize{ this->ComputeSliceSize(mip) };

			Uint32 depth{ Math::Max(this->m_Desc.Extent.Depth >> mip, 1u) };
			Uint32 numSlices{ this->m_Desc.ArrayCount * depth };

			for (Uint32 slice = 0; slice < numSlices; ++slice) {
				this->m_SliceRegions.push_back(StagingTextureRegion{ .Offset{ curOffset}, .Size{ sliceSize } });

				// update offset for the next region
				curOffset = AlignBufferOffset(static_cast<Off_T>(curOffset + sliceSize));
			}
		}
	}
}
