#ifndef FORWARD_CB_H
#define FORWARD_CB_H

#include "light_cb.h"
#include "view_cb.h"

//TODO to constexpr

#define FORWARD_MAX_LIGHTS 16
#define FORWARD_MAX_SHADOWS 16
#define FORWARD_MAX_LIGHT_PROBES 16

#define FORWARD_SPACE_MATERIAL 0
#define FORWARD_BINDING_MATERIAL_CONSTANTS 0
#define FORWARD_BINDING_MATERIAL_DIFFUSE_TEXTURE 0
#define FORWARD_BINDING_MATERIAL_SPECULAR_TEXTURE 1
#define FORWARD_BINDING_MATERIAL_NORMAL_TEXTURE 2
#define FORWARD_BINDING_MATERIAL_EMISSIVE_TEXTURE 3
#define FORWARD_BINDING_MATERIAL_OCCLUSION_TEXTURE 4
#define FORWARD_BINDING_MATERIAL_TRANSMISSION_TEXTURE 5
#define FORWARD_BINDING_MATERIAL_OPACITY_TEXTURE 6

#define FORWARD_SPACE_INPUT 1
#define FORWARD_BINDING_PUSH_CONSTANTS 1
#define FORWARD_BINDING_INSTANCE_BUFFER 10
#define FORWARD_BINDING_VERTEX_BUFFER 11

#define FORWARD_SPACE_VIEW 2
#define FORWARD_BINDING_VIEW_CONSTANTS 2

#define FORWARD_SPACE_SHADING 3
#define FORWARD_BINDING_LIGHT_CONSTANTS 3
#define FORWARD_BINDING_SHADOW_MAP_TEXTURE 20
#define FORWARD_BINDING_DIFFUSE_LIGHT_PROBE_TEXTURE 21
#define FORWARD_BINDING_SPECULAR_LIGHT_PROBE_TEXTURE 22
#define FORWARD_BINDING_ENVIRONMENT_BRDF_TEXTURE 23
#define FORWARD_BINDING_MATERIAL_SAMPLER 0
#define FORWARD_BINDING_SHADOW_MAP_SAMPLER 1
#define FORWARD_BINDING_LIGHT_PROBE_SAMPLER 2
#define FORWARD_BINDING_ENVIRONMENT_BRDF_SAMPLER 3

namespace Shader {
	constexpr Uint32 ForwardMaxLights{ FORWARD_MAX_LIGHTS };
	constexpr Uint32 ForwardMaxShadows{ FORWARD_MAX_SHADOWS };
	constexpr Uint32 ForwardMaxLightProbes{ FORWARD_MAX_LIGHT_PROBES };

	constexpr Uint32 ForwardSpaceMaterial{ FORWARD_SPACE_MATERIAL };
	constexpr Uint32 ForwardBindingMaterialConstants{ FORWARD_BINDING_MATERIAL_CONSTANTS };
	constexpr Uint32 ForwardBindingMaterialDiffuseTexture{ FORWARD_BINDING_MATERIAL_DIFFUSE_TEXTURE };
	constexpr Uint32 ForwardBindingMaterialSpecularTexture{ FORWARD_BINDING_MATERIAL_SPECULAR_TEXTURE };
	constexpr Uint32 ForwardBindingMaterialNormalTexture{ FORWARD_BINDING_MATERIAL_NORMAL_TEXTURE };
	constexpr Uint32 ForwardBindingMaterialEmissiveTexture{ FORWARD_BINDING_MATERIAL_EMISSIVE_TEXTURE };
	constexpr Uint32 ForwardBindingMaterialOcclusionTexture{ FORWARD_BINDING_MATERIAL_OCCLUSION_TEXTURE };
	constexpr Uint32 ForwardBindingMaterialTransmissionTexture{ FORWARD_BINDING_MATERIAL_TRANSMISSION_TEXTURE };
	constexpr Uint32 ForwardBindingMaterialOpacityTexture{ FORWARD_BINDING_MATERIAL_OPACITY_TEXTURE };
	
	constexpr Uint32 ForwardSpaceInput{ FORWARD_SPACE_INPUT };
	constexpr Uint32 ForwardBindingPushConstants{ FORWARD_BINDING_PUSH_CONSTANTS };
	constexpr Uint32 ForwardBindingInstanceBuffer{ FORWARD_BINDING_INSTANCE_BUFFER };
	constexpr Uint32 ForwardBindingVertexBuffer{ FORWARD_BINDING_VERTEX_BUFFER };

	constexpr Uint32 ForwardSpaceView{ FORWARD_SPACE_VIEW };
	constexpr Uint32 ForwardBindingViewConstants{ FORWARD_BINDING_VIEW_CONSTANTS };

	constexpr Uint32 ForwardSpaceShading{ FORWARD_SPACE_SHADING };
	constexpr Uint32 ForwardBindingLightConstants{ FORWARD_BINDING_LIGHT_CONSTANTS };
	constexpr Uint32 ForwardBindingShadowMapTexture{ FORWARD_BINDING_SHADOW_MAP_TEXTURE };
	constexpr Uint32 ForwardBindingDiffuseLightProbeTexture{ FORWARD_BINDING_DIFFUSE_LIGHT_PROBE_TEXTURE };
	constexpr Uint32 ForwardBindingSpecularLightProbeTexture{ FORWARD_BINDING_SPECULAR_LIGHT_PROBE_TEXTURE };
	constexpr Uint32 ForwardBindingEnvironmentBRDFTexture{ FORWARD_BINDING_ENVIRONMENT_BRDF_TEXTURE };
	constexpr Uint32 ForwardBindingMaterialSampler{ FORWARD_BINDING_MATERIAL_SAMPLER };
	constexpr Uint32 ForwardBindingShadowMapSampler{ FORWARD_BINDING_SHADOW_MAP_SAMPLER };
	constexpr Uint32 ForwardBindingLightProbeSampler{ FORWARD_BINDING_LIGHT_PROBE_SAMPLER };
	constexpr Uint32 ForwardBindingEnvironmentBRDFSampler{ FORWARD_BINDING_ENVIRONMENT_BRDF_SAMPLER };

	struct ForwardShadingViewConstants {
		PlanarViewConstants		View;
	};

	struct ForwardShadingLightConstants {
		Math::VecF2				ShadowMapTextureSize;
		Math::VecF2				ShadowMapTextureSizeInv;
		Math::VecF4				AmbientColorTop;
		Math::VecF4				AmbientColorBottom;

		Math::VecU2				Padding;
		Uint32					NumLights;
		Uint32					NumLightProbes;

		LightConstants			Lights[ForwardMaxLights];
		ShadowConstants			Shadows[ForwardMaxShadows];
		LightProbeConstants		LightProbes[ForwardMaxLightProbes];
	};

	struct ForwardPushConstants {
		Uint32					StartInstanceLocation;
		Uint32					StartVertexLocation;
		Uint32					PositionOffset;
		Uint32					TexCoordOffset;
		Uint32					NormalOffset;
		Uint32					TangentOffset;
	};
}


#endif // FORWARD_CB_H