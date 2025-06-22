#ifndef DEFERRED_LIGHTING_CB_H
#define DEFERRED_LIGHTING_CB_H

#include "light_cb.h"
#include "view_cb.h"

#define DEFERRED_MAX_LIGHTS 16
#define DEFERRED_MAX_SHADOWS 16
#define DEFERRED_MAX_LIGHT_PROBES 16

namespace Shader {
	constexpr Uint32 DeferredMaxLights{ DEFERRED_MAX_LIGHTS };
	constexpr Uint32 DeferredMaxShadows{ DEFERRED_MAX_SHADOWS };
	constexpr Uint32 DeferredMaxLightProbes{ DEFERRED_MAX_LIGHT_PROBES };
	
	struct DeferredLightingConstants {
		PlanarViewConstants		View;

		Math::VecF2				ShadowMapTextureSize;
		Int32					EnableAmbientOcclusion;
		Int32					Padding;

		Math::VecF4				AmbientColorTop;
		Math::VecF4				AmbientColorBottom;

		Uint32					NumLights;
		Uint32					NumLightProbes;
		float					IndirectDiffuseScale;
		float					IndirectSpecularScale;

		Math::VecF2				RandomOffset;
		Math::VecF2				Padding2;

		Math::VecF4				NoisePattern[4];

		LightConstants			Lights[DeferredMaxLights];
		ShadowConstants			Shadows[DeferredMaxShadows];
		LightProbeConstants		LightProbes[DeferredMaxLightProbes];
	};
}


#endif // DEFERRED_LIGHTING_CB_H