
#ifndef LIGHT_CB_H
#define LIGHT_CB_H

#include "light_types.h"

struct ShadowConstants final {
	Math::MatF44 MatWorldToUvzwShadow;

	Math::VecF2 ShadowFadeScale;
	Math::VecF2 ShadowFadeBias;

	Math::VecF2 ShadowMapCenterUV;
	float ShadowFalloffDistance;
	Uint32 ShadowMapArrayIndex;

	Math::VecF2 ShadowMapSizeTexels;
	Math::VecF2 ShadowMapSizeTexelsInv;
};

struct LightConstants final {
	Math::VecF3 Direction;
	Uint32 LightType;

	Math::VecF3 Position;
	float Radius;

	Math::VecF3 Color;
	float Intensity; // illuminance (lm/m2) for directional lights, luminous intensity (lm/sr) for positional lights

	float AngularSizeOrInvRange;   // angular size for directional lights, 1/range for spot and point lights
	float InnerAngle;
	float OuterAngle;
	float OutOfBoundsShadow;

	Math::VecI4 ShadowCascades;
	Math::VecI4 PerObjectShadows;

	Math::VecI4 ShadowChannel;
};

struct LightProbeConstants final {
	float DiffuseScale;
	float SpecularScale;
	float MipLevels;
	float _adding1;

	Uint32 DiffuseArrayIndex;
	Uint32 SpecularArrayIndex;
	Math::VecU2 _Padding2;

	Math::MatF44 FrustumPlanes[6];
};

#endif // LIGHT_CB_H