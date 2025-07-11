#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, Buffer)

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
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"
#include "D3D12RHI/Module/D3D12RHI-Format.h"
#include "D3D12RHI/Module/D3D12RHI-Heap.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	PARTING_EXPORT class Buffer final :public RHIBuffer<Buffer> {
		friend class RHIResource<Buffer>;
		friend class RHIBuffer<Buffer>;

		friend class BindingSet;
		friend class CommandList;
		friend class Device;

	public:
		Buffer(const Context& context, D3D12DeviceResources& resource, RHIBufferDesc desc) :
			RHIBuffer<Buffer>{},
			m_Context{ context },
			m_DeviceResourcesRef{ resource },
			m_Desc{ desc }{
		}

		~Buffer(void) {
			if (g_InvalidDescriptorIndex != this->m_ClearUAV) {
				this->m_DeviceResourcesRef.ShaderResourceViewHeap.ReleaseDescriptor(this->m_ClearUAV);
				this->m_ClearUAV = g_InvalidDescriptorIndex;
			}
		}

		void CreateCBV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIBufferRange range)const;
		void CreateSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIBufferRange range, RHIFormat format, RHIResourceType type)const;
		void CreateUAV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIBufferRange range, RHIFormat format, RHIResourceType type)const;
		static void CreateNullSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, const Context& context) {
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc{};//TODO 
			SRVDesc.Format = Get_DXGIFormatMapping(RHIFormat::UNKNOWN == format ? RHIFormat::R32_UINT : format).SRVFormat;
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			SRVDesc.Shader4ComponentMapping = g_D3D12DefaultShader4ComponentMapping;

			context.Device->CreateShaderResourceView(nullptr, &SRVDesc, descriptorhandle);
		}
		static void CreateNullUAV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, const Context& context) {
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc{};
			UAVDesc.Format = Get_DXGIFormatMapping(RHIFormat::UNKNOWN == format ? RHIFormat::R32_UINT : format).SRVFormat;
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			context.Device->CreateUnorderedAccessView(nullptr, nullptr, &UAVDesc, descriptorhandle);
		}

		void PostCreate(void);

		D3D12DescriptorIndex Get_ClearUAV(void);


	private:
		const Context& m_Context;
		D3D12DeviceResources& m_DeviceResourcesRef;
		const RHIBufferDesc m_Desc;
		RHIBufferStateExtension<D3D12Tag> m_StateExtension{ .DescRef{ this->m_Desc }, .ParentBuffer{ this } };//TODO dep in init maybe can see better

		RefCountPtr<ID3D12Resource> m_Resource;
		D3D12_GPU_VIRTUAL_ADDRESS m_GPUVirtualAddress{ 0 };
		D3D12_RESOURCE_DESC m_ResourceDesc{};
		D3D12DescriptorIndex m_ClearUAV{ g_InvalidDescriptorIndex };

		RefCountPtr<Heap> m_Heap;

		RefCountPtr<ID3D12Fence> m_LastUseFence;
		Uint64 m_LastUseFenceValue{ 0 };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHIBufferDesc& Imp_Get_Desc(void) const { return this->m_Desc; }
	};

	//Src

	void Buffer::CreateCBV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIBufferRange range)const {
		ASSERT(this->m_Desc.IsConstantBuffer);

		range = range.Resolve(this->m_Desc);
		ASSERT(range.ByteSize <= Max_Uint32);

		D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc{
			.BufferLocation { this->m_Resource->GetGPUVirtualAddress() + range.Offset },
			.SizeInBytes { Math::Align(static_cast<Uint32>(range.ByteSize), g_ConstantBufferOffsetSizeAlignment) }
		};
		this->m_Context.Device->CreateConstantBufferView(&CBVDesc, descriptorhandle);

	}

	void Buffer::CreateSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIBufferRange range, RHIFormat format, RHIResourceType type)const {
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc{
			.ViewDimension{ D3D12_SRV_DIMENSION_BUFFER },
			.Shader4ComponentMapping{ g_D3D12DefaultShader4ComponentMapping }
		};

		if (RHIFormat::UNKNOWN == format)
			format = this->m_Desc.Format;

		range = range.Resolve(this->m_Desc);

		switch (type) {
		case RHIResourceType::StructuredBuffer_SRV:
			ASSERT(this->m_Desc.StructStride > 0);

			SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
			SRVDesc.Buffer.FirstElement = range.Offset / this->m_Desc.StructStride;
			SRVDesc.Buffer.NumElements = static_cast<Uint32>(range.ByteSize / this->m_Desc.StructStride);
			SRVDesc.Buffer.StructureByteStride = this->m_Desc.StructStride;
			break;

		case RHIResourceType::RawBuffer_SRV:
			ASSERT(this->m_Desc.StructStride == 0);

			SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			SRVDesc.Buffer.FirstElement = range.Offset / sizeof(Uint32);
			SRVDesc.Buffer.NumElements = static_cast<Uint32>(range.ByteSize / sizeof(Uint32));
			SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
			break;

		case RHIResourceType::TypedBuffer_SRV: {
			ASSERT(RHIFormat::UNKNOWN != format);

			const auto& Mapping{ Get_DXGIFormatMapping(format) };
			const auto& Info{ Get_RHIFormatInfo(format) };

			SRVDesc.Format = Mapping.SRVFormat;
			SRVDesc.Buffer.FirstElement = range.Offset / Info.BytesPerBlock;
			SRVDesc.Buffer.NumElements = static_cast<Uint32>(range.ByteSize / Info.BytesPerBlock);
			break;
		}
		default:
			ASSERT(false);
			break;
		}

		this->m_Context.Device->CreateShaderResourceView(this->m_Resource.Get(), &SRVDesc, descriptorhandle);
	}

	void Buffer::CreateUAV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIBufferRange range, RHIFormat format, RHIResourceType type)const {
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc{
			.ViewDimension{ D3D12_UAV_DIMENSION_BUFFER }
		};

		if (RHIFormat::UNKNOWN == format)
			format = this->m_Desc.Format;

		range = range.Resolve(this->m_Desc);

		switch (type) {
		case RHIResourceType::StructuredBuffer_UAV:
			ASSERT(this->m_Desc.StructStride > 0);

			UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
			UAVDesc.Buffer.FirstElement = range.Offset / this->m_Desc.StructStride;
			UAVDesc.Buffer.NumElements = static_cast<Uint32>(range.ByteSize / this->m_Desc.StructStride);
			UAVDesc.Buffer.StructureByteStride = this->m_Desc.StructStride;
			break;

		case RHIResourceType::RawBuffer_UAV:
			ASSERT(this->m_Desc.StructStride == 0);

			UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			UAVDesc.Buffer.FirstElement = range.Offset / sizeof(Uint32);
			UAVDesc.Buffer.NumElements = static_cast<Uint32>(range.ByteSize / sizeof(Uint32));
			UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;//Add
			break;

		case RHIResourceType::TypedBuffer_UAV:
			ASSERT(RHIFormat::UNKNOWN != format);

			UAVDesc.Format = Get_DXGIFormatMapping(format).SRVFormat;
			UAVDesc.Buffer.FirstElement = range.Offset / Get_RHIFormatInfo(format).BytesPerBlock;
			UAVDesc.Buffer.NumElements = static_cast<Uint32>(range.ByteSize / Get_RHIFormatInfo(format).BytesPerBlock);
			break;

		default:
			ASSERT(false);
			break;

		}

		this->m_Context.Device->CreateUnorderedAccessView(this->m_Resource.Get(), nullptr, &UAVDesc, descriptorhandle);
	}


	void Buffer::PostCreate(void) {
		this->m_GPUVirtualAddress = this->m_Resource->GetGPUVirtualAddress();

		/*if (!this->m_Desc.DebugName.empty())
			this->m_Resource->SetName(this->m_Desc.DebugName.c_str());*/
	}

	D3D12DescriptorIndex Buffer::Get_ClearUAV(void) {
		ASSERT(this->m_Desc.CanHaveUAVs);

		if (g_InvalidDescriptorIndex == this->m_ClearUAV) {
			this->m_ClearUAV = this->m_DeviceResourcesRef.ShaderResourceViewHeap.AllocateDescriptor();

			this->CreateUAV(this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(this->m_ClearUAV), g_EntireBuffer, RHIFormat::R32_UINT, RHIResourceType::TypedBuffer_UAV);
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.CopyToShaderVisibleHeap(this->m_ClearUAV);
		}

		return this->m_ClearUAV;
	}

	//Imp

	RHIObject Buffer::Imp_GetNativeObject(RHIObjectType type)const noexcept {
		switch (type) {
		case RHIObjectType::D3D12_Resource: return RHIObject{ .Pointer{ this->m_Resource.Get() } };
		default: { LOG_ERROR("No Imp"); return RHIObject{}; }
		}
	}

}