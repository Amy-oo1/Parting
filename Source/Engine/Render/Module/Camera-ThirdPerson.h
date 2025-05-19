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

	class ThirdPersonCamera final :public BaseCamera<ThirdPersonCamera> {
		friend class BaseCamera<ThirdPersonCamera>;

	public:
		enum class KeyboardControls :Uint8 {
			HorizontalPan,

			COUNT,
		};

		enum class MouseButtons :Uint8 {
			Left,
			Middle,
			Right,

			COUNT
		};

	public:
		ThirdPersonCamera(void) = default;
		~ThirdPersonCamera(void) = default;

	public:





		void Set_Rotation(float yaw, float pitch);

		void Set_TargetPosition(Math::VecF3 pos) { this->m_TargetPos = pos; }

		void Set_Distance(float distance) { this->m_Distance = distance; }


		void Animate(float deltaT);

		void AnimateOrbit(float deltaT);

		void AnimateTranslation(const Math::MatF33& viewMatrix);

	private:


	private:
		// View parameters to derive translation amounts
		Math::MatF44 m_ProjectionMatrix{ Math::MatF44::Identity() };
		Math::MatF44 m_InverseProjectionMatrix{ Math::MatF44::Identity() };
		Math::VecF2 m_ViewportSize{ Math::VecF2::Zero() };

		Math::VecF2 m_MousePos{ Math::VecF2::Zero() };
		Math::VecF2 m_MousePosPrev{ Math::VecF2::Zero() };

		Math::VecF3 m_TargetPos{ Math::VecF3::Zero() };
		float m_Distance{ 30.f };

		float m_MinDistance{ 0.f };
		float m_MaxDistance{ Max_Float };

		float m_Yaw{ 0.f };
		float m_Pitch{ 0.f };

		float m_DeltaYaw{ 0.f };
		float m_DeltaPitch{ 0.f };
		float m_DeltaDistance{ 0.f };

		const UnorderedMap<Uint32, KeyboardControls> m_KeyboardMap{
			{ 342u /*GLFW_KEY_LEFT_ALT*/, KeyboardControls::HorizontalPan }
		};

		Array<bool, Tounderlying(KeyboardControls::COUNT)> m_KeyboardState{ false };
		Array<bool, Tounderlying(MouseButtons::COUNT)> m_MouseButtonState{ false,false,false };//NOTE : array is euqal to cstyle array,so can not init with count


	private:


	};





	void ThirdPersonCamera::Set_Rotation(float yaw, float pitch) {
		this->m_Yaw = yaw;
		this->m_Pitch = pitch;
	}

	inline void ThirdPersonCamera::Animate(float deltaT) {
		AnimateOrbit(deltaT);

		Math::QuatF orbit{ Math::RotationQuat(Math::VecF3{ this->m_Pitch, this->m_Yaw, 0 }) };

		const auto targetRotation{ orbit.ToMat() };
		AnimateTranslation(targetRotation);

		const Math::VecF3 vectorToCamera{ -this->m_Distance * targetRotation.Row2 };

		const Math::VecF3 camPos{ this->m_TargetPos + vectorToCamera };

		this->m_CameraPos = camPos;
		this->m_CameraRight = -targetRotation.Row0;
		this->m_CameraUp = targetRotation.Row1;
		this->m_CameraDir = targetRotation.Row2;
		this->UpdateWorldToView();

		this->m_MousePosPrev = this->m_MousePos;
	}

	inline void ThirdPersonCamera::AnimateOrbit(float deltaT) {
		if (m_MouseButtonState[Tounderlying(MouseButtons::Left)]) {
			auto mouseMove{ this->m_MousePos - this->m_MousePosPrev };
			float rotateSpeed = this->m_RotateSpeed;

			this->m_Yaw -= rotateSpeed * mouseMove.X;
			this->m_Pitch += rotateSpeed * mouseMove.Y;
		}

		constexpr float ORBIT_SENSITIVITY{ 1.5f };
		constexpr float ZOOM_SENSITIVITY{ 40.f };

		this->m_Distance += ZOOM_SENSITIVITY * deltaT * this->m_DeltaDistance;
		this->m_Yaw += ORBIT_SENSITIVITY * deltaT * this->m_DeltaYaw;
		this->m_Pitch += ORBIT_SENSITIVITY * deltaT * this->m_DeltaPitch;

		this->m_Distance = Math::Clamp(this->m_Distance, this->m_MinDistance, this->m_MaxDistance);

		this->m_Pitch = Math::Clamp(this->m_Pitch, Math::PI_F * -0.5f, Math::PI_F * 0.5f);

		this->m_DeltaDistance = 0;
		this->m_DeltaYaw = 0;
		this->m_DeltaPitch = 0;

	}

	inline void ThirdPersonCamera::AnimateTranslation(const Math::MatF33& viewMatrix) {
		// If the view parameters have never been set, we can't translate
		if (this->m_ViewportSize.X <= 0.f || this->m_ViewportSize.Y <= 0.f)
			return;

		if (Math::All(this->m_MousePos == this->m_MousePosPrev))
			return;

		if (this->m_MouseButtonState[Tounderlying(MouseButtons::Middle)]) {
			Math::VecF4 oldClipPos{ Math::VecF4(0.f, 0.f, this->m_Distance, 1.f) * this->m_ProjectionMatrix };
			oldClipPos /= oldClipPos.W;
			oldClipPos.X = 2.f * (this->m_MousePosPrev.X) / this->m_ViewportSize.X - 1.f;
			oldClipPos.Y = 1.f - 2.f * (this->m_MousePosPrev.Y) / this->m_ViewportSize.Y;

			Math::VecF4 newClipPos{ oldClipPos };
			newClipPos.X = 2.f * (this->m_MousePos.X) / this->m_ViewportSize.X - 1.f;
			newClipPos.Y = 1.f - 2.f * (this->m_MousePos.Y) / this->m_ViewportSize.Y;

			Math::VecF4 oldViewPos{ oldClipPos * this->m_InverseProjectionMatrix };
			oldViewPos /= oldViewPos.W;
			Math::VecF4 newViewPos{ newClipPos * this->m_InverseProjectionMatrix };
			newViewPos /= newViewPos.W;

			Math::VecF2 viewMotion{ oldViewPos.XY() - newViewPos.XY() };

			this->m_TargetPos -= viewMotion.X * viewMatrix.Row0;

			if (this->m_KeyboardState[Tounderlying(KeyboardControls::HorizontalPan)]) {
				Math::VecF3 horizontalForward{ viewMatrix.Row2.X, 0.f, viewMatrix.Row2.Z };
				float horizontalLength{ Math::Length(horizontalForward) };
				if (horizontalLength == 0.f)
					horizontalForward = Math::VecF3{ viewMatrix.Row1.X, 0.f, viewMatrix.Row1.Z };
				horizontalForward = Math::Normalize(horizontalForward);

				this->m_TargetPos += viewMotion.Y * horizontalForward * 1.5f;
			}
			else
				this->m_TargetPos += viewMotion.Y * viewMatrix.Row1;
		}
	}

}