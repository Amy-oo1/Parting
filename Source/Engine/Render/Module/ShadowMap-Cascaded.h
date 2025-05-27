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

#include "Engine/Render/Module/ShadowMap-Base.h"
#include "Engine/Render/Module/ShadowMap-Planar.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class CascadedShadowMap final :public IShadowMap<APITag> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
	public:
		CascadedShadowMap(Imp_Device* device, Uint32 resolution, Uint32 numCascades, Uint32 numPerObjectShadows, RHI::RHIFormat format, bool isUAV = false);
		~CascadedShadowMap(void) = default;


	public:
		void SetupProxyView(void);

		// Computes the cascade projections based on the view frustum, shadow distance, and the distribution exponent.
		bool SetupForPlanarView(
			const DirectionalLight<APITag>& light,
			Math::Frustum viewFrustum,
			float maxShadowDistance,
			float lightSpaceZUp,
			float lightSpaceZDown,
			float exponent = 4.f,
			Math::VecF3 preViewTranslation = 0.f,
			Uint32 numberOfCascades = Max_Uint32
		);

		// Similar to SetupForPlanarView, but the size of the cascades does not depend on orientation, and therefore 
		// the shadow map texels have the same world space projections when the camera turns or moves.
		// The downside of this algorithm is that the cascades are often larger than necessary.
		bool SetupForPlanarViewStable(
			const DirectionalLight<APITag>& light,
			Math::Frustum projectionFrustum,
			Math::AffineF3 inverseViewMatrix,
			float maxShadowDistance,
			float lightSpaceZUp,
			float lightSpaceZDown,
			float exponent = 4.f,
			Math::VecF3 preViewTranslation = 0.f,
			Uint32 numberOfCascades = Max_Uint32
		);


		void Clear(Imp_CommandList* commandList);

	private:
		RHI::RefCountPtr<Imp_Texture> m_ShadowMapTexture;
		Vector<SharedPtr<PlanarShadowMap<APITag>>> m_Cascades;
		Vector<SharedPtr<PlanarShadowMap<APITag>>> m_PerObjectShadows;

		CompositeView m_CompositeView;

		Uint32 m_CascadeCount{ 0 };

	public:
		Math::MatF44 Get_WorldToUvzwMatrix(void) const override { ASSERT(false); return Math::MatF44::Identity(); }

		const class ICompositeView& Get_View(void) const override { return this->m_CompositeView; }

		auto Get_Texture(void) -> Imp_Texture* const override { return this->m_ShadowMapTexture.Get(); }

		Uint32 Get_NumberOfCascades(void) const override { return this->m_CascadeCount; }

		const IShadowMap<APITag>* Get_Cascade(Uint32 index) const override {
			ASSERT(index < this->m_CascadeCount);
			return this->m_Cascades[index].get();
		}

		Uint32 Get_NumberOfPerObjectShadows(void) const override { return static_cast<Uint32>(this->m_PerObjectShadows.size()); }

		const IShadowMap<APITag>* Get_PerObjectShadow(Uint32 index) const override {
			ASSERT(index < this->m_PerObjectShadows.size());
			return this->m_PerObjectShadows[index].get();
		}

		Math::VecU2 Get_TextureSize(void) const override {
			const auto& textureDesc{ this->m_ShadowMapTexture->Get_Desc() };
			return Math::VecU2{ textureDesc.Extent.Width, textureDesc.Extent.Height };
		}

		Math::BoxF2 Get_UVRange(void) const override {
			ASSERT(false);
			return Math::BoxF2{};
		}

		Math::VecF2 Get_FadeRangeInTexels(void) const override { return this->m_Cascades[0]->Get_FadeRangeInTexels(); }

		bool Is_LitOutOfBounds(void) const override { return this->m_Cascades[0]->Is_LitOutOfBounds(); }

		void FillShadowConstants(ShadowConstants& constants) const override { ASSERT(false); }


	};

	template<RHI::APITagConcept APITag>
	inline CascadedShadowMap<APITag>::CascadedShadowMap(Imp_Device* device, Uint32 resolution, Uint32 numCascades, Uint32 numPerObjectShadows, RHI::RHIFormat format, bool isUAV) {
		ASSERT(numCascades > 0);
		ASSERT(numCascades <= 4);

		RHI::RHITextureDesc desc{
			.Extent {.Width{ resolution }, .Height{ resolution } },
			.ArrayCount { numCascades + numPerObjectShadows },
			.Format { format },
			.Dimension { RHI::RHITextureDimension::Texture2DArray },
			.IsRenderTarget { true },
			.IsUAV { isUAV },
			.IsTypeless { true },
			.ClearValue {Color{ 1.f } },
			.InitialState { RHI::RHIResourceState::ShaderResource },
			.KeepInitialState { true },
		};
		this->m_ShadowMapTexture = device->CreateTexture(desc);

		auto cascadeViewport{ RHI::RHIViewport::Build(static_cast<float>(resolution), static_cast<float>(resolution)) };

		for (Uint32 cascade = 0; cascade < numCascades; ++cascade) {
			SharedPtr<PlanarShadowMap<APITag>> planarShadowMap{ MakeShared<PlanarShadowMap<APITag>>(device, this->m_ShadowMapTexture.Get(), cascade , cascadeViewport) };
			this->m_Cascades.push_back(planarShadowMap);

			this->m_CompositeView.AddView(planarShadowMap->Get_PlanarView());
		}


		for (Uint32 Object = 0; Object < numPerObjectShadows; ++Object) {
			this->m_PerObjectShadows.emplace_back(MakeShared<PlanarShadowMap<APITag>>(device, this->m_ShadowMapTexture.Get(), numCascades + Object, cascadeViewport));
			this->m_PerObjectShadows.back()->Set_FalloffDistance(0.f); // disable falloff on per-object shadows: their bboxes are short by design
		}
	}

	template<RHI::APITagConcept APITag>
	inline void CascadedShadowMap<APITag>::SetupProxyView(void) {
		for (auto& cascade : this->m_Cascades)
			cascade->SetupProxyView();

		for (auto& perObjectShadow : this->m_PerObjectShadows)
			perObjectShadow->SetupProxyView();
	}

	template<RHI::APITagConcept APITag>
	inline bool CascadedShadowMap<APITag>::SetupForPlanarViewStable(const DirectionalLight<APITag>& light, Math::Frustum projectionFrustum, Math::AffineF3 inverseViewMatrix, float maxShadowDistance, float lightSpaceZUp, float lightSpaceZDown, float exponent, Math::VecF3 preViewTranslation, Uint32 numberOfCascades) {
		ASSERT(exponent > 1);

		ASSERT(Math::Length(projectionFrustum.Get_NearPlane().m_Normal) > 0.f);

		this->m_CascadeCount = Math::Min(numberOfCascades, static_cast<Uint32>(this->m_Cascades.size()));

		ASSERT(this->m_CascadeCount > 0);

		if (maxShadowDistance > 0.f) {
			auto& nearPlane{ projectionFrustum.Get_NearPlane() };
			auto& farPlane{ projectionFrustum.Get_FarPlane() };
			farPlane.m_Normal = -nearPlane.m_Normal;
			farPlane.m_Distance = -nearPlane.m_Distance + maxShadowDistance;
		}
		else
			ASSERT(Math::Length(projectionFrustum.Get_FarPlane().m_Normal) > 0.f);

		Array<Math::VecF3, Math::Frustum::NumCorners> corners{};
		for (Uint32 Index = 0; Index < Math::Frustum::NumCorners; Index++)
			corners[Index] = projectionFrustum.Get_Corner(static_cast<Math::Frustum::Corners>(Index));

		float Far{ 1.f };
		float Near{ Far / exponent };

		bool viewModified{ false };

		for (Uint32 cascade = this->m_CascadeCount - 1; cascade >= 0; --cascade) {
			if (cascade == 0)
				Near = 0.f;

			Array<Math::VecF3, Math::Frustum::NumCorners> cascadeCorners{};
			for (Uint32 Index = 0; Index < Math::Frustum::NumCorners; ++Index)
				cascadeCorners[Index] = Math::Lerp(corners[Index & 3], corners[(Index & 3u) + 4u], (Index & 4u) ? Far : Near);

			Math::VecF3 nearDiagonalCenter{
				(cascadeCorners[Tounderlying(Math::Frustum::Corners::Bottom | Math::Frustum::Corners::Left | Math::Frustum::Corners::Near)] +
					cascadeCorners[Tounderlying(Math::Frustum::Corners::Top | Math::Frustum::Corners::Right | Math::Frustum::Corners::Near)])
					* 0.5f
			};
			Math::VecF3 farDiagonalCenter{
				(cascadeCorners[Tounderlying(Math::Frustum::Corners::Bottom | Math::Frustum::Corners::Left | Math::Frustum::Corners::Far)] +
					cascadeCorners[Tounderlying(Math::Frustum::Corners::Top | Math::Frustum::Corners::Right | Math::Frustum::Corners::Far)])
					* 0.5f
			};

			float nearCenterToFarCenter{ Math::Length(farDiagonalCenter - nearDiagonalCenter) };
			float nearDiagonalHalfLength{ Math::Length(
				cascadeCorners[Tounderlying(Math::Frustum::Corners::Bottom | Math::Frustum::Corners::Left | Math::Frustum::Corners::Near)] -
					cascadeCorners[Tounderlying(Math::Frustum::Corners::Top | Math::Frustum::Corners::Right | Math::Frustum::Corners::Near)])
						* 0.5f
			};
			float farDiagonalHalfLength{ Math::Length(
				cascadeCorners[Tounderlying(Math::Frustum::Corners::Bottom | Math::Frustum::Corners::Left | Math::Frustum::Corners::Far)] -
					cascadeCorners[Tounderlying(Math::Frustum::Corners::Top | Math::Frustum::Corners::Right | Math::Frustum::Corners::Far)])
						* 0.5f
			};

			float nearCenterToSphereCenter{ (Math::Square(nearCenterToFarCenter) + Math::Square(farDiagonalHalfLength) - Math::Square(nearDiagonalHalfLength)) / (2.f * nearCenterToFarCenter) };
			nearCenterToSphereCenter = Math::Clamp(nearCenterToSphereCenter, 0.f, nearCenterToFarCenter);

			Math::VecF3 sphereCenter{ nearDiagonalCenter + Math::Normalize(farDiagonalCenter - nearDiagonalCenter) * nearCenterToSphereCenter };
			float sphereRadius{ Math::Sqrt(Math::Square(farDiagonalHalfLength) + Math::Square(nearCenterToFarCenter - nearCenterToSphereCenter)) };

			Math::VecF3 cascadeCenter{ inverseViewMatrix.TransformPoint(sphereCenter) };

			Math::VecF3 halfShadowBoxSize{ sphereRadius };
			float fadeRange{ sphereRadius * 0.1f };

			float zDown{ Math::Max(sphereRadius, lightSpaceZDown) };
			float zUp{ Math::Max(sphereRadius, lightSpaceZUp) };
			cascadeCenter += Math::VecF3{ light.Get_Direction() * static_cast<double>((zDown - zUp) * 0.5f) };
			halfShadowBoxSize.Z = (zDown + zUp) * 0.5f;

			if (this->m_Cascades[cascade]->SetupDynamicDirectionalLightView(light, cascadeCenter, halfShadowBoxSize, preViewTranslation, fadeRange))
				viewModified = true;

			Far = Near;
			Near = Far / exponent;

			if (cascade == 0)
				break;//TODO : 
		}

		return viewModified;
	}

	template<RHI::APITagConcept APITag>
	inline void CascadedShadowMap<APITag>::Clear(Imp_CommandList* commandList) {
		const auto& depthFormatInfo{ RHI::Get_RHIFormatInfo(this->m_ShadowMapTexture->Get_Desc().Format) };

		commandList->ClearDepthStencilTexture(this->m_ShadowMapTexture, RHI::g_AllSubResourceSet, 1.f, depthFormatInfo.HasStencil ? Optional<Uint8>{  0 } : NullOpt);
	}

}