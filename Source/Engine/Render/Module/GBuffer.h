#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT VectorMath;
PARTING_IMPORT Logger;


PARTING_SUBMODULE(Parting, SSAOPass)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {

	template<RHI::APITagConcept APITag>
	class GBufferRenderTargets {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
	public:
		GBufferRenderTargets(void) = default;
		virtual ~GBufferRenderTargets(void) = default;

	public:
		STDNODISCARD Math::VecU2 Get_Size(void) const { return this->m_Size; }
		STDNODISCARD Uint32 Get_SampleCount(void) const { return this->m_SampleCount; }
		STDNODISCARD bool Get_UseReverseProjection(void) const { return this->m_UseReverseProjection; }

	protected:
		Math::VecU2 m_Size{ Math::VecU2::Zero() };
		Uint32 m_SampleCount{ 0 };
		bool m_UseReverseProjection{ false };

	public:
		RHI::RefCountPtr<Imp_Texture> Depth;
		RHI::RefCountPtr<Imp_Texture> GBufferDiffuse;
		RHI::RefCountPtr<Imp_Texture> GBufferSpecular;
		RHI::RefCountPtr<Imp_Texture> GBufferNormals;
		RHI::RefCountPtr<Imp_Texture> GBufferEmissive;

		RHI::RefCountPtr<Imp_Texture> MotionVectors;

		SharedPtr<FrameBufferFactory<APITag>> GBufferFrameBuffer;

	public:
		virtual void Init(
			Imp_Device* device,
			Math::VecU2 size,
			Uint32 sampleCount,
			bool enableMotionVectors,
			bool useReverseProjection
		);

		virtual void Clear(Imp_CommandList* commandList);

	};

	template<RHI::APITagConcept APITag>
	inline void GBufferRenderTargets<APITag>::Init(Imp_Device* device, Math::VecU2 size, Uint32 sampleCount, bool enableMotionVectors, bool useReverseProjection) {
		RHI::RHITextureDescBuilder descBuilder{}; descBuilder
			.Set_Width(size.X).Set_Height(size.Y)
			.Set_SampleCount(sampleCount)
			.Set_Dimension(sampleCount > 1 ? RHI::RHITextureDimension::Texture2DMS : RHI::RHITextureDimension::Texture2D)
			.Set_IsRenderTarget(true)
			.Set_ClearValue(Color{ 0.f })
			.Set_InitialState(RHI::RHIResourceState::RenderTarget)
			.Set_KeepInitialState(true);

		this->GBufferDiffuse = device->CreateTexture(descBuilder
			.Set_Format(RHI::RHIFormat::SRGBA8_UNORM)
			.Set_DebugName("GBufferDiffuse")
			.Build()
		);

		this->GBufferSpecular = device->CreateTexture(descBuilder
			.Set_Format(RHI::RHIFormat::SRGBA8_UNORM)
			.Set_DebugName("GBufferSpecular")
			.Build()
		);

		this->GBufferNormals = device->CreateTexture(descBuilder
			.Set_Format(RHI::RHIFormat::RGBA16_SNORM)
			.Set_DebugName("GBufferNormals")
			.Build()
		);

		this->GBufferEmissive = device->CreateTexture(descBuilder
			.Set_Format(RHI::RHIFormat::RGBA16_FLOAT)
			.Set_DebugName("GBufferEmissive")
			.Build()
		);

		this->MotionVectors = device->CreateTexture(descBuilder
			.Set_Width(enableMotionVectors ? size.X : 1).Set_Height(enableMotionVectors ? size.Y : 1)
			.Set_Format(RHI::RHIFormat::RG16_FLOAT)
			.Set_DebugName("GBufferMotionVectors")
			.Build()
		);

		constexpr Array<RHI::RHIFormat, 4> depthFormats{
			RHI::RHIFormat::D24S8,
			RHI::RHIFormat::D32S8,
			RHI::RHIFormat::D32,
			RHI::RHIFormat::D16
		};

		constexpr RHI::RHIFormatSupport depthFeatures{
			RHI::RHIFormatSupport::Texture |
			RHI::RHIFormatSupport::DepthStencil |
			RHI::RHIFormatSupport::ShaderLoad
		};

		this->Depth = device->CreateTexture(descBuilder
			.Set_Width(size.X).Set_Height(size.Y)
			.Set_Format(device->ChooseFormat(depthFeatures, depthFormats.data(), static_cast<Uint32>(depthFormats.size())))
			.Set_DebugName("GBufferDepth")
			.Set_IsTypeless(true)
			.Set_ClearValue(Color{ useReverseProjection ? 0.f : 1.f })
			.Set_InitialState(RHI::RHIResourceState::DepthWrite)
			.Build()
		);

		this->GBufferFrameBuffer = MakeShared<FrameBufferFactory<APITag>>(device);


		GBufferFrameBuffer->RenderTargets.assign({
			this->GBufferDiffuse,
			this->GBufferSpecular,
			this->GBufferNormals,
			this->GBufferEmissive
			});

		if (enableMotionVectors)
			GBufferFrameBuffer->RenderTargets.push_back(this->MotionVectors);

		this->GBufferFrameBuffer->DepthStencil = this->Depth;

		this->m_Size = size;
		this->m_SampleCount = sampleCount;
		this->m_UseReverseProjection = useReverseProjection;
	}

	template<RHI::APITagConcept APITag>
	inline void GBufferRenderTargets<APITag>::Clear(Imp_CommandList* commandList) {
		const RHI::RHIFormatInfo& depthFormatInfo{ RHI::Get_RHIFormatInfo(this->Depth->Get_Desc().Format) };



		commandList->ClearTextureFloat(this->GBufferDiffuse, RHI::g_AllSubResourceSet, Color{ 0.f });
		commandList->ClearTextureFloat(this->GBufferSpecular, RHI::g_AllSubResourceSet, Color{ 0.f });
		commandList->ClearTextureFloat(this->GBufferNormals, RHI::g_AllSubResourceSet, Color{ 0.f });
		commandList->ClearTextureFloat(this->GBufferEmissive, RHI::g_AllSubResourceSet, Color{ 0.f });
		commandList->ClearTextureFloat(this->MotionVectors, RHI::g_AllSubResourceSet, Color{ 0.f });

		float depthClearValue = this->m_UseReverseProjection ? 0.f : 1.f;
		commandList->ClearDepthStencilTexture(this->Depth, RHI::g_AllSubResourceSet, depthClearValue, depthFormatInfo.HasStencil ? Optional<Uint8>{0} : NullOpt);
	}

}