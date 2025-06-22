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


PARTING_MODULE(ShadowMap)


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

#include "Shader/view_cb.h"


#endif // PARTING_MODULE_BUILD

namespace Parting {

	enum class  ViewType :Uint8 {
		None = 0x00,

		PLANAR = 0x01,
		STEREO = 0x02,
		CUBEMAP = 0x04
	};
	EXPORT_ENUM_CLASS_OPERATORS(ViewType);

	class IView;

	class ICompositeView {
	public:
		ICompositeView(void) = default;
		virtual ~ICompositeView(void) = default;

	public:
		STDNODISCARD virtual Uint32 Get_NumChildViews(ViewType supportedTypes) const = 0;
		STDNODISCARD virtual const IView* Get_ChildView(ViewType supportedTypes, Uint32 index) const = 0;

	};

	class IView : public ICompositeView {
	public:
		IView(void) = default;
		virtual ~IView(void) = default;

	public:

	public:

	public:
		STDNODISCARD Uint32 Get_NumChildViews(ViewType supportedTypes) const override;
		STDNODISCARD const IView* Get_ChildView(ViewType supportedTypes, Uint32 index) const override;

	public:
		virtual	void FillPlanarViewConstants(Shader::PlanarViewConstants& constants) const;

	public:

		STDNODISCARD virtual RHI::RHIViewportState Get_ViewportState(void) const = 0;
		STDNODISCARD virtual RHI::RHITextureSubresourceSet Get_Subresources(void) const = 0;
		STDNODISCARD virtual bool Is_ReverseDepth(void) const = 0;
		STDNODISCARD virtual bool Is_OrthographicProjection(void) const = 0;
		STDNODISCARD virtual bool Is_StereoView(void) const = 0;
		STDNODISCARD virtual bool Is_CubemapView(void) const = 0;
		STDNODISCARD virtual bool Is_BoxVisible(const Math::BoxF3& bbox) const = 0;
		STDNODISCARD virtual bool Is_Mirrored(void) const = 0;
		STDNODISCARD virtual Math::VecF3 Get_ViewOrigin(void) const = 0;
		STDNODISCARD virtual Math::VecF3 Get_ViewDirection(void) const = 0;
		STDNODISCARD virtual Math::Frustum Get_ViewFrustum(void) const = 0;
		STDNODISCARD virtual Math::Frustum Get_ProjectionFrustum(void) const = 0;
		STDNODISCARD virtual Math::AffineF3 Get_ViewMatrix(void) const = 0;
		STDNODISCARD virtual Math::AffineF3 Get_InverseViewMatrix(void) const = 0;
		STDNODISCARD virtual Math::MatF44 Get_ProjectionMatrix(bool includeOffset = true) const = 0;
		STDNODISCARD virtual Math::MatF44 Get_InverseProjectionMatrix(bool includeOffset = true) const = 0;
		STDNODISCARD virtual Math::MatF44 Get_ViewProjectionMatrix(bool includeOffset = true) const = 0;
		STDNODISCARD virtual Math::MatF44 Get_InverseViewProjectionMatrix(bool includeOffset = true) const = 0;
		STDNODISCARD virtual RHI::RHIRect2D Get_ViewExtent(void) const = 0;
		STDNODISCARD virtual Math::VecF2 Get_PixelOffset(void) const = 0;
	};

	class PlanarView final : public IView {
	public:
		PlanarView(void) = default;
		~PlanarView(void) = default;

	public:
		STDNODISCARD const RHI::RHIViewport& Get_Viewport(void) const { return this->m_Viewport; }
		STDNODISCARD const RHI::RHIRect2D& Get_ScissorRect(void) const { return this->m_ScissorRect; }


		void Set_Viewport(const RHI::RHIViewport& viewport);
		void Set_ArraySlice(Uint32 arraySlice);
		void Set_Matrices(const Math::AffineF3& viewMatrix, const Math::MatF44& projMatrix);
		void Set_PixelOffset(const Math::VecF2 offset);

		void UpdateCache(void);

		void EnsureCacheIsValid(void) const { ASSERT(this->m_CacheValid); }

	private:
		RHI::RHIViewport m_Viewport;
		RHI::RHIRect2D m_ScissorRect;
		Uint32 m_ArraySlice{ 0 };
		Math::AffineF3 m_ViewMatrix{ Math::AffineF3::Identity() };
		Math::MatF44 m_ProjMatrix{ Math::MatF44::Identity() };
		Math::VecF2 m_PixelOffset{ Math::VecF2::Zero() };

		// Derived matrices and other information - computed and cached on access
		Math::MatF44 m_PixelOffsetMatrix{ Math::MatF44::Identity() };
		Math::MatF44 m_PixelOffsetMatrixInv{ Math::MatF44::Identity() };
		Math::MatF44 m_ViewProjMatrix = Math::MatF44::Identity();
		Math::MatF44 m_ViewProjOffsetMatrix{ Math::MatF44::Identity() };
		Math::AffineF3 m_ViewMatrixInv{ Math::AffineF3::Identity() };
		Math::MatF44 m_ProjMatrixInv{ Math::MatF44::Identity() };
		Math::MatF44 m_ViewProjMatrixInv{ Math::MatF44::Identity() };
		Math::MatF44 m_ViewProjOffsetMatrixInv{ Math::MatF44::Identity() };

		Math::Frustum m_ViewFrustum{ Math::Frustum::Empty() };
		Math::Frustum m_ProjectionFrustum{ Math::Frustum::Empty() };

		bool m_ReverseDepth{ false };
		bool m_IsMirrored{ false };
		bool m_CacheValid{ false };

	public:
		STDNODISCARD RHI::RHIViewportState Get_ViewportState(void) const override;
		STDNODISCARD RHI::RHITextureSubresourceSet Get_Subresources(void) const override;
		STDNODISCARD bool Is_ReverseDepth(void) const override;
		STDNODISCARD bool Is_OrthographicProjection(void) const override;
		STDNODISCARD bool Is_StereoView(void) const override;
		STDNODISCARD bool Is_CubemapView(void) const override;
		STDNODISCARD bool Is_BoxVisible(const Math::BoxF3& bbox) const override;
		STDNODISCARD bool Is_Mirrored(void) const override;
		STDNODISCARD Math::VecF3 Get_ViewOrigin(void) const override;
		STDNODISCARD Math::VecF3 Get_ViewDirection(void) const override;
		STDNODISCARD Math::Frustum Get_ViewFrustum(void) const override;
		STDNODISCARD Math::Frustum Get_ProjectionFrustum(void) const override;
		STDNODISCARD Math::AffineF3 Get_ViewMatrix(void) const override;
		STDNODISCARD Math::AffineF3 Get_InverseViewMatrix(void) const override;
		STDNODISCARD Math::MatF44 Get_ProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD Math::MatF44 Get_InverseProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD Math::MatF44 Get_ViewProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD Math::MatF44 Get_InverseViewProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD RHI::RHIRect2D Get_ViewExtent(void) const override;
		STDNODISCARD Math::VecF2 Get_PixelOffset(void) const override;

	};

	class CompositeView : public ICompositeView {
	public:
		CompositeView(void) = default;
		~CompositeView(void) = default;

	public:
		void AddView(SharedPtr<IView> view);

	private:
		Vector<SharedPtr<IView>> m_ChildViews;

	public:
		STDNODISCARD virtual Uint32 Get_NumChildViews(ViewType supportedTypes) const override;
		STDNODISCARD virtual const IView* Get_ChildView(ViewType supportedTypes, Uint32 index) const override;
	};

	template<typename ChildType>
	class StereoView final : public IView {
	public:
		StereoView(void) = default;
		~StereoView(void) = default;

	public:
		ChildType LeftView;
		ChildType RightView;

	public:
		STDNODISCARD Uint32 Get_NumChildViews(ViewType supportedTypes) const override;
		STDNODISCARD const IView* Get_ChildView(ViewType supportedTypes, Uint32 index) const override;

	public:
		STDNODISCARD RHI::RHIViewportState Get_ViewportState(void) const override;
		STDNODISCARD RHI::RHITextureSubresourceSet Get_Subresources(void) const override;
		STDNODISCARD bool Is_ReverseDepth(void) const override;
		STDNODISCARD bool Is_OrthographicProjection(void) const override;
		STDNODISCARD bool Is_StereoView(void) const override;
		STDNODISCARD bool Is_CubemapView(void) const override;
		STDNODISCARD bool Is_BoxVisible(const Math::BoxF3& bbox) const override;
		STDNODISCARD bool Is_Mirrored(void) const override;
		STDNODISCARD Math::VecF3 Get_ViewOrigin(void) const override;
		STDNODISCARD Math::VecF3 Get_ViewDirection(void) const override;
		STDNODISCARD Math::Frustum Get_ViewFrustum(void) const override;
		STDNODISCARD Math::Frustum Get_ProjectionFrustum(void) const override;
		STDNODISCARD Math::AffineF3 Get_ViewMatrix(void) const override { LOG_ERROR("err call"); return Math::AffineF3::Identity(); }
		STDNODISCARD Math::AffineF3 Get_InverseViewMatrix(void) const override { LOG_ERROR("err call"); return Math::AffineF3::Identity(); }
		STDNODISCARD Math::MatF44 Get_ProjectionMatrix(bool includeOffset = true) const override { LOG_ERROR("err call"); return Math::MatF44::Identity(); }
		STDNODISCARD Math::MatF44 Get_InverseProjectionMatrix(bool includeOffset = true) const override { LOG_ERROR("err call"); return Math::MatF44::Identity(); }
		STDNODISCARD Math::MatF44 Get_ViewProjectionMatrix(bool includeOffset = true) const override { LOG_ERROR("err call"); return Math::MatF44::Identity(); }
		STDNODISCARD Math::MatF44 Get_InverseViewProjectionMatrix(bool includeOffset = true) const override { LOG_ERROR("err call"); return Math::MatF44::Identity(); }
		STDNODISCARD RHI::RHIRect2D Get_ViewExtent(void) const override;
		STDNODISCARD Math::VecF2 Get_PixelOffset(void) const override;


	};

	using StereoPlanarView = StereoView<PlanarView>;

	class CubemapView : public IView {
	public:
		CubemapView(void) = default;
		~CubemapView(void) = default;

	public:
		void Set_Transform(Math::AffineF3 viewMatrix, float zNear, float cullDistance, bool useReverseInfiniteProjections = true);
		void Set_ArrayViewports(Uint32 resolution, Uint32 firstArraySlice);
		void UpdateCache(void);

		STDNODISCARD float Get_NearPlane(void) const;
		STDNODISCARD Math::BoxF3 Get_CullingBox(void) const;

		static Uint32* Get_CubemapCoordinateSwizzle(void);

	protected:
		void EnsureCacheIsValid(void) const;

	protected:
		Array<PlanarView, 6> m_FaceViews;
		Math::AffineF3 m_ViewMatrix{ Math::AffineF3::Identity() };
		Math::AffineF3 m_ViewMatrixInv{ Math::AffineF3::Identity() };
		Math::MatF44 m_ProjMatrix{ Math::MatF44::Identity() };
		Math::MatF44 m_ProjMatrixInv{ Math::MatF44::Identity() };
		Math::MatF44 m_ViewProjMatrix{ Math::MatF44::Identity() };
		Math::MatF44 m_ViewProjMatrixInv{ Math::MatF44::Identity() };

		float m_CullDistance{ 1.f };
		float m_NearPlane{ 1.f };
		Math::VecF3 m_Center{ Math::VecF3::Zero() };
		Math::BoxF3 m_CullingBox{ Math::BoxF3::Empty() };
		Uint32 m_FirstArraySlice{ 0 };
		bool m_CacheValid{ false };


	public:
		STDNODISCARD Uint32 Get_NumChildViews(ViewType supportedTypes) const override;
		STDNODISCARD const IView* Get_ChildView(ViewType supportedTypes, Uint32 index) const override;

	public:
		STDNODISCARD virtual RHI::RHIViewportState Get_ViewportState(void) const override;
		STDNODISCARD virtual RHI::RHITextureSubresourceSet Get_Subresources(void) const override;
		STDNODISCARD virtual bool Is_ReverseDepth(void) const override;
		STDNODISCARD virtual bool Is_OrthographicProjection(void) const override;
		STDNODISCARD virtual bool Is_StereoView(void) const override;
		STDNODISCARD virtual bool Is_CubemapView(void) const override;
		STDNODISCARD virtual bool Is_BoxVisible(const Math::BoxF3& bbox) const override;
		STDNODISCARD virtual bool Is_Mirrored(void) const override;
		STDNODISCARD virtual Math::VecF3 Get_ViewOrigin(void) const override;
		STDNODISCARD virtual Math::VecF3 Get_ViewDirection(void) const override;
		STDNODISCARD virtual Math::Frustum Get_ViewFrustum(void) const override;
		STDNODISCARD virtual Math::Frustum Get_ProjectionFrustum(void) const override;
		STDNODISCARD virtual Math::AffineF3 Get_ViewMatrix(void) const override;
		STDNODISCARD virtual Math::AffineF3 Get_InverseViewMatrix(void) const override;
		STDNODISCARD virtual Math::MatF44 Get_ProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD virtual Math::MatF44 Get_InverseProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD virtual Math::MatF44 Get_ViewProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD virtual Math::MatF44 Get_InverseViewProjectionMatrix(bool includeOffset = true) const override;
		STDNODISCARD virtual RHI::RHIRect2D Get_ViewExtent(void) const override;
		STDNODISCARD virtual Math::VecF2 Get_PixelOffset(void) const override;

	};

	constexpr Array<Math::MatF33, 6> g_CubemapViewMatrices{
	Math::MatF33{
		0.f,	0.f,	1.f,
		0.f,	1.f,	0.f,
		-1.f,	0.f,	0.f
	},
	Math::MatF33{
		0.f,	0.f,	-1.f,
		0.f,	1.f,	0.f,
		1.f,	0.f,	0.f
	},
	Math::MatF33{
		1.f,	0.f,	0.f,
		0.f,	0.f,	1.f,
		0.f,	-1.f,	0.f
	},
	Math::MatF33{
		1.f,	0.f,	0.f,
		0.f,	0.f,	-1.f,
		0.f,	1.f,	0.f
	},
	Math::MatF33{
		1.f,	0.f,	0.f,
		0.f,	1.f,	0.f,
		0.f,	0.f,	1.f
	},
	Math::MatF33{
		-1.f,	0.f,	0.f,
		0.f,	1.f,	0.f,
		0.f,	0.f,	-1.f
	}
	};



	inline Uint32 IView::Get_NumChildViews(ViewType supportedTypes) const { return 1; }

	inline const IView* IView::Get_ChildView(ViewType supportedTypes, Uint32 index) const { return this; }


	inline void IView::FillPlanarViewConstants(Shader::PlanarViewConstants& constants) const {
		constants.MatWorldToView = Math::AffineToHomogeneous(this->Get_ViewMatrix());
		constants.MatViewToClip = this->Get_ProjectionMatrix(true);
		constants.MatWorldToClip = this->Get_ViewProjectionMatrix(true);
		constants.MatClipToView = this->Get_InverseProjectionMatrix(true);
		constants.MatViewToWorld = Math::AffineToHomogeneous(this->Get_InverseViewMatrix());
		constants.MatClipToWorld = this->Get_InverseViewProjectionMatrix(true);
		constants.MatViewToClipNoOffset = this->Get_ProjectionMatrix(false);
		constants.MatWorldToClipNoOffset = this->Get_ViewProjectionMatrix(false);
		constants.MatClipToViewNoOffset = this->Get_InverseProjectionMatrix(false);
		constants.MatClipToWorldNoOffset = this->Get_InverseViewProjectionMatrix(false);

		auto viewportState{ this->Get_ViewportState() };
		const auto& viewport{ viewportState.Viewports[0] };
		constants.ViewportOrigin = Math::VecF2{ static_cast<float>(viewport.MinX,viewport.MinY) };
		constants.ViewportSize = Math::VecF2{ static_cast<float>(viewport.Width()), static_cast<float>(viewport.Height()) };
		constants.ViewportSizeInv = 1.f / constants.ViewportSize;

		constants.ClipToWindowScale = Math::VecF2{ 0.5f * static_cast<float>(viewport.Width()), -0.5f * static_cast<float>(viewport.Height()) };
		constants.ClipToWindowBias = constants.ViewportOrigin + constants.ViewportSize * 0.5f;

		constants.WindowToClipScale = 1.f / constants.ClipToWindowScale;
		constants.WindowToClipBias = -constants.ClipToWindowBias * constants.WindowToClipScale;

		constants.CameraDirectionOrPosition =
			this->Is_OrthographicProjection()
			? Math::VecF4{ this->Get_ViewDirection(), 0.f }
		: Math::VecF4{ this->Get_ViewOrigin(), 1.f };

		constants.PixelOffset = this->Get_PixelOffset();
	}

	void PlanarView::Set_Viewport(const RHI::RHIViewport& viewport) {
		this->m_Viewport = viewport;
		this->m_ScissorRect = RHI::BuildScissorRect(viewport);
		this->m_CacheValid = false;
	}

	void PlanarView::Set_ArraySlice(Uint32 arraySlice) {
		this->m_ArraySlice = arraySlice;
	}


	inline RHI::RHIViewportState PlanarView::Get_ViewportState(void) const { return RHI::RHIViewportStateBuilder{}.AddViewport(this->m_Viewport).AddScissorRect(this->m_ScissorRect).Build(); }

	inline RHI::RHITextureSubresourceSet PlanarView::Get_Subresources(void) const { return RHI::RHITextureSubresourceSet{ .BaseArraySlice{ this->m_ArraySlice } }; }

	inline bool PlanarView::Is_ReverseDepth(void) const { this->EnsureCacheIsValid(); return this->m_ReverseDepth; }

	inline bool PlanarView::Is_OrthographicProjection(void) const { return this->m_ProjMatrix[2][2] == 0.f; }

	inline  bool PlanarView::Is_StereoView(void) const { return false; }

	inline bool PlanarView::Is_CubemapView(void) const { return false; }

	inline bool PlanarView::Is_BoxVisible(const Math::BoxF3& bbox) const { this->EnsureCacheIsValid(); return this->m_ViewFrustum.IntersectsWith(bbox); }

	inline bool PlanarView::Is_Mirrored(void) const { this->EnsureCacheIsValid(); return this->m_IsMirrored; }

	inline Math::VecF3 PlanarView::Get_ViewOrigin(void) const { this->EnsureCacheIsValid(); return this->m_ViewMatrixInv.m_Translation; }

	inline Math::VecF3 PlanarView::Get_ViewDirection(void) const { this->EnsureCacheIsValid(); return this->m_ViewMatrixInv.m_Linear[2]; }

	inline  Math::Frustum PlanarView::Get_ViewFrustum(void) const { this->EnsureCacheIsValid(); return this->m_ViewFrustum; }

	inline Math::Frustum PlanarView::Get_ProjectionFrustum(void) const { this->EnsureCacheIsValid(); return this->m_ProjectionFrustum; }

	inline Math::AffineF3 PlanarView::Get_ViewMatrix(void) const { return this->m_ViewMatrix; }

	inline Math::AffineF3 PlanarView::Get_InverseViewMatrix(void) const { this->EnsureCacheIsValid(); return this->m_ViewMatrixInv; }

	inline Math::MatF44 PlanarView::Get_ProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return includeOffset ? this->m_ProjMatrix * this->m_PixelOffsetMatrix : this->m_ProjMatrix; }

	inline Math::MatF44 PlanarView::Get_InverseProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return includeOffset ? this->m_ProjMatrixInv * this->m_PixelOffsetMatrixInv : this->m_ProjMatrixInv; }

	inline Math::MatF44 PlanarView::Get_ViewProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return includeOffset ? this->m_ViewProjOffsetMatrix : this->m_ViewProjMatrix; }

	inline Math::MatF44 PlanarView::Get_InverseViewProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return includeOffset ? this->m_ViewProjOffsetMatrixInv : this->m_ViewProjMatrixInv; }

	inline RHI::RHIRect2D PlanarView::Get_ViewExtent(void) const { return this->m_ScissorRect; }

	inline Math::VecF2 PlanarView::Get_PixelOffset(void) const { return this->m_PixelOffset; }

	void CompositeView::AddView(SharedPtr<IView> view) {
		this->m_ChildViews.push_back(view);
	}

	inline Uint32 CompositeView::Get_NumChildViews(ViewType supportedTypes) const { return static_cast<Uint32>(this->m_ChildViews.size()); }

	inline const IView* CompositeView::Get_ChildView(ViewType supportedTypes, Uint32 index) const { return this->m_ChildViews[index].get(); }

	inline void PlanarView::Set_Matrices(const Math::AffineF3& viewMatrix, const Math::MatF44& projMatrix) {
		this->m_ViewMatrix = viewMatrix;
		this->m_ProjMatrix = projMatrix;
		this->m_CacheValid = false;
	}

	inline void PlanarView::Set_PixelOffset(const Math::VecF2 offset) {
		this->m_PixelOffset = offset;
		this->m_CacheValid = false;
	}

	inline void PlanarView::UpdateCache(void) {
		if (this->m_CacheValid)
			return;

		this->m_PixelOffsetMatrix = Math::AffineToHomogeneous(Math::Translation(Math::VecF3{
				2.f * this->m_PixelOffset.X / (this->m_Viewport.MaxX - this->m_Viewport.MinX),
				-2.f * this->m_PixelOffset.Y / (this->m_Viewport.MaxY - this->m_Viewport.MinY),
				0.f
			})
		);
		this->m_PixelOffsetMatrixInv = Math::Inverse(this->m_PixelOffsetMatrix);

		this->m_ViewProjMatrix = Math::AffineToHomogeneous(this->m_ViewMatrix) * this->m_ProjMatrix;
		this->m_ViewProjOffsetMatrix = this->m_ViewProjMatrix * this->m_PixelOffsetMatrix;

		this->m_ViewMatrixInv = Math::Inverse(this->m_ViewMatrix);
		this->m_ProjMatrixInv = Math::Inverse(this->m_ProjMatrix);
		this->m_ViewProjMatrixInv = this->m_ProjMatrixInv * Math::AffineToHomogeneous(this->m_ViewMatrixInv);
		this->m_ViewProjOffsetMatrixInv = this->m_PixelOffsetMatrixInv * this->m_ViewProjMatrixInv;

		this->m_ReverseDepth = (0.f == this->m_ProjMatrix[2][2]);
		this->m_ViewFrustum = Math::Frustum{ this->m_ViewProjMatrix, this->m_ReverseDepth };
		this->m_ProjectionFrustum = Math::Frustum{ this->m_ProjMatrix, this->m_ReverseDepth };

		this->m_IsMirrored = Math::Determinant(this->m_ViewMatrix.m_Linear) < 0.f;

		this->m_CacheValid = true;
	}

	template<typename ChildType>inline Uint32 StereoView<ChildType>::Get_NumChildViews(ViewType supportedTypes) const { return (ViewType::None != (supportedTypes & ViewType::STEREO)) ? 1u : 2u; }

	template<typename ChildType>inline const IView* StereoView<ChildType>::Get_ChildView(ViewType supportedTypes, Uint32 index) const {
		if (ViewType::None != (supportedTypes & ViewType::STEREO)) {
			ASSERT(index == 0);
			return this;
		}

		ASSERT(index < 2);
		if (index == 0)
			return &this->LeftView;
		return &this->RightView;

		/*return (ViewType::None != (supportedTypes & ViewType::STEREO)) ? this : (index == 0 ? (&this->LeftView) : (&this->RightView)); */
	}

	template<typename ChildType>inline RHI::RHIViewportState StereoView<ChildType>::Get_ViewportState(void) const {
		RHI::RHIViewportState left{ LeftView.Get_ViewportState() };
		RHI::RHIViewportState right{ RightView.Get_ViewportState() };

		for (const auto& viewport : Span<RHI::RHIViewport>{ right.Viewports.data(),right.ViewportCount })
			left.Viewports[left.ViewportCount++] = viewport;
		for (const auto& scissor : Span<RHI::RHIRect2D>{ right.ScissorRects.data(),right.ScissorCount })
			left.ScissorRects[left.ScissorCount++] = scissor;

		return left;
	}

	template<typename ChildType> inline RHI::RHITextureSubresourceSet StereoView<ChildType>::Get_Subresources(void) const { return this->LeftView.Get_Subresources();/*TODO */ }

	template<typename ChildType> inline bool StereoView<ChildType>::Is_ReverseDepth(void) const { return this->LeftView.Is_ReverseDepth(); }

	template<typename ChildType> inline bool StereoView<ChildType>::Is_OrthographicProjection(void) const { return LeftView.Is_OrthographicProjection(); }

	template<typename ChildType> inline bool StereoView<ChildType>::Is_StereoView(void) const { return true; }

	template<typename ChildType> inline bool StereoView<ChildType>::Is_CubemapView(void) const { return true; }

	template<typename ChildType>inline bool StereoView<ChildType>::Is_BoxVisible(const Math::BoxF3& bbox) const { return this->LeftView.Is_BoxVisible(bbox) || this->RightView.Is_BoxVisible(bbox); }

	template<typename ChildType> inline bool StereoView<ChildType>::Is_Mirrored(void) const { return this->LeftView.Is_Mirrored(); }

	template<typename ChildType> inline Math::VecF3 StereoView<ChildType>::Get_ViewOrigin(void) const { return (this->LeftView.Get_ViewOrigin() + this->RightView.Get_ViewOrigin()) * 0.5f; }

	template<typename ChildType> inline Math::VecF3 StereoView<ChildType>::Get_ViewDirection(void) const { return this->LeftView.Get_ViewDirection(); }

	template<typename ChildType> inline Math::Frustum StereoView<ChildType>::Get_ViewFrustum(void) const {
		auto left{ this->LeftView.Get_ViewFrustum() };
		auto right{ this->RightView.Get_ViewFrustum() };

		// not robust but should work for regular stereo views
		left.m_Planes[Tounderlying(Math::Frustum::PlaneType::Right)] = right.m_Planes[Tounderlying(Math::Frustum::PlaneType::Left)];

		return left;
	}

	template<typename ChildType>inline Math::Frustum StereoView<ChildType>::Get_ProjectionFrustum(void) const {
		auto left{ this->LeftView.Get_ProjectionFrustum() };
		auto right{ this->RightView.Get_ProjectionFrustum() };

		// not robust but should work for regular stereo views
		left.m_Planes[Tounderlying(Math::Frustum::PlaneType::Right)] = right.m_Planes[Tounderlying(Math::Frustum::PlaneType::Left)];

		return left;
	}

	template<typename ChildType> inline RHI::RHIRect2D StereoView<ChildType>::Get_ViewExtent(void) const {
		//auto left{ this->LeftView.Get_ViewExtent() };
		//auto right{ this->RightView.Get_ViewExtent() };

		//return RHI::RHIRect2D::Merge(left, right);//TODO 

		ASSERT(false);
	}

	template<typename ChildType> inline Math::VecF2 StereoView<ChildType>::Get_PixelOffset(void) const { return this->LeftView.Get_PixelOffset(); }


	inline void CubemapView::Set_Transform(Math::AffineF3 viewMatrix, float zNear, float cullDistance, bool useReverseInfiniteProjections) {
		this->m_ViewMatrix = viewMatrix;
		this->m_NearPlane = zNear;
		this->m_CullDistance = cullDistance;

		Math::MatF44 faceProjMatrix;
		if (useReverseInfiniteProjections)
			faceProjMatrix = Math::MatF44(
				1.f,	0.f,	0.f,	0.f,
				0.f,	1.f,	0.f,	0.f,
				0.f,	0.f,	0.f,	1.f,
				0.f,	0.f,	zNear,	0.f
			);
		else
			faceProjMatrix = Math::PerspProjD3DStyle(-1.f, 1.f, -1.f, 1.f, zNear, cullDistance);

		for (Uint32 face = 0; face < 6; ++face) {
			Math::AffineF3 faceViewMatrix{ this->m_ViewMatrix * Math::AffineF3{ g_CubemapViewMatrices[face], Math::VecF3::Zero() } };

			this->m_FaceViews[face].Set_Matrices(faceViewMatrix, faceProjMatrix);
		}

		this->m_CacheValid = false;
	}

	inline void CubemapView::Set_ArrayViewports(Uint32 resolution, Uint32 firstArraySlice) {
		this->m_FirstArraySlice = firstArraySlice;

		for (int face = 0; face < 6; ++face) {
			this->m_FaceViews[face].Set_Viewport(RHI::RHIViewport::Build(static_cast<float>(resolution), static_cast<float>(resolution)));
			this->m_FaceViews[face].Set_ArraySlice(face + firstArraySlice);
		}
	}

	inline void CubemapView::UpdateCache(void) {
		for (auto& view : this->m_FaceViews)
			view.UpdateCache();

		if (this->m_CacheValid)
			return;

		this->m_ViewMatrixInv = Math::Inverse(this->m_ViewMatrix);
		this->m_ProjMatrix = Math::AffineToHomogeneous(Math::Scaling<float, 3>(1.0f / this->m_NearPlane));
		this->m_ProjMatrixInv = Math::Inverse(this->m_ProjMatrix);
		this->m_ViewProjMatrix = Math::AffineToHomogeneous(this->m_ViewMatrix) * this->m_ProjMatrix;
		this->m_ViewProjMatrixInv = Math::Inverse(this->m_ViewProjMatrix);
		this->m_Center = Math::Inverse(this->m_ViewMatrix).m_Translation;
		this->m_CullingBox = Math::BoxF3(this->m_Center - this->m_CullDistance, this->m_Center + this->m_CullDistance);

		this->m_CacheValid = true;
	}

	inline float CubemapView::Get_NearPlane(void) const { return this->m_NearPlane; }

	inline Math::BoxF3 CubemapView::Get_CullingBox(void) const { this->EnsureCacheIsValid(); return this->m_CullingBox; }

	inline Uint32* CubemapView::Get_CubemapCoordinateSwizzle(void) {
		ASSERT(false);

		return nullptr;
	}

	void CubemapView::EnsureCacheIsValid(void) const { ASSERT(this->m_CacheValid); }

	inline Uint32 CubemapView::Get_NumChildViews(ViewType supportedTypes) const { return ViewType::None != (supportedTypes & ViewType::CUBEMAP) ? 1 : 6; }

	inline const IView* CubemapView::Get_ChildView(ViewType supportedTypes, Uint32 index) const {
		if (ViewType::None != (supportedTypes & ViewType::CUBEMAP))
			return this;

		ASSERT(index < 6);
		return &this->m_FaceViews[index];
	}

	inline RHI::RHIViewportState CubemapView::Get_ViewportState(void) const {
		RHI::RHIViewportStateBuilder result;

		for (const auto& faceView : this->m_FaceViews) {
			result.AddViewport(faceView.Get_Viewport());
			result.AddScissorRect(faceView.Get_ScissorRect());
		}

		return result.Build();
	}

	inline RHI::RHITextureSubresourceSet CubemapView::Get_Subresources(void) const { return RHI::RHITextureSubresourceSet{ .BaseArraySlice{ this->m_FirstArraySlice }, .ArraySliceCount{ 6 } }; }

	inline bool CubemapView::Is_ReverseDepth(void) const { return true; }

	inline bool CubemapView::Is_OrthographicProjection(void) const { return false; }

	inline bool CubemapView::Is_StereoView(void) const { return false; }

	inline bool CubemapView::Is_CubemapView(void) const { return true; }

	inline bool CubemapView::Is_BoxVisible(const Math::BoxF3& bbox) const {
		this->EnsureCacheIsValid();

		if (this->m_CullDistance <= 0)
			return true;

		return this->m_CullingBox.Intersects(bbox);
	}

	inline bool CubemapView::Is_Mirrored(void) const { return false; }

	inline Math::VecF3 CubemapView::Get_ViewOrigin(void) const { return this->m_Center; }

	inline Math::VecF3 CubemapView::Get_ViewDirection(void) const { ASSERT(false); return 0.f; }

	inline Math::Frustum CubemapView::Get_ViewFrustum(void) const { this->EnsureCacheIsValid(); return Math::Frustum::FromBox(this->m_CullingBox); }

	inline Math::Frustum CubemapView::Get_ProjectionFrustum(void) const { this->EnsureCacheIsValid(); return Math::Frustum::FromBox(Math::BoxF3{ -this->m_CullDistance, this->m_CullDistance }); }

	inline Math::AffineF3 CubemapView::Get_ViewMatrix(void) const { return this->m_ViewMatrix; }

	inline Math::AffineF3 CubemapView::Get_InverseViewMatrix(void) const { this->EnsureCacheIsValid(); return this->m_ViewMatrixInv; }

	inline Math::MatF44 CubemapView::Get_ProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return this->m_ProjMatrix; }

	inline Math::MatF44 CubemapView::Get_InverseProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return this->m_ProjMatrixInv; }

	inline Math::MatF44 CubemapView::Get_ViewProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return this->m_ViewProjMatrix; }

	inline Math::MatF44 CubemapView::Get_InverseViewProjectionMatrix(bool includeOffset) const { this->EnsureCacheIsValid(); return this->m_ViewProjMatrixInv; }

	inline RHI::RHIRect2D CubemapView::Get_ViewExtent(void) const { return this->m_FaceViews[0].Get_ViewExtent(); }

	inline Math::VecF2 CubemapView::Get_PixelOffset(void) const { return Math::VecF2::Zero(); }

}