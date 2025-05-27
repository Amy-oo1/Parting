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

#include "Engine/Render/Module/View.h"

#include "Engine/Render/Module/ShadowMap-Base.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class PlanarShadowMap final :public IShadowMap<APITag> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;

	public:
		PlanarShadowMap(Imp_Device* device, Imp_Texture* texture, Uint32 arraySlice, const RHI::RHIViewport& viewport);
		~PlanarShadowMap(void) = default;

	public:
		SharedPtr<PlanarView> Get_PlanarView(void) { return this->m_View; }

		void Set_FalloffDistance(float distance) { this->m_FalloffDistance = distance; }

		void SetupProxyView(void);

		bool SetupDynamicDirectionalLightView(
			const DirectionalLight<APITag>& light,
			Math::VecF3 anchor,
			Math::VecF3 halfShadowBoxSize,
			Math::VecF3 preViewTranslation = 0.f,
			float fadeRangeWorld = 0.f
		);

	private:
		RHI::RefCountPtr<Imp_Texture> m_ShadowMapTexture;

		Math::VecF2 m_ShadowMapSize{ Math::VecF2::Zero() };//TODO :Uint32
		Math::VecF2 m_TextureSize{ Math::VecF2::Zero() };//TODO :Uint32
		Math::VecF2 m_FadeRangeTexels{ 1.f };

		SharedPtr<PlanarView> m_View;

		float m_FalloffDistance{ 1.f };

		bool m_IsLitOutOfBounds{ false };

	public:
		Math::MatF44 Get_WorldToUvzwMatrix(void) const override {
			// Calculate alternate matrix that maps to [0, 1] UV space instead of [-1, 1] clip space
			constexpr Math::MatF44 matClipToUvzw{
				0.5f,	0,		0,		0,
				0,		-0.5f,	0,		0,
				0,		0,		1,		0,
				0.5f,	0.5f,	0,		1,
			};

			return this->m_View->Get_ViewProjectionMatrix() * matClipToUvzw;
		}

		const class ICompositeView& Get_View(void) const override { return *this->m_View; }

		auto Get_Texture(void) -> Imp_Texture* const override { return this->m_ShadowMapTexture; }

		Uint32 Get_NumberOfCascades(void) const override { return 1; }

		const IShadowMap<APITag>* Get_Cascade(Uint32 index) const override { return this; }

		Uint32 Get_NumberOfPerObjectShadows(void) const override { return 0; }

		const IShadowMap<APITag>* Get_PerObjectShadow(Uint32 index) const override { return nullptr; }

		Math::VecU2 Get_TextureSize(void) const override { return Math::VecU2{ this->m_TextureSize }; }

		Math::BoxF2 Get_UVRange(void) const override {
			const auto& viewportState{ this->m_View->Get_ViewportState() };
			const auto& viewport{ viewportState.Viewports[0] };
			Math::VecF2 topLeft{ viewport.MinX, viewport.MinY };
			Math::VecF2 bottomRight{ viewport.MaxX, viewport.MaxY };

			return Math::BoxF2{
				Math::VecF2{ (topLeft + 1.f) / this->m_TextureSize },
				Math::VecF2{ (bottomRight - 1.f) / this->m_TextureSize }
			};
		}

		Math::VecF2 Get_FadeRangeInTexels(void) const override { return this->m_FadeRangeTexels; }

		bool Is_LitOutOfBounds(void) const override { return this->m_IsLitOutOfBounds; }

		void FillShadowConstants(ShadowConstants& constants) const override {
			constants.MatWorldToUvzwShadow = this->Get_WorldToUvzwMatrix();
			constants.ShadowMapArrayIndex = this->m_View->Get_Subresources().BaseArraySlice;
			Math::BoxF2 uvRange{ this->Get_UVRange() };
			Math::VecF2 fadeUV{ this->Get_FadeRangeInTexels() / this->m_TextureSize };
			constants.ShadowMapCenterUV = uvRange.Get_Center();
			constants.ShadowFadeBias = uvRange.Diagonal() / (2.f * fadeUV);
			constants.ShadowFadeScale = -1.f / fadeUV;
			constants.ShadowFalloffDistance = this->m_FalloffDistance;
			constants.ShadowMapSizeTexels = Math::VecF2{ this->Get_TextureSize() };
			constants.ShadowMapSizeTexelsInv = 1.f / constants.ShadowMapSizeTexels;
		}

	};


	template<RHI::APITagConcept APITag>
	inline PlanarShadowMap<APITag>::PlanarShadowMap(Imp_Device* device, Imp_Texture* texture, Uint32 arraySlice, const RHI::RHIViewport& viewport) :m_ShadowMapTexture{ texture } {
		const RHI::RHITextureDesc& Desc{ this->m_ShadowMapTexture->Get_Desc() };

		this->m_TextureSize = Math::VecF2{ static_cast<float>(Desc.Extent.Width), static_cast<float>(Desc.Extent.Height) };
		this->m_ShadowMapSize = Math::VecF2{ viewport.MaxX - viewport.MinX, viewport.MaxY - viewport.MinY };

		this->m_View = MakeShared<PlanarView>();
		this->m_View->Set_Viewport(viewport);
		this->m_View->Set_ArraySlice(arraySlice);
	}

	template<RHI::APITagConcept APITag>
	inline void PlanarShadowMap<APITag>::SetupProxyView(void) {

		Math::AffineF3 viewToWorld{ Math::LookatZ(Math::VecF3{ 0.f, 1.f, 0.f }, Math::VecF3{ 0.f, 0.f, 1.f }) };
		Math::AffineF3 worldToView{ Math::Transpose(viewToWorld) };

		Math::MatF44 projection{ Math::OrthoProjD3DStyle(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f) };

		this->m_View->Set_Matrices(worldToView, projection);
		this->m_View->UpdateCache();
	}

	template<RHI::APITagConcept APITag>
	inline bool PlanarShadowMap<APITag>::SetupDynamicDirectionalLightView(const DirectionalLight<APITag>& light, Math::VecF3 anchor, Math::VecF3 halfShadowBoxSize, Math::VecF3 preViewTranslation, float fadeRangeWorld) {
		Math::AffineD3 viewToWorld{ light.Get_Node()->Get_LocalToWorldTransform() };
		// Zero the translation component to ignore where the actual light scene node is located, we only care about direction
		viewToWorld.m_Translation = Math::VecD3::Zero();
		viewToWorld = Math::Scaling(Math::VecD3{ 1.0, 1.0, -1.0 }) * viewToWorld;
		Math::AffineF3 worldToView{ Math::AffineF3{ Math::Inverse(viewToWorld) } };

		// Snap the anchor to the texel grid to avoid jitter
		Math::VecF2 texelSize{ Math::VecF2{ 2.f } *halfShadowBoxSize.XY() / this->m_ShadowMapSize };

		Math::VecF3 anchorView{ worldToView.TransformPoint(anchor - preViewTranslation) };
		anchorView.XY() = Math::VecF2{ Math::Round(anchorView.XY() / texelSize) }*texelSize;
		Math::VecF3 center{ Math::VecF3{ viewToWorld.TransformPoint(Math::VecD3{ anchorView }) } + preViewTranslation };

		// Make sure we didn't move the anchor too far
		ASSERT(Math::Length(center - anchor) < Math::Max(texelSize.X, texelSize.Y) * 2);

		worldToView = Math::Translation(-center) * worldToView;

		// Build the projection matrix
		Math::MatF44 projection{ Math::OrthoProjD3DStyle(
			-halfShadowBoxSize.X, halfShadowBoxSize.X,
			-halfShadowBoxSize.Y, halfShadowBoxSize.Y,
			-halfShadowBoxSize.Z, halfShadowBoxSize.Z)
		};

		bool viewIsModified{
			this->m_View->Get_ViewMatrix() != worldToView ||
			Math::Any(this->m_View->Get_ProjectionMatrix(false) != projection)
		};

		this->m_View->Set_Matrices(worldToView, projection);
		this->m_View->UpdateCache();

		this->m_FadeRangeTexels = Math::Clamp(
			Math::VecF2{ (fadeRangeWorld * this->m_ShadowMapSize) / (halfShadowBoxSize.XY() * 2.f) },
			Math::VecF2{ 1.f },
			this->m_ShadowMapSize * 0.5f
		);

		return viewIsModified;
	}
}