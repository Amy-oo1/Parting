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
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Texture)

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
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Texture.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {
	//NOTE :this struct byte lessest more than 32 byte no copy
	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIFrameBufferAttachment final {
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;

		Imp_Texture* Texture{ nullptr };
		RHITextureSubresourceSet Subresources{};

		RHIFormat Format{ RHIFormat::UNKNOWN };
		bool IsReadOnly{ false };

		STDNODISCARD constexpr bool Is_Valid(void)const { return nullptr != this->Texture /* && this->Format != RHIFormat::UNKNOWN*/; }
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIFrameBufferDesc final {
		Array<RHIFrameBufferAttachment<APITag>, g_MaxRenderTargetCount> ColorAttachments;
		RemoveCV<decltype(g_MaxRenderTargetCount)>::type ColorAttachmentCount{ 0 };

		RHIFrameBufferAttachment<APITag> DepthStencilAttachment;
	};

	PARTING_EXPORT template<APITagConcept APITag>
		class RHIFrameBufferDescBuilder final {
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;
		public:
			constexpr RHIFrameBufferDescBuilder& Reset(void) { this->m_Desc = RHIFrameBufferDesc<APITag>{}; return *this; }

			constexpr RHIFrameBufferDescBuilder& AddColorAttachment(const RHIFrameBufferAttachment<APITag>& attachment) {
				ASSERT(this->m_Desc.ColorAttachmentCount < g_MaxRenderTargetCount);

				this->m_Desc.ColorAttachments[this->m_Desc.ColorAttachmentCount++] = attachment;
				return *this;
			}
			constexpr RHIFrameBufferDescBuilder& Set_DepthStencilAttachment(const RHIFrameBufferAttachment<APITag>& attachment) { this->m_Desc.DepthStencilAttachment = attachment; return *this; }
			
			RHIFrameBufferDescBuilder& AddColorAttachment(Imp_Texture* texture) { this->m_Desc.ColorAttachments[this->m_Desc.ColorAttachmentCount++] = RHIFrameBufferAttachment<APITag>{ .Texture { texture } }; return *this; }

			STDNODISCARD constexpr const RHIFrameBufferDesc<APITag>& Build(void) { return this->m_Desc; }
		private:
			RHIFrameBufferDesc<APITag> m_Desc{};
	};

	// Describes the parameters of a framebuffer that can be used to determine if a given framebuffer
	// is compatible with a certain graphics or meshlet pipeline object. All fields of FramebufferInfo
	// must match between the framebuffer and the pipeline for them to be compatible.
	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIFrameBufferInfo final {
		Array<RHIFormat, g_MaxRenderTargetCount> ColorFormats{};
		Uint32 ColorFormatCount{ 0 };

		RHIFormat DepthFormat{ RHIFormat::UNKNOWN };
		Uint32 SampleCount{ 1 };
		Uint32 SampleQuality{ 0 };

		Uint32 Width{ 0 };
		Uint32 Height{ 0 };

		STDNODISCARD constexpr RHIViewport Get_Viewport(float minz = 0.f, float maxz = 1.f) {
			return RHIViewport{
				.MinX{ 0.f }, .MaxX{ static_cast<float>(this->Width) },
				.MinY{ 0.f }, .MaxY{ static_cast<float>(this->Height) },
				.MinZ{ minz }, .MaxZ{ maxz }
			};
		}

		STDNODISCARD constexpr bool operator==(const RHIFrameBufferInfo<APITag>& other) const {
			return
				this->ColorFormatCount == other.ColorFormatCount &&
				RHIFrameBufferInfo<APITag>::Is_FormatsEqual(this->ColorFormatCount, this->ColorFormats, other.ColorFormats) &&
				this->DepthFormat == other.DepthFormat &&
				this->SampleCount == other.SampleCount &&
				this->SampleQuality == other.SampleQuality;
			//&&this->Width == other.Width &&
			//this->Height == other.Height;
		}
		STDNODISCARD constexpr bool operator!=(const RHIFrameBufferInfo<APITag>& other) const { return !(*this == other); }

		STDNODISCARD static RHIFrameBufferInfo<APITag> Build(const RHIFrameBufferDesc<APITag>& desc) {
			RHIFrameBufferInfo<APITag> Re;//TODO : Forget....

			for (const auto& Attachment : Span<const RHIFrameBufferAttachment<APITag>>{ desc.ColorAttachments.data(), desc.ColorAttachmentCount }) {
				if (RHIFormat::UNKNOWN == Attachment.Format && nullptr != Attachment.Texture)
					Re.ColorFormats[Re.ColorFormatCount++] = Attachment.Texture->Get_Desc().Format;
				else
					Re.ColorFormats[Re.ColorFormatCount++] = Attachment.Format;
			}

			if (desc.DepthStencilAttachment.Is_Valid()) {
				const RHITextureDesc& TextureDesc{ desc.DepthStencilAttachment.Texture->Get_Desc() };
				Re.DepthFormat = TextureDesc.Format;
				Re.SampleCount = TextureDesc.SampleCount;
				Re.SampleQuality = TextureDesc.SampleQuality;

				Re.Width = Math::Max(TextureDesc.Extent.Width >> desc.DepthStencilAttachment.Subresources.BaseMipLevel, 1u);
				Re.Height = Math::Max(TextureDesc.Extent.Height >> desc.DepthStencilAttachment.Subresources.BaseMipLevel, 1u);

			}
			else if (0 != desc.ColorAttachmentCount && desc.ColorAttachments[0].Is_Valid()) {
				ASSERT(nullptr != desc.ColorAttachments[0].Texture);
				const auto& TextureDesc{ desc.ColorAttachments[0].Texture->Get_Desc() };
				Re.SampleCount = TextureDesc.SampleCount;
				Re.SampleQuality = TextureDesc.SampleQuality;

				Re.Width = Math::Max(TextureDesc.Extent.Width >> desc.ColorAttachments[0].Subresources.BaseMipLevel, 1u);
				Re.Height = Math::Max(TextureDesc.Extent.Height >> desc.ColorAttachments[0].Subresources.BaseMipLevel, 1u);
			}

			return Re;
		}

		STDNODISCARD static constexpr bool Is_FormatsEqual(Uint32 Count, const Array<RHIFormat, g_MaxRenderTargetCount>& Lhs, const Array<RHIFormat, g_MaxRenderTargetCount>& Rhs) {//TODOsapan
			for (Uint32 Index = 0; Index < Count; ++Index)
				if (Lhs[Index] != Rhs[Index])
					return false;

			return true;
		}

		struct RHIFrameBufferInfoHash final {
			Uint64 operator()(const RHIFrameBufferInfo<APITag>& s) const {
				Uint64 hash{ 0 };
				for (Uint32 Index = 0; Index < s.ColorFormatCount; ++Index)
					hash = HashCombine(hash, Hash<RHIFormat>{}(s.ColorFormats[Index]));
				hash = HashCombine(hash, Hash<RHIFormat>{}(s.DepthFormat));
				hash = HashCombine(hash, HashUint32{}(s.SampleCount));
				hash = HashCombine(hash, HashUint32{}(s.SampleQuality));
				/*HashCombine(hash, HashUint32{}(s.Width));
				HashCombine(hash, HashUint32{}(s.Height));*/

				return hash;
			}
		};
	};

	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
		class RHIFrameBuffer :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHIFrameBuffer(void) = default;
			~RHIFrameBuffer(void) = default;

		public:
			STDNODISCARD const RHIFrameBufferDesc<APITag>& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }
			STDNODISCARD const RHIFrameBufferInfo<APITag>& Get_Info(void)const { return this->Get_Derived()->Imp_Get_Info(); }

		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
		private:
			const RHIFrameBufferDesc<APITag>& Imp_Get_Desc(void)const { LOG_ERROR("No Imp"); return RHIFrameBufferDesc<APITag>{}; }
			const RHIFrameBufferInfo<APITag>& Imp_Get_Info(void)const { LOG_ERROR("No Imp"); return RHIFrameBufferInfo<APITag>{}; }


	};
}