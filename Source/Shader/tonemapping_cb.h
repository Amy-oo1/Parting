#ifndef TONE_MAPPING_CB_H
#define TONE_MAPPING_CB_H

struct ToneMappingConstants {
	Math::VecU2			ViewOrigin;
	Math::VecU2			ViewSize;

	float				LogLuminanceScale;
	float				LogLuminanceBias;
	float				HistogramLowPercentile;
	float				HistogramHighPercentile;

	float				EyeAdaptationSpeedUp;
	float				EyeAdaptationSpeedDown;
	float				MinAdaptedLuminance;
	float				MaxAdaptedLuminance;

	float				FrameTime;
	float				ExposureScale;
	float				WhitePointInvSquared;
	Uint32				SourceSlice;

	Math::VecF2			ColorLUTTextureSize;
	Math::VecF2			ColorLUTTextureSizeInv;
};

#endif // TONE_MAPPING_CB_H