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
#include "D3D12RHI/Module/D3D12RHI-Buffer.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	// helper function for texture subresource calculations
   // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx
	STDNODISCARD inline constexpr Uint32 CalcSubresource(Uint32 MipSlice, Uint32 ArraySlice, Uint32 PlaneSlice, Uint32 MipLevels, Uint32 ArraySize) {
		return MipSlice + (ArraySlice * MipLevels) + (PlaneSlice * MipLevels * ArraySize);
	}

	//NOTE :Texture
	PARTING_EXPORT class Texture :public RHITexture<Texture> {
		friend class RHIResource<Texture>;
		friend class RHITexture<Texture>;

		friend class BindingSet;
		friend class CommandList;
		friend class Device;
	public:
		Texture(const Context& context, D3D12DeviceResources& resource, const RHITextureDesc& desc, const D3D12_RESOURCE_DESC& d3d12resourcedesc) :
			RHITexture<Texture>{},
			m_Context{ context },
			m_DeviceResourcesRef{ resource },
			m_Desc{ desc },
			m_ResourceDesc{ d3d12resourcedesc } {
		}

		~Texture(void);

	public:

		void PostCreate(void);

		void CreateSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, RHITextureDimension dimension, RHITextureSubresourceSet subresources)const;
		void CreateUAV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, RHITextureDimension dimension, RHITextureSubresourceSet subresources)const;
		void CreateRTV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, RHITextureSubresourceSet subresources)const;
		void CreateDSV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHITextureSubresourceSet subresources, bool isReadOnly = false)const;

		D3D12DescriptorIndex Get_ClearMipLevelUAV(Uint32 MipLevel);

		//RHIObject Get_NativeView()

		static D3D12_RESOURCE_DESC ConvertTextureDesc(const RHITextureDesc& desc);

		static D3D12_CLEAR_VALUE ConvertTextureClearValue(const RHITextureDesc& desc);

	private:
		const Context& m_Context;
		D3D12DeviceResources& m_DeviceResourcesRef;

		const RHITextureDesc m_Desc;
		RHITextureStateExtension<D3D12Tag> m_StateExtension{ .DescRef{ this->m_Desc }, .ParentTextureRef{ this } };

		const D3D12_RESOURCE_DESC m_ResourceDesc;
		RefCountPtr<ID3D12Resource> m_Resource;
		Uint8 m_PlaneCount{ Max_Uint8 };//TODO 0 or Max_Uint8 both is err (map uint init 0...
		HANDLE m_SharedHandle{ nullptr };

		RefCountPtr<Heap> m_Heap;

		RHITextureBindingMap<D3D12DescriptorIndex> m_RenderTargetViews{ 0, g_TextureBindingKeyHash };//TODO :function to class 
		RHITextureBindingMap<D3D12DescriptorIndex> m_DepthStencilViews{ 0, g_TextureBindingKeyHash };
		RHITextureBindingMap<D3D12DescriptorIndex> m_CustomSRVs{ 0, g_TextureBindingKeyHash };
		RHITextureBindingMap<D3D12DescriptorIndex> m_CustomUAVs{ 0, g_TextureBindingKeyHash };
		Vector<D3D12DescriptorIndex> m_ClearMipLevelUAVs;

	private:

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHITextureDesc& Imp_Get_Desc(void)const { return this->m_Desc; }
		RHIObject Imp_GetNativeView(RHIObjectType objectType, RHIFormat format, RHITextureSubresourceSet subresources, RHITextureDimension dimension, bool isReadOnlyDSV);
	};

	//Src

	inline Texture::~Texture(void) {
		for (const auto& [key, ValueIndex] : this->m_RenderTargetViews)
			this->m_DeviceResourcesRef.RenderTargetViewHeap.ReleaseDescriptor(ValueIndex, 1);
		for (const auto& [key, ValueIndex] : this->m_DepthStencilViews)
			this->m_DeviceResourcesRef.DepthStencilViewHeap.ReleaseDescriptor(ValueIndex, 1);
		for (const auto& [key, ValueIndex] : this->m_CustomSRVs)
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.ReleaseDescriptor(ValueIndex, 1);
		for (const auto& [key, ValueIndex] : this->m_CustomUAVs)
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.ReleaseDescriptor(ValueIndex, 1);
		for (const auto& Index : this->m_ClearMipLevelUAVs)
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.ReleaseDescriptor(Index, 1);
	}

	inline void Texture::PostCreate(void) {
		//TODO :set Name;
		this->m_Resource->SetName(WString{ this->m_Desc.DebugName.begin(),this->m_Desc.DebugName.end() }.c_str());

		if (this->m_Desc.IsUAV)
			this->m_ClearMipLevelUAVs.resize(this->m_Desc.MipLevelCount, g_InvalidDescriptorIndex);

		this->m_PlaneCount = this->m_DeviceResourcesRef.Get_FormatPlaneCount(this->m_ResourceDesc.Format);

		ASSERT(0 != this->m_PlaneCount);
	}

	inline void Texture::CreateSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, RHITextureDimension dimension, RHITextureSubresourceSet subresources)const {
		subresources = subresources.Resolve(this->m_Desc, false);

		if (RHITextureDimension::Unknown == dimension)
			dimension = this->m_Desc.Dimension;

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = Get_DXGIFormatMapping(format == RHIFormat::UNKNOWN ? this->m_Desc.Format : format).SRVFormat;
		SRVDesc.Shader4ComponentMapping = g_D3D12DefaultShader4ComponentMapping;

		Uint32 PlaneSlice{ DXGI_FORMAT_X24_TYPELESS_G8_UINT == SRVDesc.Format ? 1u : 0u };

		switch (dimension) {
		case RHITextureDimension::Texture1D:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			SRVDesc.Texture1D.MostDetailedMip = subresources.BaseMipLevel;
			SRVDesc.Texture1D.MipLevels = subresources.MipLevelCount;

			break;
		case RHITextureDimension::Texture1DArray:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			SRVDesc.Texture1DArray.MostDetailedMip = subresources.BaseMipLevel;
			SRVDesc.Texture1DArray.MipLevels = subresources.MipLevelCount;
			SRVDesc.Texture1DArray.FirstArraySlice = subresources.BaseArraySlice;
			SRVDesc.Texture1DArray.ArraySize = subresources.ArraySliceCount;

			break;
		case RHITextureDimension::Texture2D:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MostDetailedMip = subresources.BaseMipLevel;
			SRVDesc.Texture2D.MipLevels = subresources.MipLevelCount;
			SRVDesc.Texture2D.PlaneSlice = PlaneSlice;

			break;
		case RHITextureDimension::Texture2DArray:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			SRVDesc.Texture2DArray.MostDetailedMip = subresources.BaseMipLevel;
			SRVDesc.Texture2DArray.MipLevels = subresources.MipLevelCount;
			SRVDesc.Texture2DArray.FirstArraySlice = subresources.BaseArraySlice;
			SRVDesc.Texture2DArray.ArraySize = subresources.ArraySliceCount;
			SRVDesc.Texture2DArray.PlaneSlice = PlaneSlice;

			break;
		case RHITextureDimension::TextureCube:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			SRVDesc.TextureCube.MostDetailedMip = subresources.BaseMipLevel;
			SRVDesc.TextureCube.MipLevels = subresources.MipLevelCount;

			break;
		case RHITextureDimension::TextureCubeArray:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			SRVDesc.TextureCubeArray.MostDetailedMip = subresources.BaseMipLevel;
			SRVDesc.TextureCubeArray.MipLevels = subresources.MipLevelCount;
			SRVDesc.TextureCubeArray.First2DArrayFace = subresources.BaseArraySlice;
			SRVDesc.TextureCubeArray.NumCubes = subresources.ArraySliceCount / 6;

			break;
		case RHITextureDimension::Texture2DMS:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			SRVDesc.Texture2DMS.UnusedField_NothingToDefine = 0;

			break;
		case RHITextureDimension::Texture2DMSArray:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
			SRVDesc.Texture2DMSArray.FirstArraySlice = subresources.BaseArraySlice;
			SRVDesc.Texture2DMSArray.ArraySize = subresources.ArraySliceCount;

			break;
		case RHITextureDimension::Texture3D:
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			SRVDesc.Texture3D.MostDetailedMip = subresources.BaseMipLevel;
			SRVDesc.Texture3D.MipLevels = subresources.MipLevelCount;
			SRVDesc.Texture3D.ResourceMinLODClamp = 0.0f;

			break;
		default:
			ASSERT(false);
			break;
		}

		this->m_Context.Device->CreateShaderResourceView(this->m_Resource.Get(), &SRVDesc, descriptorhandle);
	}

	inline void Texture::CreateUAV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, RHITextureDimension dimension, RHITextureSubresourceSet subresources)const {
		subresources = subresources.Resolve(this->m_Desc, true);

		if (RHITextureDimension::Unknown == dimension)
			dimension = this->m_Desc.Dimension;

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc{};
		UAVDesc.Format = Get_DXGIFormatMapping(format == RHIFormat::UNKNOWN ? this->m_Desc.Format : format).SRVFormat;

		switch (dimension) {
		case RHITextureDimension::Texture1D:
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			UAVDesc.Texture1D.MipSlice = subresources.BaseMipLevel;

			break;
		case RHITextureDimension::Texture1DArray:
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			UAVDesc.Texture1DArray.MipSlice = subresources.BaseMipLevel;
			UAVDesc.Texture1DArray.FirstArraySlice = subresources.BaseArraySlice;
			UAVDesc.Texture1DArray.ArraySize = subresources.ArraySliceCount;

			break;
		case RHITextureDimension::Texture2D:
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			UAVDesc.Texture2D.MipSlice = subresources.BaseMipLevel;

			break;
		case RHITextureDimension::Texture2DArray:
		case RHITextureDimension::TextureCube:
		case RHITextureDimension::TextureCubeArray:
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			UAVDesc.Texture2DArray.MipSlice = subresources.BaseMipLevel;
			UAVDesc.Texture2DArray.FirstArraySlice = subresources.BaseArraySlice;
			UAVDesc.Texture2DArray.ArraySize = subresources.ArraySliceCount;

			break;
		case RHITextureDimension::Texture3D:
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			UAVDesc.Texture3D.MipSlice = subresources.BaseMipLevel;
			UAVDesc.Texture3D.FirstWSlice = 0;
			UAVDesc.Texture3D.WSize = this->m_Desc.Extent.Depth;

			break;
		case RHITextureDimension::Texture2DMS:
		case RHITextureDimension::Texture2DMSArray:
		default:
			ASSERT(false);
			break;
		}

		this->m_Context.Device->CreateUnorderedAccessView(this->m_Resource.Get(), nullptr, &UAVDesc, descriptorhandle);
	}

	inline void Texture::CreateRTV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHIFormat format, RHITextureSubresourceSet subresources)const {
		subresources = subresources.Resolve(this->m_Desc, true);

		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc{};
		RTVDesc.Format = Get_DXGIFormatMapping(RHIFormat::UNKNOWN == format ? this->m_Desc.Format : format).RTVFormat;//TODO : C-type (no default), not use juheti init ,some field no init (bug)

		switch (this->m_Desc.Dimension) {
			using enum RHITextureDimension;
		case Texture1D:
			RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			RTVDesc.Texture1D.MipSlice = subresources.BaseMipLevel;
			break;

		case Texture1DArray:
			RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			RTVDesc.Texture1DArray.MipSlice = subresources.BaseMipLevel;
			RTVDesc.Texture1DArray.FirstArraySlice = subresources.BaseArraySlice;
			RTVDesc.Texture1DArray.ArraySize = subresources.ArraySliceCount;
			break;

		case Texture2D:
			RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			RTVDesc.Texture2D.MipSlice = subresources.BaseMipLevel;
			break;

		case Texture2DArray:case TextureCube:case TextureCubeArray:
			RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			RTVDesc.Texture2DArray.MipSlice = subresources.BaseMipLevel;
			RTVDesc.Texture2DArray.FirstArraySlice = subresources.BaseArraySlice;
			RTVDesc.Texture2DArray.ArraySize = subresources.ArraySliceCount;
			break;

		case Texture2DMS:
			RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			RTVDesc.Texture2DMS.UnusedField_NothingToDefine = 0;
			break;

		case Texture2DMSArray:
			RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
			RTVDesc.Texture2DMSArray.FirstArraySlice = subresources.BaseArraySlice;
			RTVDesc.Texture2DMSArray.ArraySize = subresources.ArraySliceCount;
			break;

		case Texture3D:
			RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			RTVDesc.Texture3D.MipSlice = subresources.BaseMipLevel;
			RTVDesc.Texture3D.FirstWSlice = subresources.BaseArraySlice;
			RTVDesc.Texture3D.WSize = subresources.ArraySliceCount;
			break;

		default:
			ASSERT(false);
			break;
		}

		this->m_Context.Device->CreateRenderTargetView(this->m_Resource.Get(), &RTVDesc, descriptorhandle);
	}

	inline void Texture::CreateDSV(D3D12_CPU_DESCRIPTOR_HANDLE descriptorhandle, RHITextureSubresourceSet subresources, bool isReadOnly)const {
		subresources = subresources.Resolve(this->m_Desc, true);

		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc{ .Format{ Get_DXGIFormatMapping(this->m_Desc.Format).RTVFormat } };

		if (isReadOnly) {
			DSVDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
			if (DXGI_FORMAT_D24_UNORM_S8_UINT == DSVDesc.Format || DXGI_FORMAT_D32_FLOAT_S8X24_UINT == DSVDesc.Format)
				DSVDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		}

		switch (this->m_Desc.Dimension) {
			using enum RHITextureDimension;
		case Texture1D:
			DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			DSVDesc.Texture1D.MipSlice = subresources.BaseMipLevel;
			break;

		case Texture1DArray:
			DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
			DSVDesc.Texture1DArray.MipSlice = subresources.BaseMipLevel;
			DSVDesc.Texture1DArray.FirstArraySlice = subresources.BaseArraySlice;
			DSVDesc.Texture1DArray.ArraySize = subresources.ArraySliceCount;
			break;

		case Texture2D:
			DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			DSVDesc.Texture2D.MipSlice = subresources.BaseMipLevel;
			break;

		case Texture2DArray:case TextureCube:case TextureCubeArray:
			DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			DSVDesc.Texture2DArray.MipSlice = subresources.BaseMipLevel;
			DSVDesc.Texture2DArray.FirstArraySlice = subresources.BaseArraySlice;
			DSVDesc.Texture2DArray.ArraySize = subresources.ArraySliceCount;
			break;

		case Texture2DMS:
			DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
			break;

		case Texture2DMSArray:
			DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			DSVDesc.Texture2DMSArray.FirstArraySlice = subresources.BaseArraySlice;
			DSVDesc.Texture2DMSArray.ArraySize = subresources.ArraySliceCount;
			break;

		case RHITextureDimension::Texture3D:default:
			ASSERT(false);
			break;
		}

		this->m_Context.Device->CreateDepthStencilView(this->m_Resource.Get(), &DSVDesc, descriptorhandle);
	}

	inline D3D12DescriptorIndex Texture::Get_ClearMipLevelUAV(Uint32 MipLevel) {
		ASSERT(this->m_Desc.IsUAV);

		auto DescriptorIndex{ this->m_ClearMipLevelUAVs[MipLevel] };

		if (g_InvalidDescriptorIndex != DescriptorIndex)
			return DescriptorIndex;

		DescriptorIndex = this->m_DeviceResourcesRef.ShaderResourceViewHeap.AllocateDescriptor(1);
		RHITextureSubresourceSet Subresources{
			.BaseMipLevel{ MipLevel },
			.MipLevelCount{ 1 },
			.BaseArraySlice{ 0 },
			.ArraySliceCount{ RHITextureSubresourceSet::s_ALLArraySlices }
		};

		this->CreateUAV(this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(DescriptorIndex), RHIFormat::UNKNOWN, RHITextureDimension::Unknown, Subresources);
		this->m_DeviceResourcesRef.ShaderResourceViewHeap.CopyToShaderVisibleHeap(DescriptorIndex, 1);
		this->m_ClearMipLevelUAVs[MipLevel] = DescriptorIndex;

		return DescriptorIndex;
	}

	inline D3D12_RESOURCE_DESC Texture::ConvertTextureDesc(const RHITextureDesc& d) {
		const auto& formatMapping{ Get_DXGIFormatMapping(d.Format) };
		const auto& formatInfo{ Get_RHIFormatInfo(d.Format) };

		D3D12_RESOURCE_DESC desc{
			.Width { d.Extent.Width },
			.Height { d.Extent.Height },
			.MipLevels { static_cast<Uint16>(d.MipLevelCount) },
			.Format { d.IsTypeless ? formatMapping.ResourceFormat : formatMapping.RTVFormat },
			.SampleDesc {.Count { d.SampleCount }, .Quality { d.SampleQuality } }
			//derrf
		};

		switch (d.Dimension) {
			using enum RHITextureDimension;
		case Texture1D:case Texture1DArray:
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			desc.DepthOrArraySize = static_cast<Uint16>(d.ArrayCount);
			break;

		case Texture2D:case Texture2DArray:case TextureCube:case TextureCubeArray:case Texture2DMS:case Texture2DMSArray:
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.DepthOrArraySize = static_cast<Uint16>(d.ArrayCount);
			break;

		case Texture3D:
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			desc.DepthOrArraySize = static_cast<UINT16>(d.Extent.Depth);
			break;

		case Unknown:default:
			ASSERT(false);
			break;
		}

		if (!d.IsShaderResource)
			desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		if (d.IsRenderTarget) {
			if (formatInfo.HasDepth || formatInfo.HasStencil)
				desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			else
				desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}

		if (d.IsUAV)
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		return desc;
	}

	inline D3D12_CLEAR_VALUE Texture::ConvertTextureClearValue(const RHITextureDesc& d) {
		const auto& formatMapping{ Get_DXGIFormatMapping(d.Format) };
		const auto& formatInfo{ Get_RHIFormatInfo(d.Format) };

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = formatMapping.RTVFormat;
		if (formatInfo.HasDepth || formatInfo.HasStencil) {
			clearValue.DepthStencil.Depth = d.ClearValue->R;
			clearValue.DepthStencil.Stencil = static_cast<Uint8>(d.ClearValue->G);
		}
		else {
			clearValue.Color[0] = d.ClearValue->R;
			clearValue.Color[1] = d.ClearValue->G;
			clearValue.Color[2] = d.ClearValue->B;
			clearValue.Color[3] = d.ClearValue->A;
		}

		return clearValue;
	}

	//Imp

	inline RHIObject Texture::Imp_GetNativeObject(RHIObjectType type)const noexcept {
		switch (type) {
		case RHI::RHIObjectType::SharedHandle: return RHIObject{ .Pointer{ this->m_SharedHandle } };
		case RHI::RHIObjectType::D3D12_Resource: return RHIObject{ .Pointer{ this->m_Resource.Get() } };
		default:
			ASSERT(false);
			return RHIObject{};
		}
	}

	inline RHIObject Texture::Imp_GetNativeView(RHIObjectType objectType, RHIFormat format, RHITextureSubresourceSet subresources, RHITextureDimension dimension, bool isReadOnlyDSV) {
		switch (objectType) {
		case RHIObjectType::D3D12_ShaderResourceViewGpuDescripror: {
			RHITextureBindingKey key{ .SubresourceSet { subresources },.Format { format } };

			D3D12DescriptorIndex descriptorIndex;
			if (auto found{ this->m_CustomSRVs.find(key) }; found == this->m_CustomSRVs.end()) {
				descriptorIndex = this->m_DeviceResourcesRef.ShaderResourceViewHeap.AllocateDescriptor();
				this->m_CustomSRVs[key] = descriptorIndex;

				const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{ this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(descriptorIndex) };
				this->CreateSRV(cpuHandle, format, dimension, subresources);
				this->m_DeviceResourcesRef.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorIndex);
			}
			else
				descriptorIndex = found->second;

			return RHIObject{ .Integer { this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(descriptorIndex).ptr } };
		}
		case RHIObjectType::D3D12_UnorderedAccessViewGpuDescripror: {
			RHITextureBindingKey key{ .SubresourceSet { subresources },.Format { format } };

			D3D12DescriptorIndex descriptorIndex;
			if (auto found{ this->m_CustomSRVs.find(key) }; found == this->m_CustomSRVs.end()) {
				descriptorIndex = this->m_DeviceResourcesRef.ShaderResourceViewHeap.AllocateDescriptor();
				this->m_CustomUAVs[key] = descriptorIndex;

				const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{ this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(descriptorIndex) };
				this->CreateUAV(cpuHandle, format, dimension, subresources);
				this->m_DeviceResourcesRef.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorIndex);
			}
			else
				descriptorIndex = found->second;

			return RHIObject{ .Integer { this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(descriptorIndex).ptr } };
		}
		case RHIObjectType::D3D12_RenderTargetViewDescriptor: {
			RHITextureBindingKey key{ .SubresourceSet { subresources },.Format { format } };

			D3D12DescriptorIndex descriptorIndex;
			if (auto found{ this->m_RenderTargetViews.find(key) }; found == this->m_RenderTargetViews.end()) {
				descriptorIndex = this->m_DeviceResourcesRef.RenderTargetViewHeap.AllocateDescriptor();
				this->m_RenderTargetViews[key] = descriptorIndex;

				const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{ this->m_DeviceResourcesRef.RenderTargetViewHeap.Get_CPUHandle(descriptorIndex).ptr };
				this->CreateRTV(cpuHandle, format, subresources);
			}
			else
				descriptorIndex = found->second;

			return RHIObject{ this->m_DeviceResourcesRef.RenderTargetViewHeap.Get_CPUHandle(descriptorIndex).ptr };
		}
		case RHIObjectType::D3D12_DepthStencilViewDescriptor: {
			RHITextureBindingKey key{ .SubresourceSet { subresources }, .Format { format }, .IsReadOnlyDSV { isReadOnlyDSV } };

			D3D12DescriptorIndex descriptorIndex;
			if (auto found{ this->m_DepthStencilViews.find(key) }; found == this->m_DepthStencilViews.end()) {
				descriptorIndex = this->m_DeviceResourcesRef.DepthStencilViewHeap.AllocateDescriptor();
				this->m_DepthStencilViews[key] = descriptorIndex;

				const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = this->m_DeviceResourcesRef.DepthStencilViewHeap.Get_CPUHandle(descriptorIndex);
				this->CreateDSV(cpuHandle, subresources, isReadOnlyDSV);
			}
			else
				descriptorIndex = found->second;

			return RHIObject{ .Integer{ this->m_DeviceResourcesRef.DepthStencilViewHeap.Get_CPUHandle(descriptorIndex).ptr } };
		}
		default:
			ASSERT(false);
			return	RHIObject{ .Pointer {nullptr} };
		}
	}


	//NOTE :StagingTexture
	PARTING_EXPORT class StagingTexture final :public RHIStagingTexture<StagingTexture> {
		friend class RHIResource<StagingTexture>;
		friend class RHIStagingTexture<StagingTexture>;

		friend class CommandList;
		friend class Device;
	public:
		StagingTexture(void) = default;
		~StagingTexture(void) = default;

	public:
		struct SliceRegion final {
			long /*_off_t*/ Offset{ 0 };
			Uint64 Size{ 0 };

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint{};
		};

	public:
		SliceRegion Get_SliceRegion(ID3D12Device* device, const RHITextureSlice& slice);

		void ComputeSubresourceOffsets(ID3D12Device* device);

		Uint64 Get_SizeInBytes(ID3D12Device* device);

	private:

	private:
		RHITextureDesc m_Desc;
		D3D12_RESOURCE_DESC m_ResourceDesc{};
		RefCountPtr<Buffer> m_Buffer{ nullptr };
		RHICPUAccessMode m_CPUAccessMode{ RHICPUAccessMode::None };

		Vector<Uint64> m_SubresourceOffsets;

		RefCountPtr<ID3D12Fence> m_LastUseFence{ nullptr };
		Uint64 m_LastUseFenceValue{ 0 };

		SliceRegion m_MappedRegion;
		RHICPUAccessMode m_MappedRegionAccessMode{ RHICPUAccessMode::None };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;
		const RHITextureDesc& Imp_Get_Desc(void)const { return this->m_Desc; }
	};

	//Src

	inline StagingTexture::SliceRegion StagingTexture::Get_SliceRegion(ID3D12Device* device, const RHITextureSlice& slice) {
		SliceRegion Re;

		const auto Subresource{ CalcSubresource(
			slice.MipLevel,
			slice.ArraySlice,
			0,
			this->m_Desc.MipLevelCount,
			this->m_Desc.ArrayCount
		) };

		ASSERT(Subresource < this->m_SubresourceOffsets.size());

		Uint64 Size{};
		device->GetCopyableFootprints(&this->m_ResourceDesc, Subresource, 1, this->m_SubresourceOffsets[Subresource], &Re.Footprint, nullptr, nullptr, &Size);

		Re.Offset = static_cast<decltype(Re.Offset)>(Re.Footprint.Offset);
		Re.Size = Size;

		return Re;
	}

	inline void StagingTexture::ComputeSubresourceOffsets(ID3D12Device* device) {
		const auto LastSubresource{ CalcSubresource(
			this->m_ResourceDesc.MipLevels - 1,
			this->m_ResourceDesc.DepthOrArraySize - 1,
			0,
			this->m_ResourceDesc.MipLevels,
			this->m_ResourceDesc.DepthOrArraySize
		) };

		this->m_SubresourceOffsets.resize(LastSubresource + 1);

		Uint64 BaseOffset{ 0 };
		for (RemoveCV<decltype(LastSubresource)>::type Index = 0; Index < LastSubresource + 1; ++Index) {
			Uint64 SubresouceSize{};
			device->GetCopyableFootprints(&m_ResourceDesc, Index, 1, 0, nullptr, nullptr, nullptr, &SubresouceSize);

			this->m_SubresourceOffsets[Index] = BaseOffset;
			BaseOffset += SubresouceSize;
			BaseOffset = g_D3D12TextureDataPlacementAlignment * (BaseOffset + g_D3D12TextureDataPlacementAlignment - 1) / g_D3D12TextureDataPlacementAlignment;
		}

	}

	inline Uint64 StagingTexture::Get_SizeInBytes(ID3D12Device* device) {
		const auto LastSubresource{
			CalcSubresource(
				this->m_ResourceDesc.MipLevels - 1,
				this->m_ResourceDesc.DepthOrArraySize - 1,
				0,
				this->m_Desc.MipLevelCount,
				this->m_Desc.ArrayCount
			) };

		ASSERT(LastSubresource < this->m_SubresourceOffsets.size());

		Uint64 LastSubresourceSize{};
		device->GetCopyableFootprints(&this->m_ResourceDesc, LastSubresource, 1, 0, nullptr, nullptr, nullptr, &LastSubresourceSize);

		return this->m_SubresourceOffsets[LastSubresource] + LastSubresourceSize;
	}

	//Imp

	inline RHIObject StagingTexture::Imp_GetNativeObject(RHIObjectType type) const noexcept {
		switch (type) {
		case RHIObjectType::D3D12_Resource: return RHIObject{ .Pointer{ this->m_Buffer.Get() } };
		default:ASSERT(false); return RHIObject{};
		}
	}
}