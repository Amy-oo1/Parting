#ifndef BLIT_CB_H
#define BLIT_CB_H

struct BLITConstants final{
	Math::VecF2 SourceOrigin;
	Math::VecF2 SourceSize;

	Math::VecF2 TargetOrigin;
	Math::VecF2 TargetSize;
	
	float SharpenFactor;
};

namespace Shader {
	using BLITConstants = BLITConstants;
}



#endif // BLIT_CB_H