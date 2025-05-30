#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"


PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;
PARTING_IMPORT Algorithm;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Heap)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(Sampler)


#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Container/Module/Container.h"
#include "Core/Logger/Module/Logger.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Heap.h"
#include "RHI/Module/RHI-Texture.h"
#include "RHI/Module/RHI-Buffer.h"
#include "RHI/Module/RHI-Sampler.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace RHI {

	//identifies the underlying resource type in a binding
	PARTING_EXPORT enum class RHIResourceType : Uint8 {
		None,
		Texture_SRV,
		Texture_UAV,
		TypedBuffer_SRV,
		TypedBuffer_UAV,
		StructuredBuffer_SRV,
		StructuredBuffer_UAV,
		RawBuffer_SRV,
		RawBuffer_UAV,
		ConstantBuffer,
		VolatileConstantBuffer,
		Sampler,
		PushConstants,

		Count
	};

	PARTING_EXPORT struct/* alignas(4)*/ RHIBindingLayoutItem final {
		Uint32 Slot;
		RHIResourceType Type{ RHIResourceType::None };
		Uint16 ByteSize;

		STDNODISCARD constexpr bool operator==(const RHIBindingLayoutItem& other) const noexcept { return Slot == other.Slot && Type == other.Type && ByteSize == other.ByteSize; }
		STDNODISCARD constexpr bool operator!=(const RHIBindingLayoutItem& other) const noexcept { return !(*this == other); }

		STDNODISCARD static constexpr RHIBindingLayoutItem Texture_SRV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::Texture_SRV } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem Texture_UAV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::Texture_UAV } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem TypedBuffer_SRV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::TypedBuffer_SRV } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem TypedBuffer_UAV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::TypedBuffer_UAV } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem StructuredBuffer_SRV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::StructuredBuffer_SRV } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem StructuredBuffer_UAV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::StructuredBuffer_UAV } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem RawBuffer_SRV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::RawBuffer_SRV} }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem RawBuffer_UAV(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::RawBuffer_UAV} }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem ConstantBuffer(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::ConstantBuffer } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem VolatileConstantBuffer(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::VolatileConstantBuffer } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem Sampler(Uint32 solt) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::Sampler } }; }
		STDNODISCARD static constexpr RHIBindingLayoutItem PushConstants(Uint32 solt, Uint16 Size) noexcept { return RHIBindingLayoutItem{ .Slot{ solt }, .Type { RHIResourceType::PushConstants }, .ByteSize { Size } }; }


	};

	PARTING_EXPORT struct RHIBindingLayoutDesc final {
		RHIShaderType Visibility{ RHIShaderType::None };

		// In DX12, this controls the register space of the bindings
		// In Vulkan, DXC maps register spaces to descriptor sets by default, so this can be used to
		// determine the descriptor set index for the binding layout.
		// In order to use this behaviour, you must set `registerSpaceIsDescriptorSet` to true.  See below.
		Uint32 RegisterSpace{ 0 };

		// This flag controls the behavior for pipelines that use multiple binding layouts.
		// It must be set to the same value for _all_ of the binding layouts in a pipeline.
		// - When it's set to `false`, the `registerSpace` parameter only affects the DX12 implementation,
		//   and the validation layer will report an error when non-zero `registerSpace` is used with other APIs.
		// - When it's set to `true` the parameter also affects the Vulkan implementation, allowing any
		//   layout to occupy any register space or descriptor set, regardless of their order in the pipeline.
		//   However, a consequence of DXC mapping the descriptor set index to register space is that you may
		//   not have more than one `BindingLayout` using the same `registerSpace` value in the same pipeline.
		// - When it's set to different values for the layouts in a pipeline, the validation layer will report
		//   an error.
		bool RegisterSpaceIsDescriptorSet{ false };
		Array<RHIBindingLayoutItem, g_MaxBindingsPerLayout> Bindings{};
		RemoveCV<decltype(g_MaxBindingsPerLayout)>::type BindingCount{ 0 };
	};

	PARTING_EXPORT class RHIBindingLayoutDescBuilder final {
	public:
		constexpr RHIBindingLayoutDescBuilder& Reset(void) { this->m_Desc = RHIBindingLayoutDesc{}; return *this; }

		constexpr RHIBindingLayoutDescBuilder& Set_Visibility(RHIShaderType visibility) { this->m_Desc.Visibility = visibility; return *this; }
		constexpr RHIBindingLayoutDescBuilder& Set_RegisterSpace(Uint32 registerSpace) { this->m_Desc.RegisterSpace = registerSpace; return *this; }
		constexpr RHIBindingLayoutDescBuilder& Set_RegisterSpaceIsDescriptorSet(bool isDescriptorSet) { this->m_Desc.RegisterSpaceIsDescriptorSet = isDescriptorSet; return *this; }
		constexpr RHIBindingLayoutDescBuilder& AddBinding(RHIBindingLayoutItem binding) { this->m_Desc.Bindings[this->m_Desc.BindingCount++] = binding; return *this; }

		constexpr RHIBindingLayoutDescBuilder& AddBinding(Uint32 Slot, RHIResourceType type) { this->m_Desc.Bindings[this->m_Desc.BindingCount++] = RHIBindingLayoutItem{ .Slot{ Slot }, .Type {type} }; return *this; }

		STDNODISCARD constexpr const RHIBindingLayoutDesc& Build(void) { return this->m_Desc; }
	private:
		RHIBindingLayoutDesc m_Desc{};
	};

	PARTING_EXPORT template<typename Derived>
		class RHIBindingLayout :public RHIResource<Derived> {
		friend class RHIResource<Derived>;//NOTE : Up final Class not Miiddle Class ,if you shoudl cut call in middle ,kan do 
		public:
			RHIBindingLayout(void) = default;
			PARTING_VIRTUAL ~RHIBindingLayout(void) = default;

		public:
			STDNODISCARD const RHIBindingLayoutDesc* Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }  // returns nullptr for bindless layouts
		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }

		private:
			const RHIBindingLayoutDesc* Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return nullptr; }
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIBindingSetItem final {
		RHIShaderBindingResources<APITag> ResourcePtr{ nullptr };

		Uint32 Slot;

		RHIResourceType Type{ RHIResourceType::None };
		RHITextureDimension Dimension{ RHITextureDimension::Unknown }; // valid for Texture_SRV, Texture_UAV
		RHIFormat Format; // valid for Texture_SRV, Texture_UAV, Buffer_SRV, Buffer_UAV
		//Uint8 unused : 8;

		union {
			RHITextureSubresourceSet Subresources; // valid for Texture_SRV, Texture_UAV
			RHIBufferRange Range; // valid for Buffer_SRV, Buffer_UAV, ConstantBuffer
			Uint64 RawData[2];
		};

		STDNODISCARD constexpr bool operator==(const RHIBindingSetItem<APITag>& other) const noexcept {
			return
				this->ResourcePtr == other.ResourcePtr &&
				this->Slot == other.Slot &&
				this->Type == other.Type &&
				this->Dimension == other.Dimension &&
				this->Format == other.Format &&
				this->RawData[0] == other.RawData[0] &&
				this->RawData[1] == other.RawData[1];
		}
		STDNODISCARD constexpr bool operator!=(const RHIBindingSetItem<APITag>& other) const noexcept { return !(*this == other); }

		STDNODISCARD static constexpr decltype(auto) None(Uint32 Slot = 0) { return RHIBindingSetItem<APITag>{.Slot{ Slot }, .RawData{ 0, 0 } }; }

		STDNODISCARD static constexpr decltype(auto) Texture_SRV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Texture* texture, RHIFormat format = RHIFormat::UNKNOWN, RHITextureSubresourceSet subresources = g_AllSubResourceSet, RHITextureDimension dimension = RHITextureDimension::Unknown) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ texture },
					.Slot{ Slot },
					.Type{ RHIResourceType::Texture_SRV },
					.Dimension{ dimension },
					.Format{ format },
					.Subresources{ subresources }
			};
		}

		STDNODISCARD static constexpr decltype(auto) Texture_UAV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Texture* texture, RHIFormat format = RHIFormat::UNKNOWN, RHITextureSubresourceSet subresources = RHITextureSubresourceSet{ .ArraySliceCount{ Max_Uint32 } }, RHITextureDimension dimension = RHITextureDimension::Unknown) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ texture },
					.Slot{ Slot },
					.Type{ RHIResourceType::Texture_UAV },
					.Dimension{ dimension },
					.Format{ format },
					.Subresources{ subresources }
			};
		}

		STDNODISCARD static constexpr decltype(auto) TypedBuffer_SRV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Buffer* buffer, RHIFormat format = RHIFormat::UNKNOWN, RHIBufferRange range = RHIBufferRange{ .Offset{ 0 }, .ByteSize{ 0 } }) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ buffer },
					.Slot{ Slot },
					.Type{ RHIResourceType::TypedBuffer_SRV },
					.Format{ format },
					.Range{ range }
			};
		}

		STDNODISCARD static constexpr decltype(auto) TypedBuffer_UAV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Buffer* buffer, RHIFormat format = RHIFormat::UNKNOWN, RHIBufferRange range = g_EntireBuffer) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ buffer },
					.Slot{ Slot },
					.Type{ RHIResourceType::TypedBuffer_UAV },
					.Format{ format },
					.Range{ range }
			};
		}

		STDNODISCARD static constexpr decltype(auto) ConstantBuffer(Uint32 Slot, RHITypeTraits<APITag>::Imp_Buffer* buffer, RHIBufferRange range = g_EntireBuffer) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ buffer },
					.Slot{ Slot },
					.Type{ buffer->Get_Desc().IsVolatile ? RHIResourceType::VolatileConstantBuffer : RHIResourceType::ConstantBuffer },
					.Range{ range }
			};
		}

		STDNODISCARD static constexpr decltype(auto) Sampler(Uint32 Slot, RHITypeTraits<APITag>::Imp_Sampler* sampler) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ sampler },
					.Slot{ Slot },
					.Type{ RHIResourceType::Sampler },
					.RawData{ 0, 0 }
			};
		}

		STDNODISCARD static constexpr decltype(auto) StructuredBuffer_SRV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Buffer* buffer, RHIFormat format = RHIFormat::UNKNOWN, RHIBufferRange range = g_EntireBuffer) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ buffer },
					.Slot{ Slot },
					.Type{ RHIResourceType::StructuredBuffer_SRV },
					.Format{ format },
					.Range{ range }
			};
		}

		STDNODISCARD static constexpr decltype(auto) StructuredBuffer_UAV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Buffer* buffer, RHIFormat format = RHIFormat::UNKNOWN, RHIBufferRange range = g_EntireBuffer) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ buffer },
					.Slot{ Slot },
					.Type{ RHIResourceType::StructuredBuffer_UAV },
					.Format{ format },
					.Range{ range }
			};
		}

		STDNODISCARD static constexpr decltype(auto) RawBuffer_SRV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Buffer* buffer, RHIBufferRange range = g_EntireBuffer) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ buffer },
					.Slot{ Slot },
					.Type{ RHIResourceType::RawBuffer_SRV },
					.Range{ range }
			};
		}

		STDNODISCARD static constexpr decltype(auto) RawBuffer_UAV(Uint32 Slot, RHITypeTraits<APITag>::Imp_Buffer* buffer, RHIBufferRange range = g_EntireBuffer) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ buffer },
					.Slot{ Slot },
					.Type{ RHIResourceType::RawBuffer_UAV },
					.Range{ range }
			};
		}

		STDNODISCARD static constexpr decltype(auto) PushConstants(Uint32 Slot, Uint32 ByteSize) {
			return RHIBindingSetItem<APITag>{
				.ResourcePtr{ nullptr },
					.Slot{ Slot },
					.Type{ RHIResourceType::PushConstants },
					.Range{ .Offset{ 0 }, .ByteSize{ ByteSize } }
			};
		}

		struct BindingSetItemHash final {
			Uint64 operator()(const RHIBindingSetItem<APITag>& item) {
				Uint64 hash{ 0 };
			/*	hash = ::HashCombine(hash, Hash<RHIShaderBindingResources<APITag>>{}(item.ResourcePtr));*/
				hash = ::HashCombine(hash, HashUint32{}(item.Slot));
				hash = ::HashCombine(hash, Hash<RHIResourceType>{}(item.Type));
				hash = ::HashCombine(hash, Hash<RHITextureDimension>{}(item.Dimension));
				hash = ::HashCombine(hash, Hash<RHIFormat>{}(item.Format));
				hash = ::HashCombine(hash, HashUint64{}(item.RawData[0]));
				hash = ::HashCombine(hash, HashUint64{}(item.RawData[1]));
				return hash;
			}
		};

	};

	// describes a set of bindings across all stages of the pipeline
	// (not all bindings need to be present in the set, but the set must be defined by a single BindingSetItem object)
	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIBindingSetDesc final {
		Array<RHIBindingSetItem<APITag>, g_MaxBindingsPerLayout> Bindings{};
		RemoveCV<decltype(g_MaxBindingsPerLayout)>::type BindingCount{ 0 };

		// Enables automatic liveness tracking of this binding set by nvrhi command lists.
	   // By setting trackLiveness to false, you take the responsibility of not releasing it 
	   // until all rendering commands using the binding set are finished.
		bool TrackLiveness{ true };

		STDNODISCARD constexpr bool operator==(const RHIBindingSetDesc<APITag>& other) const noexcept {
			if (this->BindingCount != other.BindingCount)
				return false;

			for (Uint32 Index = 0; Index < this->BindingCount; ++Index)
				if (this->Bindings[Index] != other.Bindings[Index])
					return false;
			return true;
		}
		STDNODISCARD constexpr bool operator!=(const RHIBindingSetDesc<APITag>& other) const noexcept { return !(*this == other); }

		struct BindingSetHash final {
			Uint64 operator ()(const RHIBindingSetDesc<APITag>& desc)const noexcept {
				Uint64 hash{ 0 };
				for (Uint32 Index = 0; Index < desc.BindingCount; ++Index)
					hash = HashCombine(hash, typename RHIBindingSetItem<APITag>::BindingSetItemHash{}(desc.Bindings[Index]));

				return hash;
			}
		};
	};

	/*PARTING_EXPORT*/ template<APITagConcept APITag>
	class RHIBindingSetDescBuilder final {
	public:
		constexpr RHIBindingSetDescBuilder& Reset(void) { this->m_Desc = RHIBindingSetDesc<APITag>{}; return *this; }

		constexpr RHIBindingSetDescBuilder& Set_TrackLiveness(bool trackLiveness) { this->m_Desc.TrackLiveness = trackLiveness; return *this; }
		constexpr RHIBindingSetDescBuilder& AddBinding(const RHIBindingSetItem<APITag>& binding) { this->m_Desc.Bindings[this->m_Desc.BindingCount++] = binding; return *this; }

		STDNODISCARD constexpr const RHIBindingSetDesc<APITag>& Build(void) { return this->m_Desc; }

	public://TODO :
		RHIBindingSetDesc<APITag> m_Desc{};
	};

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
		class RHIBindingSet :public RHIResource<Derived> {
		friend class RHIResource<Derived>;

		using Imp_BindingLayout = typename RHITypeTraits<APITag>::Imp_BindingLayout;

		protected:
			RHIBindingSet(void) = default;
			PARTING_VIRTUAL ~RHIBindingSet(void) = default;

		public:
			STDNODISCARD const RHIBindingSetDesc<APITag>* Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }
			STDNODISCARD const Imp_BindingLayout* Get_Layout()const { return this->Get_Derived()->Imp_Get_Layout(); }

		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
		private:
			const RHIBindingSetDesc<APITag>* Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return nullptr; }
			const Imp_BindingLayout* Imp_Get_Layout(void)const { LOG_ERROR("No Imp"); return nullptr; }
	};
}