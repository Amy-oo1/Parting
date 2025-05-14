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

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<typename Derived>
	class BaseCamera {
	protected:
		BaseCamera(void) = default;
		PARTING_VIRTUAL ~BaseCamera(void) = default;

	public:
		void Set_MoveSpeed(float value) { this->m_MoveSpeed = value; }
		void Set_RotateSpeed(float value) { this->m_RotateSpeed = value; }

	private:
	

	private:
		float m_MoveSpeed{ 1.f };		// movement speed in units/second
		float m_RotateSpeed{ 0.005f };	// mouse sensitivity in radians/pixel


	private:
		STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
		STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
	private:


	};
}