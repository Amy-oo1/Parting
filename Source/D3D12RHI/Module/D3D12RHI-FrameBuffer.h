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
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Texture)

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
#include "D3D12RHI/Module/D3D12RHI-Texture.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	class FrameBuffer final :public RHIFrameBuffer<FrameBuffer, D3D12Tag> {
		friend class RHIResource<FrameBuffer>;
		friend class RHIFrameBuffer<FrameBuffer, D3D12Tag>;

		friend class CommandList;
		friend class Device;
	public:
		explicit FrameBuffer(D3D12DeviceResources& resource) :
			RHIFrameBuffer<FrameBuffer, D3D12Tag>{},
			m_DeviceResourcesRef{ resource } {
		}

		~FrameBuffer(void) {
			this->m_TextureCount = 0;

			for (Uint32 Index = 0; Index < this->m_RTVCount; ++Index)
				this->m_DeviceResourcesRef.RenderTargetViewHeap.ReleaseDescriptor(this->m_RTVs[Index]);
			this->m_RTVCount = 0;

			if (g_InvalidDescriptorIndex != this->DSV)
				this->m_DeviceResourcesRef.DepthStencilViewHeap.ReleaseDescriptor(this->DSV);
		}

	public:

	private:

	private:
		D3D12DeviceResources& m_DeviceResourcesRef;

		RHIFrameBufferDesc<D3D12Tag> m_Desc{};
		RHIFrameBufferInfo<D3D12Tag> m_Info{};

		Array<RefCountPtr<Texture>, g_MaxRenderTargetCount + 1> m_Texture{};
		RemoveCV<decltype(g_MaxRenderTargetCount)>::type m_TextureCount{ 0 };

		Array<D3D12DescriptorIndex, g_MaxRenderTargetCount> m_RTVs{};
		RemoveCV<decltype(g_MaxRenderTargetCount)>::type m_RTVCount{ 0 };

		D3D12DescriptorIndex DSV{ g_InvalidDescriptorIndex };

		Uint32 m_RenderTargetWidth{ 0 };
		Uint32 m_RenderTargetHeight{ 0 };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };

		const RHIFrameBufferDesc<D3D12Tag>& Imp_Get_Desc(void)const { return this->m_Desc; };
		const RHIFrameBufferInfo<D3D12Tag>& Imp_Get_Info(void)const { return this->m_Info; };
	};
}