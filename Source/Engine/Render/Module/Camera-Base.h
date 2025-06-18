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
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/VectorMath/Module/VectorMath.h"
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
		STDNODISCARD const Math::AffineF3& Get_WorldToViewMatrix(void) const { return this->m_MatWorldToView; }
		STDNODISCARD const Math::AffineF3& Get_TranslatedWorldToViewMatrix(void) const { return this->m_MatTranslatedWorldToView; }
		STDNODISCARD const Math::VecF3& Get_Position(void) const { return this->m_CameraPos; }
		STDNODISCARD const Math::VecF3& Get_Dir(void) const { return this->m_CameraDir; }
		STDNODISCARD const Math::VecF3& Get_Up(void) const { return this->m_CameraUp; }




		void Set_MoveSpeed(float value) { this->m_MoveSpeed = value; }
		void Set_RotateSpeed(float value) { this->m_RotateSpeed = value; }




		void BaseLookAt(Math::VecF3 cameraPos, Math::VecF3 cameraTarget, Math::VecF3 cameraUp = Math::VecF3{ 0.f,1.f,0.f });

		void UpdateWorldToView(void);



	private:



	protected:
		Math::AffineF3 m_MatWorldToView{ Math::AffineF3::Identity() };
		Math::AffineF3 m_MatTranslatedWorldToView{ Math::AffineF3::Identity() };

		Math::VecF3 m_CameraPos{ 0.f };				// in worldspace
		Math::VecF3 m_CameraDir{ 1.f, 0.f, 0.f };	// normalized
		Math::VecF3 m_CameraUp{ 0.f, 1.f, 0.f };	// normalized
		Math::VecF3 m_CameraRight{ 0.f, 0.f, 1.f }; // normalized


		float m_MoveSpeed{ 1.f };		// movement speed in units/second
		float m_RotateSpeed{ 0.005f };	// mouse sensitivity in radians/pixel


	public:
		void KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods) { this->Get_Derived()->Imp_KeyboardUpdate(key, scancode, action, mods); }
		void MousePosUpdate(double xpos, double ypos) { this->Get_Derived()->Imp_MousePosUpdate(xpos, ypos); }
		void MouseButtonUpdate(Int32 button, Int32 action, Int32 mods) { this->Get_Derived()->Imp_MouseButtonUpdate(button, action, mods); }
		void MouseScrollUpdate(double xoffset, double yoffset) { this->Get_Derived()->Imp_MouseScrollUpdate(xoffset, yoffset); }
		void JoystickButtonUpdate(Int32 button, bool pressed) { this->Get_Derived()->Imp_JoystickButtonUpdate(button, pressed); }
		void JoystickUpdate(Int32 axis, float value) { this->Get_Derived()->Imp_JoystickUpdate(axis, value); }
		void Animate(float deltaT) { this->Get_Derived()->Imp_Animate(deltaT); }



	private:
		STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
		STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
	private:
		void Imp_KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods) { LOG_ERROR("Empty Imp"); }
		void Imp_MousePosUpdate(double xpos, double ypos) { LOG_ERROR("Empty Imp"); }
		void Imp_MouseButtonUpdate(Int32 button, Int32 action, Int32 mods) { LOG_ERROR("Empty Imp"); }
		void Imp_MouseScrollUpdate(double xoffset, double yoffset) { LOG_ERROR("Empty Imp"); }
		void Imp_JoystickButtonUpdate(Int32 button, bool pressed) { LOG_ERROR("Empty Imp"); }
		void Imp_JoystickUpdate(Int32 axis, float value) { LOG_ERROR("Empty Imp"); }
		void Imp_Animate(float deltaT) { LOG_ERROR("Empty Imp"); }

	};









	template<typename Derived>
	inline void BaseCamera<Derived>::BaseLookAt(Math::VecF3 cameraPos, Math::VecF3 cameraTarget, Math::VecF3 cameraUp) {
		this->m_CameraPos = cameraPos;
		this->m_CameraDir = Math::Normalize(cameraTarget - cameraPos);
		this->m_CameraUp = Math::Normalize(cameraUp);
		this->m_CameraRight = Math::Normalize(Math::Cross(this->m_CameraDir, this->m_CameraUp));
		this->m_CameraUp = Math::Normalize(Math::Cross(this->m_CameraRight, this->m_CameraDir));

		this->UpdateWorldToView();
	}

	template<typename Derived>
	inline void BaseCamera<Derived>::UpdateWorldToView(void) {
		this->m_MatTranslatedWorldToView = Math::AffineF3::FromCols(
			this->m_CameraRight,
			this->m_CameraUp,
			this->m_CameraDir,
			0.f
		);
		this->m_MatWorldToView = Math::Translation(-this->m_CameraPos) * this->m_MatTranslatedWorldToView;
	}

}