#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(BindingCache)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "ThirdParty/ShaderMake/include/ShaderMake/ShaderBlob.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/Logger/Module/Logger.h"

#include "Engine/Render/Module/Camera-Base.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	class FirstPersonCamera final :public BaseCamera<FirstPersonCamera> {
		friend class BaseCamera<FirstPersonCamera>;
	public:
		FirstPersonCamera(void) = default;
		~FirstPersonCamera(void) = default;

	public:
		void LookAt(Math::VecF3 cameraPos, Math::VecF3 cameraTarget, Math::VecF3 cameraUp = Math::VecF3{ 0.f, 1.f, 0.f });


	private:


	private:
		Math::VecF2 m_MousePos{ Math::VecF2::Zero() };
		Math::VecF2 m_MousePosPrev{ Math::VecF2::Zero() };
		Math::VecF2 m_MouseMotionAccumulator{ Math::VecF2::Zero() };
		Math::VecF3 m_CameraMovePrev{ Math::VecF3::Zero() };
		Math::VecF3 m_CameraMoveDamp{ Math::VecF3::Zero() };
		bool m_IsDragging{ false };


	private:


	};



	void FirstPersonCamera::LookAt(Math::VecF3 cameraPos, Math::VecF3 cameraTarget, Math::VecF3 cameraUp) {
		this->BaseLookAt(cameraPos, cameraTarget, cameraUp);

		this->m_MouseMotionAccumulator = 0.f;
		this->m_CameraMoveDamp = 0.f;
		this->m_CameraMovePrev = 0.f;

	}





}