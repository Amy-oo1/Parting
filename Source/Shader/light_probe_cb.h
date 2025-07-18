#ifndef LIGHT_PROBE_CB_H
#define LIGHT_PROBE_CB_H



namespace Shader {
	struct LightProbeProcessingConstants {
		Uint32		SampleCount;
		float		LodBias;
		float		Roughness;
		float		InputCubeSize;
	};
}

#endif // LIGHT_PROBE_CB_H