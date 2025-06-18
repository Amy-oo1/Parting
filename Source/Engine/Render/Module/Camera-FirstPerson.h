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

#include "Engine/Application/Module/GLFWWrapper.h"
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
		enum class KeyboardControls :Uint8 {
			MoveUp,
			MoveDown,
			MoveLeft,
			MoveRight,
			MoveForward,
			MoveBackward,

			YawRight,
			YawLeft,
			PitchUp,
			PitchDown,
			RollLeft,
			RollRight,

			SpeedUp,
			SlowDown,

			COUNT
		};

		enum class MouseButtons :Uint8 {
			Left,
			Middle,
			Right,

			COUNT,

			MouseButtonFirst = Left
		};

	public:
		FirstPersonCamera(void) = default;
		~FirstPersonCamera(void) = default;

	public:
		void LookAt(Math::VecF3 cameraPos, Math::VecF3 cameraTarget, Math::VecF3 cameraUp = Math::VecF3{ 0.f, 1.f, 0.f });
		void LookTo(Math::VecF3 cameraPos, Math::VecF3 cameraDir, Math::VecF3 cameraUp = Math::VecF3{ 0.f, 1.f, 0.f });

		Pair<bool, Math::AffineF3> AnimateRoll(Math::AffineF3 initialRotation);
		Pair<bool, Math::VecF3> AnimateTranslation(float deltaT);
		void UpdateCamera(Math::VecF3 cameraMoveVec, Math::AffineF3 cameraRotation);

	private:


	private:
		const UnorderedMap<Int32, KeyboardControls> m_KeyboardMap{
			{ GLFW_KEY_Q, KeyboardControls::MoveDown },
			{ GLFW_KEY_E, KeyboardControls::MoveUp },
			{ GLFW_KEY_A, KeyboardControls::MoveLeft },
			{ GLFW_KEY_D, KeyboardControls::MoveRight },
			{ GLFW_KEY_W, KeyboardControls::MoveForward },
			{ GLFW_KEY_S, KeyboardControls::MoveBackward },
			{ GLFW_KEY_LEFT, KeyboardControls::YawLeft },
			{ GLFW_KEY_RIGHT, KeyboardControls::YawRight },
			{ GLFW_KEY_UP, KeyboardControls::PitchUp },
			{ GLFW_KEY_DOWN, KeyboardControls::PitchDown },
			{ GLFW_KEY_Z, KeyboardControls::RollLeft },
			{ GLFW_KEY_C, KeyboardControls::RollRight },
			{ GLFW_KEY_LEFT_SHIFT, KeyboardControls::SpeedUp },
			{ GLFW_KEY_RIGHT_SHIFT, KeyboardControls::SpeedUp },
			{ GLFW_KEY_LEFT_CONTROL, KeyboardControls::SlowDown },
			{ GLFW_KEY_RIGHT_CONTROL, KeyboardControls::SlowDown }
		};

		const UnorderedMap<Int32, MouseButtons> m_MouseButtonMap{
			{ GLFW_MOUSE_BUTTON_LEFT, MouseButtons::Left },
			{ GLFW_MOUSE_BUTTON_MIDDLE, MouseButtons::Middle },
			{ GLFW_MOUSE_BUTTON_RIGHT, MouseButtons::Right }
		};

		Array<bool, Tounderlying(KeyboardControls::COUNT)> m_KeyboardState{};
		Array<bool, Tounderlying(MouseButtons::COUNT)> m_MouseButtonState{};


		Math::VecF2 m_MousePos{ Math::VecF2::Zero() };
		Math::VecF2 m_MousePosPrev{ Math::VecF2::Zero() };
		Math::VecF2 m_MouseMotionAccumulator{ Math::VecF2::Zero() };
		Math::VecF3 m_CameraMovePrev{ Math::VecF3::Zero() };
		Math::VecF3 m_CameraMoveDamp{ Math::VecF3::Zero() };
		bool m_IsDragging{ false };


	private:
		void Imp_KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods);
		void Imp_MousePosUpdate(double xpos, double ypos);
		void Imp_MouseButtonUpdate(Int32 button, Int32 action, Int32 mods);
		void Imp_Animate(float deltaT);

	};



	void FirstPersonCamera::LookAt(Math::VecF3 cameraPos, Math::VecF3 cameraTarget, Math::VecF3 cameraUp) {
		this->BaseLookAt(cameraPos, cameraTarget, cameraUp);

		this->m_MouseMotionAccumulator = 0.f;
		this->m_CameraMoveDamp = 0.f;
		this->m_CameraMovePrev = 0.f;

	}

	inline void FirstPersonCamera::LookTo(Math::VecF3 cameraPos, Math::VecF3 cameraDir, Math::VecF3 cameraUp) {
		this->BaseLookAt(cameraPos, cameraPos + cameraDir, cameraUp);

		this->m_MouseMotionAccumulator = 0.f;
		this->m_CameraMoveDamp = 0.f;
		this->m_CameraMovePrev = 0.f;
	}

	inline Pair<bool, Math::AffineF3> FirstPersonCamera::AnimateRoll(Math::AffineF3 cameraRotation) {
		bool cameraDirty{ false };
		using enum KeyboardControls;
		if (this->m_KeyboardState[Tounderlying(RollLeft)] ||
			this->m_KeyboardState[Tounderlying(RollRight)]) {
			float roll{
				static_cast<float>(this->m_KeyboardState[Tounderlying(RollLeft)]) * -this->m_RotateSpeed * 2.0f +
				static_cast<float>(this->m_KeyboardState[Tounderlying(RollRight)]) * this->m_RotateSpeed * 2.0f
			};

			cameraRotation = Math::Rotation(this->m_CameraDir, roll) * cameraRotation;
			cameraDirty = true;
		}

		return MakePair(cameraDirty, cameraRotation);
	}

	inline Pair<bool, Math::VecF3> FirstPersonCamera::AnimateTranslation(float deltaT) {
		bool cameraDirty{ false };
		float moveStep{ deltaT * this->m_MoveSpeed };
		Math::VecF3 cameraMoveVec{ 0.f };

		using enum KeyboardControls;

		if (this->m_KeyboardState[Tounderlying(SpeedUp)])
			moveStep *= 3.f;

		if (this->m_KeyboardState[Tounderlying(SlowDown)])
			moveStep *= 0.1f;

		if (this->m_KeyboardState[Tounderlying(MoveForward)]) {
			cameraDirty = true;
			cameraMoveVec += this->m_CameraDir * moveStep;
		}

		if (this->m_KeyboardState[Tounderlying(MoveBackward)]) {
			cameraDirty = true;
			cameraMoveVec += -this->m_CameraDir * moveStep;
		}

		if (this->m_KeyboardState[Tounderlying(MoveLeft)]) {
			cameraDirty = true;
			cameraMoveVec += -this->m_CameraRight * moveStep;
		}

		if (this->m_KeyboardState[Tounderlying(MoveRight)]) {
			cameraDirty = true;
			cameraMoveVec += this->m_CameraRight * moveStep;
		}

		if (this->m_KeyboardState[Tounderlying(MoveUp)]) {
			cameraDirty = true;
			cameraMoveVec += this->m_CameraUp * moveStep;
		}

		if (this->m_KeyboardState[Tounderlying(MoveDown)]) {
			cameraDirty = true;
			cameraMoveVec += -this->m_CameraUp * moveStep;
		}
		return MakePair(cameraDirty, cameraMoveVec);
	}

	inline void FirstPersonCamera::UpdateCamera(Math::VecF3 cameraMoveVec, Math::AffineF3 cameraRotation) {
		this->m_CameraPos += cameraMoveVec;
		this->m_CameraDir = Math::Normalize(cameraRotation.TransformVector(this->m_CameraDir));
		this->m_CameraUp = Math::Normalize(cameraRotation.TransformVector(this->m_CameraUp));
		this->m_CameraRight = Math::Normalize(Math::Cross(this->m_CameraDir, this->m_CameraUp));

		this->UpdateWorldToView();
	}

	inline void FirstPersonCamera::Imp_KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods) {
		if (this->m_KeyboardMap.find(key) == this->m_KeyboardMap.end()) {
			LOG_INFO("Key not found in map");
			return;
		}

		this->m_KeyboardState[Tounderlying(this->m_KeyboardMap.at(key))] = (action == GLFW_PRESS || action == GLFW_REPEAT);
	}

	inline void FirstPersonCamera::Imp_MousePosUpdate(double xpos, double ypos) {
		this->m_MousePos = Math::VecF2{ static_cast<float>(xpos), static_cast<float>(ypos) };
	}

	inline void FirstPersonCamera::Imp_MouseButtonUpdate(Int32 button, Int32 action, Int32 mods) {
		if (this->m_MouseButtonMap.find(button) == this->m_MouseButtonMap.end()) {
			LOG_INFO("Mouse button not found in map");

			return;
		}

		this->m_MouseButtonState[Tounderlying(this->m_MouseButtonMap.at(button))] = (GLFW_PRESS == action);
	}

	inline void FirstPersonCamera::Imp_Animate(float deltaT) {
		// Track mouse delta.
		// Use m_IsDragging to avoid random camera rotations when clicking inside an inactive window.
		Math::VecF2 mouseMove{ 0.f };
		if (this->m_MouseButtonState[Tounderlying(MouseButtons::Left)]) {
			if (this->m_IsDragging)
				mouseMove = this->m_MousePos - this->m_MousePosPrev;

			this->m_IsDragging = true;
		}
		else
			this->m_IsDragging = false;
		this->m_MousePosPrev = this->m_MousePos;

		bool cameraDirty{ false };
		Math::AffineF3 cameraRotation{ Math::AffineF3::Identity() };

		// handle mouse rotation first
		// this will affect the movement vectors in the world matrix, which we use below
		if (this->m_MouseButtonState[Tounderlying(MouseButtons::Left)] && (mouseMove.X != 0.f || mouseMove.Y != 0.f)) {
			float yaw{ this->m_RotateSpeed * mouseMove.X };
			float pitch{ this->m_RotateSpeed * mouseMove.Y };

			cameraRotation = Math::Rotation(Math::VecF3{ 0.f, 1.f, 0.f }, -yaw);
			cameraRotation = Math::Rotation(this->m_CameraRight, -pitch) * cameraRotation;

			cameraDirty = true;
		}

		// handle keyboard roll next
		{
			bool cameraRotationresult{ false };
			Tie(cameraRotationresult, cameraRotation) = this->AnimateRoll(cameraRotation);
			cameraDirty |= cameraRotationresult;
		}

		// handle translation
		const auto [translateResult, cameraMoveVec] = this->AnimateTranslation(deltaT);
		cameraDirty |= translateResult;

		if (cameraDirty)
			this->UpdateCamera(cameraMoveVec, cameraRotation);
	}




}