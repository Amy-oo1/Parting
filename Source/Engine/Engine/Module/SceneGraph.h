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

#endif // PARTING_MODULE_BUILD#pragma once

//namespace Parting {
//
//	class SceneGraph;
//	class SceneGraphNode;
//	class SceneTypeFactory;
//
//	enum class SceneContentFlags : Uint32{
//		None = 0,
//		OpaqueMeshes = 0x01,
//		AlphaTestedMeshes = 0x02,
//		BlendedMeshes = 0x04,
//		Lights = 0x08,
//		Cameras = 0x10,
//		Animations = 0x20
//	};
//	EXPORT_ENUM_CLASS_OPERATORS(SceneContentFlags);
//
//	class SceneGraphNode final : public EnableSharedFromThis<SceneGraphNode> {
//	public:
//		SceneGraphNode(void) = default;
//		~SceneGraphNode(void) = default;
//
//	public:
//
//
//	private:
//
//	};
//}
