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

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"


#endif // PARTING_MODULE_BUILD

namespace Parting {

	enum class  ViewType :Uint8 {
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
		/*STDNODISCARD virtual Uint32 GetNumChildViews(ViewType supportedTypes) const = 0;
		STDNODISCARD virtual const IView* GetChildView(ViewType supportedTypes, Uint32 index) const = 0;*/


	};

	class IView : public ICompositeView {
	public:
		IView(void) = default;
		virtual ~IView(void) = default;
	};

	class PlanarView final : public IView {
	public:
		PlanarView(void) = default;
		~PlanarView(void) = default;

	public:
		void Set_Viewport(const RHI::RHIViewport& viewport);
		void Set_ArraySlice(Uint32 arraySlice);
		void Set_Matrices(const Math::AffineF3& viewMatrix, const Math::MatF44& projMatrix);

		void UpdateCache(void);

	private:
		RHI::RHIViewport m_Viewport;
		RHI::RHIRect2D m_ScissorRect;
		Uint32 m_ArraySlice{ 0 };
		RHI::RHIVariableRateShadingState m_ShadingRateState;
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
	};

	class CompositeView : public ICompositeView {
	public:
		CompositeView(void) = default;
		~CompositeView(void) = default;

	public:
		void AddView(SharedPtr<IView> view);

	private:
		Vector<SharedPtr<IView>> m_ChildViews;
	};


	void PlanarView::Set_Viewport(const RHI::RHIViewport& viewport) {
		this->m_Viewport = viewport;
		this->m_ScissorRect = RHI::BuildScissorRect(viewport);
		this->m_CacheValid = false;
	}

	void PlanarView::Set_ArraySlice(Uint32 arraySlice) {
		this->m_ArraySlice = arraySlice;
	}

	void CompositeView::AddView(SharedPtr<IView> view) {
		this->m_ChildViews.push_back(view);
	}

	inline void PlanarView::Set_Matrices(const Math::AffineF3& viewMatrix, const Math::MatF44& projMatrix) {
		this->m_ViewMatrix = viewMatrix;
		this->m_ProjMatrix = projMatrix;
		this->m_CacheValid = false;
	}

	inline void PlanarView::UpdateCache(void) {
		if (this->m_CacheValid)
			return;

		this->m_PixelOffsetMatrix = Math::AffineToHomogeneous(Math::Translation(Math::VecF3{
				2.f * this->m_PixelOffset.X / (this->m_Viewport.MaxX - m_Viewport.MinX),
				-2.f * this->m_PixelOffset.Y / (this->m_Viewport.MaxY - this->m_Viewport.MinY),
				0.f
			})
		);
		this->m_PixelOffsetMatrixInv = Inverse(this->m_PixelOffsetMatrix);

		this->m_ViewProjMatrix = AffineToHomogeneous(this->m_ViewMatrix) * this->m_ProjMatrix;
		this->m_ViewProjOffsetMatrix = this->m_ViewProjMatrix * this->m_PixelOffsetMatrix;

		this->m_ViewMatrixInv = Inverse(this->m_ViewMatrix);
		this->m_ProjMatrixInv = Inverse(this->m_ProjMatrix);
		this->m_ViewProjMatrixInv = m_ProjMatrixInv * AffineToHomogeneous(this->m_ViewMatrixInv);
		this->m_ViewProjOffsetMatrixInv = this->m_PixelOffsetMatrixInv * this->m_ViewProjMatrixInv;

		this->m_ReverseDepth = (this->m_ProjMatrix[2][2] == 0.f);
		this->m_ViewFrustum = Math::Frustum{ this->m_ViewProjMatrix, this->m_ReverseDepth };
		m_ProjectionFrustum = Math::Frustum{ this->m_ProjMatrix, this->m_ReverseDepth };

		this->m_IsMirrored = Math::Determinant(this->m_ViewMatrix.m_Linear) < 0.f;

		this->m_CacheValid = true;
	}

}