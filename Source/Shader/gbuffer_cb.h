#ifndef GBUFFER_CB_H
#define GBUFFER_CB_H

#include "view_cb.h"

#define GBUFFER_SPACE_MATERIAL 0
#define GBUFFER_BINDING_MATERIAL_CONSTANTS 0
#define GBUFFER_BINDING_MATERIAL_DIFFUSE_TEXTURE 0
#define GBUFFER_BINDING_MATERIAL_SPECULAR_TEXTURE 1
#define GBUFFER_BINDING_MATERIAL_NORMAL_TEXTURE 2
#define GBUFFER_BINDING_MATERIAL_EMISSIVE_TEXTURE 3
#define GBUFFER_BINDING_MATERIAL_OCCLUSION_TEXTURE 4
#define GBUFFER_BINDING_MATERIAL_TRANSMISSION_TEXTURE 5
#define GBUFFER_BINDING_MATERIAL_OPACITY_TEXTURE 6

#define GBUFFER_SPACE_INPUT 1
#define GBUFFER_BINDING_PUSH_CONSTANTS 1
#define GBUFFER_BINDING_INSTANCE_BUFFER 10
#define GBUFFER_BINDING_VERTEX_BUFFER 11

#define GBUFFER_SPACE_VIEW 2
#define GBUFFER_BINDING_VIEW_CONSTANTS 2
#define GBUFFER_BINDING_MATERIAL_SAMPLER 0

namespace Shader {
	constexpr Uint32 GBufferSpaceMaterial{ GBUFFER_SPACE_MATERIAL };
	constexpr Uint32 GBufferBindingMaterialConstants{ GBUFFER_BINDING_MATERIAL_CONSTANTS };
	constexpr Uint32 GBufferBindingMaterialDiffuseTexture{ GBUFFER_BINDING_MATERIAL_DIFFUSE_TEXTURE };
	constexpr Uint32 GBufferBindingMaterialSpecularTexture{ GBUFFER_BINDING_MATERIAL_SPECULAR_TEXTURE };
	constexpr Uint32 GBufferBindingMaterialNormalTexture{ GBUFFER_BINDING_MATERIAL_NORMAL_TEXTURE };
	constexpr Uint32 GBufferBindingMaterialEmissiveTexture{ GBUFFER_BINDING_MATERIAL_EMISSIVE_TEXTURE };
	constexpr Uint32 GBufferBindingMaterialOcclusionTexture{ GBUFFER_BINDING_MATERIAL_OCCLUSION_TEXTURE };
	constexpr Uint32 GBufferBindingMaterialTransmissionTexture{ GBUFFER_BINDING_MATERIAL_TRANSMISSION_TEXTURE };
	constexpr Uint32 GBufferBindingMaterialOpacityTexture{ GBUFFER_BINDING_MATERIAL_OPACITY_TEXTURE };
	
	constexpr Uint32 GBufferSpaceInput{ GBUFFER_SPACE_INPUT };
	constexpr Uint32 GBufferBindingPushConstants{ GBUFFER_BINDING_PUSH_CONSTANTS };
	constexpr Uint32 GBufferBindingInstanceBuffer{ GBUFFER_BINDING_INSTANCE_BUFFER };
	constexpr Uint32 GBufferBindingVertexBuffer{ GBUFFER_BINDING_VERTEX_BUFFER };
	
	constexpr Uint32 GBufferSpaceView{ GBUFFER_SPACE_VIEW };
	constexpr Uint32 GBufferBindingViewConstants{ GBUFFER_BINDING_VIEW_CONSTANTS };
	constexpr Uint32 GBufferBindingMaterialSampler{ GBUFFER_BINDING_MATERIAL_SAMPLER };

	struct GBufferFillConstants {
		PlanarViewConstants		View;
		PlanarViewConstants		ViewPrev;
	};

	struct GBufferPushConstants {
		Uint32					StartInstanceLocation;
		Uint32					StartVertexLocation;
		Uint32					PositionOffset;
		Uint32					PrevPositionOffset;
		Uint32					TexCoordOffset;
		Uint32					NormalOffset;
		Uint32					TangentOffset;
	};
}


#endif // GBUFFER_CB_H