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



	private:
		RHI::RHIViewport m_Viewport;
		RHI::RHIRect2D m_ScissorRect;
		Uint32 m_ArraySlice{ 0 };

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

}