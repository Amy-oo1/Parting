#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(DescriptorTableManager)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/View.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class FrameBufferFactory final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
	public:
		FrameBufferFactory(Imp_Device* device) :m_Device{ device } {}
		~FrameBufferFactory(void) = default;

	public:
		auto Get_FrameBuffer(const RHI::RHITextureSubresourceSet& subresources) -> Imp_FrameBuffer*;
		auto Get_FrameBuffer(const IView& view) -> Imp_FrameBuffer*;

	public:
		Vector<RHI::RefCountPtr<Imp_Texture>> RenderTargets;
		RHI::RefCountPtr<Imp_Texture> DepthStencil;
		RHI::RefCountPtr<Imp_Texture> ShadingRateSurface;

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		UnorderedMap<RHI::RHITextureSubresourceSet, RHI::RefCountPtr<Imp_FrameBuffer>, decltype(RHI::g_TextureSubresourceSetHash)> m_FrameBufferCache{ 0, RHI::g_TextureSubresourceSetHash };//TODO :


	};

	template<RHI::APITagConcept APITag>
	inline auto FrameBufferFactory<APITag>::Get_FrameBuffer(const RHI::RHITextureSubresourceSet& subresources) -> Imp_FrameBuffer* {
		auto& item{ this->m_FrameBufferCache[subresources] };

		if (nullptr == item) {
			RHI::RHIFrameBufferDescBuilder<APITag> descBuilder{};
			for (auto renderTarget : this->RenderTargets)
				descBuilder.AddColorAttachment(RHI::RHIFrameBufferAttachment<APITag>{.Texture{ renderTarget }, .Subresources{ subresources } });

			if (nullptr != this->DepthStencil)
				descBuilder.Set_DepthStencilAttachment(RHI::RHIFrameBufferAttachment<APITag>{.Texture{ this->DepthStencil }, .Subresources{ subresources } });

			if (nullptr != ShadingRateSurface)
				descBuilder.Set_ShadingRateAttachment(RHI::RHIFrameBufferAttachment<APITag>{.Texture{ this->ShadingRateSurface }, .Subresources{ subresources } });

			item = this->m_Device->CreateFrameBuffer(descBuilder.Build());
		}

		return item;
	}

	template<RHI::APITagConcept APITag>
	inline auto FrameBufferFactory<APITag>::Get_FrameBuffer(const IView& view) -> Imp_FrameBuffer* {
		return this->Get_FrameBuffer(view.Get_Subresources());
	}

}