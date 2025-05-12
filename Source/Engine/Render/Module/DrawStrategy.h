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


PARTING_MODULE(DrawStrategy)


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

#include "Engine/Engine/Module/SceneGraph.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	class IView;
	struct DrawItem;

	class IDrawStrategy{
	public:
		IDrawStrategy(void) = default;
		virtual ~IDrawStrategy(void) = default;

	public:
		/*virtual void PrepareForView(const SharedPtr<SceneGraphNode>& rootNode,const IView& view) = 0;

		virtual const DrawItem* GetNextItem(void) = 0;*/
	};

	class PassthroughDrawStrategy final : public IDrawStrategy {
	public:
		PassthroughDrawStrategy(void) = default;
		~PassthroughDrawStrategy(void) = default;
	};

	class InstancedOpaqueDrawStrategy final: public IDrawStrategy {
	public:
		InstancedOpaqueDrawStrategy(void) = default;
		~InstancedOpaqueDrawStrategy(void) = default;
	};

	class TransparentDrawStrategy : public IDrawStrategy {
	public:
		TransparentDrawStrategy(void) = default;
		~TransparentDrawStrategy(void) = default;
	};

}