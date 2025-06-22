#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"


PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Heap)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Heap.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {

	PARTING_EXPORT enum class RHITextureDimension : Uint8 {
		Unknown,
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		TextureCube,
		TextureCubeArray,
		Texture2DMS,
		Texture2DMSArray,
		Texture3D
	};

	PARTING_EXPORT struct RHITextureDesc final {
		RHIExtent3D Extent{ .Width{ 1 } ,.Height{ 1 }, .Depth{ 1 } };
		Uint32 ArrayCount{ 1 };
		Uint32 MipLevelCount{ 1 };
		Uint32 SampleCount{ 1 };
		Uint32 SampleQuality{ 0 };//NOTE Sampler Desc Not need maybe fr
		RHIFormat Format{ RHIFormat::UNKNOWN };
		RHITextureDimension Dimension{ RHITextureDimension::Texture2D };
		String DebugName;

		bool IsShaderResource{ true }; // Note: isShaderResource is initialized to 'true' for backward compatibility
		bool IsRenderTarget{ false };
		bool IsUAV{ false };
		bool IsTypeless{ false };

		// Indicates that the texture is created with no backing memory,
		// and memory is bound to the texture later using bindTextureMemory.
		// On DX12, the texture resource is created at the time of memory binding.
		bool IsVirtual{ false };
		bool IsTiled{ false };

		Optional<Color> ClearValue{ NullOpt };

		RHIResourceState InitialState{ RHIResourceState::Unknown };

		// If keepInitialState is true, command lists that use the texture will automatically
		// begin tracking the texture from the initial state and transition it to the initial state 
		// on command list close.
		bool KeepInitialState{ false };
	};

	PARTING_EXPORT class RHITextureDescBuilder final {
	public:
		constexpr RHITextureDescBuilder& Reset(void) { this->m_Desc = RHITextureDesc{}; return *this; }
		constexpr RHITextureDescBuilder& Set_Extent(const RHIExtent3D& extent) { this->m_Desc.Extent = extent; return *this; }
		constexpr RHITextureDescBuilder& Set_ArraySize(const Uint32 arraySize) { this->m_Desc.ArrayCount = arraySize; return *this; }
		constexpr RHITextureDescBuilder& Set_MipLevels(const Uint32 mipLevels) { this->m_Desc.MipLevelCount = mipLevels; return *this; }
		constexpr RHITextureDescBuilder& Set_SampleCount(const Uint32 sampleCount) { this->m_Desc.SampleCount = sampleCount; return *this; }
		constexpr RHITextureDescBuilder& Set_SampleQuality(const Uint32 sampleQuality) { this->m_Desc.SampleQuality = sampleQuality; return *this; }
		constexpr RHITextureDescBuilder& Set_Format(const RHIFormat format) { this->m_Desc.Format = format; return *this; }
		constexpr RHITextureDescBuilder& Set_Dimension(const RHITextureDimension dimension) { this->m_Desc.Dimension = dimension; return *this; }
		constexpr RHITextureDescBuilder& Set_DebugName(const String& debugName) { this->m_Desc.DebugName = debugName; return *this; }
		constexpr RHITextureDescBuilder& Set_IsShaderResource(const bool isShaderResource) { this->m_Desc.IsShaderResource = isShaderResource; return *this; }
		constexpr RHITextureDescBuilder& Set_IsRenderTarget(const bool isRenderTarget) { this->m_Desc.IsRenderTarget = isRenderTarget; return *this; }
		constexpr RHITextureDescBuilder& Set_IsUAV(const bool isUAV) { this->m_Desc.IsUAV = isUAV; return *this; }
		constexpr RHITextureDescBuilder& Set_IsTypeless(const bool isTypeless) { this->m_Desc.IsTypeless = isTypeless; return *this; }
		constexpr RHITextureDescBuilder& Set_IsVirtual(const bool isVirtual) { this->m_Desc.IsVirtual = isVirtual; return *this; }
		constexpr RHITextureDescBuilder& Set_IsTiled(const bool isTiled) { this->m_Desc.IsTiled = isTiled; return *this; }
		constexpr RHITextureDescBuilder& Set_ClearValue(const Color& clearValue) { this->m_Desc.ClearValue = clearValue; return *this; }
		constexpr RHITextureDescBuilder& Set_InitialState(const RHIResourceState initialState) { this->m_Desc.InitialState = initialState; return *this; }
		constexpr RHITextureDescBuilder& Set_KeepInitialState(const bool keepInitialState) { this->m_Desc.KeepInitialState = keepInitialState; return *this; }

		constexpr RHITextureDescBuilder& Set_Width(const Uint32 width) { this->m_Desc.Extent.Width = width; return *this; }
		constexpr RHITextureDescBuilder& Set_Height(const Uint32 height) { this->m_Desc.Extent.Height = height; return *this; }
		constexpr RHITextureDescBuilder& Set_Depth(const Uint32 depth) { this->m_Desc.Extent.Depth = depth; return *this; }
		constexpr RHITextureDescBuilder& Set_SamplerDesc(const Uint32 Count, const Uint32 Quality) { this->m_Desc.SampleCount = Count; this->m_Desc.SampleQuality = Quality; return *this; }

		STDNODISCARD constexpr const RHITextureDesc& Build(void)const { return this->m_Desc; }


	private:
		RHITextureDesc m_Desc{};
	};

	PARTING_EXPORT struct RHITextureSlice final {
		RHIOffset3D Offset{ .X{ 0 }, .Y{ 0 }, .Z{ 0 } };
		RHIExtent3D Extent{ .Width{ Max_Uint32 }, .Height{ Max_Uint32 }, .Depth{ Max_Uint32 } };// means the entire dimension is part of the region resolve() below will translate these values into actual dimensions

		Uint32 MipLevel{ 0 };//TODO 
		Uint32 ArraySlice{ 0 };//TODO swap member loaction 

		STDNODISCARD RHITextureSlice Resolve(const RHITextureDesc& desc)const {
			auto slice{ *this };

			if (Max_Uint32 == slice.Extent.Width)
				slice.Extent.Width = Math::Max(1u, desc.Extent.Width >> this->MipLevel);
			if (Max_Uint32 == slice.Extent.Height)
				slice.Extent.Height = Math::Max(1u, desc.Extent.Height >> this->MipLevel);
			if (Max_Uint32 == slice.Extent.Depth) {
				if (RHITextureDimension::Texture3D == desc.Dimension)
					slice.Extent.Depth = Math::Max(1u, desc.Extent.Depth >> this->MipLevel);
				else
					slice.Extent.Depth = 1;
			}

			return slice;
		}
	};

	PARTING_EXPORT struct RHITextureSliceBuilder final {
	public:
		STDNODISCARD constexpr RHITextureSliceBuilder& Reset(void) { this->m_Slice = RHITextureSlice{}; return *this; }

		STDNODISCARD constexpr RHITextureSliceBuilder& Set_Offset(const RHIOffset3D& offset) { this->m_Slice.Offset = offset; return *this; }
		STDNODISCARD constexpr RHITextureSliceBuilder& Set_Extent(const RHIExtent3D& extent) { this->m_Slice.Extent = extent; return *this; }
		STDNODISCARD constexpr RHITextureSliceBuilder& Set_MipLevel(const Uint32 mipLevel) { this->m_Slice.MipLevel = mipLevel; return *this; }
		STDNODISCARD constexpr RHITextureSliceBuilder& Set_ArraySlice(const Uint32 arraySlice) { this->m_Slice.ArraySlice = arraySlice; return *this; }

		STDNODISCARD constexpr RHITextureSliceBuilder& Set_Width(const Uint32 width) { this->m_Slice.Extent.Width = width; return *this; }
		STDNODISCARD constexpr RHITextureSliceBuilder& Set_Height(const Uint32 height) { this->m_Slice.Extent.Height = height; return *this; }
		STDNODISCARD constexpr RHITextureSliceBuilder& Set_Depth(const Uint32 depth) { this->m_Slice.Extent.Depth = depth; return *this; }

		STDNODISCARD constexpr const RHITextureSlice& Build(void) const { return this->m_Slice; }

	private:
		RHITextureSlice m_Slice{};
	};

	PARTING_EXPORT struct RHITextureSubresourceSet final {
		static constexpr Uint32 s_ALLMipLevels{ Max_Uint32 };
		static constexpr Uint32 s_ALLArraySlices{ Max_Uint32 };

		Uint32 BaseMipLevel{ 0 };
		Uint32 MipLevelCount{ 1 };

		Uint32 BaseArraySlice{ 0 };
		Uint32 ArraySliceCount{ 1 };

		STDNODISCARD bool Is_EntireTexture(const RHITextureDesc& desc)const {
			if (this->BaseMipLevel > 0u || this->BaseMipLevel + this->MipLevelCount < desc.MipLevelCount)
				return false;

			switch (desc.Dimension) {
				using enum RHITextureDimension;
			case Texture1DArray:case Texture2DArray:case TextureCube:case TextureCubeArray:case Texture2DMSArray:
				if (this->BaseArraySlice > 0u || this->BaseArraySlice + this->ArraySliceCount < desc.ArrayCount)
					return false;
			default:
				return true;
			};
		}

		STDNODISCARD RHITextureSubresourceSet Resolve(const RHITextureDesc& desc, bool issinglemiplevel)const {
			RHITextureSubresourceSet Re{ .BaseMipLevel = this->BaseMipLevel };

			if (!issinglemiplevel) {
				const Uint64 LastMipLevelPlusOne{ Math::Min<Uint64>(static_cast<Uint64>(this->BaseMipLevel) + this->MipLevelCount, desc.MipLevelCount) };
				Re.MipLevelCount = static_cast<Uint32>(Math::Max<Uint64>(0, LastMipLevelPlusOne - static_cast<Uint64>(this->BaseMipLevel)));
			}

			switch (desc.Dimension) {
				using enum RHITextureDimension;
			case Texture1DArray:case Texture2DArray:case TextureCube:case TextureCubeArray:case Texture2DMSArray:
				Re.BaseArraySlice = this->BaseArraySlice;
				Re.ArraySliceCount = Math::Max(0u, Math::Min(this->BaseArraySlice + this->ArraySliceCount, desc.ArrayCount) - this->BaseArraySlice);
				break;

			default: break;//NOTE : nothing to do
			};

			return Re;

		}

		STDNODISCARD constexpr bool operator==(const RHITextureSubresourceSet&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHITextureSubresourceSet&)const noexcept = default;

		struct Hash final {
			Uint64 operator()(const RHITextureSubresourceSet& set) const noexcept {
				Uint64 hash{ 0 };
				hash = HashCombine(hash, HashUint32{}(set.BaseMipLevel));
				hash = HashCombine(hash, HashUint32{}(set.MipLevelCount));
				hash = HashCombine(hash, HashUint32{}(set.BaseArraySlice));
				hash = HashCombine(hash, HashUint32{}(set.ArraySliceCount));

				return hash;
			}
		};;

	};

	struct RHITextureBindingKey final {
		RHITextureSubresourceSet SubresourceSet;
		RHIFormat Format;
		bool IsReadOnlyDSV{ false };

		STDNODISCARD constexpr bool operator==(const RHITextureBindingKey&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHITextureBindingKey&)const noexcept = default;

		struct Hash final {
			Uint64 operator()(const RHITextureBindingKey& key) const noexcept {
				Uint64 hash{ 0 };
				hash = HashCombine(hash, RHITextureSubresourceSet::Hash{}(key.SubresourceSet));
				hash = HashCombine(hash, ::Hash<UnderlyingType<RHIFormat>>()(Tounderlying(key.Format)));
				hash = HashCombine(hash, ::Hash<bool>()(key.IsReadOnlyDSV));
				return hash;
			}
		};
	};

	template<typename Value>
	using RHITextureBindingMap = UnorderedMap<RHITextureBindingKey, Value, RHI::RHITextureBindingKey::Hash>;

	template<APITagConcept APITag>
	struct RHITextureSubresourcesKey final {
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;
		Imp_Texture* Texture{ nullptr };
		RHITextureSubresourceSet Subresources;

		STDNODISCARD constexpr bool operator==(const RHITextureSubresourcesKey<APITag>&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHITextureSubresourcesKey<APITag>&) const noexcept = default;

		struct Hash final {
			Uint64 operator()(const RHITextureSubresourcesKey<APITag>& key) const noexcept {
				Uint64 hash{ 0 };
				hash = ::HashCombine(hash, ::HashVoidPtr{}(key.Texture));
				hash = ::HashCombine(hash, RHITextureSubresourceSet::Hash{}(key.Subresources));
				return hash;
			}
		};

	};



	PARTING_EXPORT HEADER_INLINE constexpr RHITextureSubresourceSet g_AllSubResourceSet{
		.BaseMipLevel{ 0 },
		.MipLevelCount{ RHITextureSubresourceSet::s_ALLMipLevels },
		.BaseArraySlice{ 0 },
		.ArraySliceCount{ RHITextureSubresourceSet::s_ALLArraySlices }
	};

	PARTING_EXPORT template<typename Derived>
		class RHITexture :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHITexture(void) = default;
			PARTING_VIRTUAL ~RHITexture(void) = default;

		public:
			STDNODISCARD const RHITextureDesc& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }
			STDNODISCARD RHIObject GetNativeView(RHIObjectType objectType, RHIFormat format = RHIFormat::UNKNOWN, RHITextureSubresourceSet subresources = g_AllSubResourceSet, RHITextureDimension dimension = RHITextureDimension::Unknown, bool isReadOnlyDSV = false) { return this->Get_Derived()->Imp_GetNativeView(objectType, format, subresources, dimension, isReadOnlyDSV); }

		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
		private:
			const RHITextureDesc& Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return RHITextureDesc{}; }
			RHIObject Imp_GetNativeView(RHIObjectType objectType, RHIFormat format, RHITextureSubresourceSet subresources, RHITextureDimension dimension, bool isReadOnlyDSV) { LOG_ERROR("No Imp"); return RHIObject{}; }
	};

	PARTING_EXPORT template<typename Derived>
		class RHIStagingTexture :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHIStagingTexture(void) = default;
			PARTING_VIRTUAL ~RHIStagingTexture(void) = default;

		public:
			STDNODISCARD const RHITextureDesc& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }

		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }

		private:
			const RHITextureDesc& Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return RHITextureDesc{}; }

	};

	PARTING_EXPORT struct RHITiledTextureCoordinate final {
		Uint16 MipLevel{ 0 };
		Uint16 ArrayLevel{ 0 };
		RHIExtent3D TileCoordinate{ .Width{ 0 }, .Height{ 0 }, .Depth{ 0 } };
	};

	PARTING_EXPORT struct RHITiledTextureRegion final {
		Uint32 TilesCount{ 0 };
		RHIExtent3D TileSize{ .Width{ 0 }, .Height{ 0 }, .Depth{ 0 } };
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHITextureTilesMapping final {
		using Imp_Heap = typename RHITypeTraits<APITag>::Imp_Heap;

		RHITiledTextureCoordinate* TiledTextureCoordinates{ nullptr };
		RHITiledTextureRegion* TiledTextureRegions{ nullptr };
		Uint64* ByteOffsets{ nullptr };
		Uint32 TextureRegionCount{ 0 };
		Imp_Heap* Heap{ nullptr };
	};

	PARTING_EXPORT struct RHIPackedMipDesc final {
		Uint32 StandardMipCount{ 0 };
		Uint32 PackedMipCount{ 0 };
		Uint32 TilesForPackedMipCount{ 0 };
		Uint32 StartTileIndexInOverallResource{ 0 };
	};

	PARTING_EXPORT struct RHITileShape final {
		Uint32 WidthInTexels{ 0 };
		Uint32 HeightInTexels{ 0 };
		Uint32 DepthInTexels{ 0 };
	};

	PARTING_EXPORT struct RHISubresourceTiling final {
		Uint32 WidthInTiles{ 0 };
		Uint32 HeightInTiles{ 0 };
		Uint32 DepthInTiles{ 0 };
		Uint32 StartTileIndexInOverallResource{ 0 };
	};

}