/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#ifndef SKY_CB_H
#define SKY_CB_H

namespace Shader{
	struct ProceduralSkyShaderParameters {
		Math::VecF3						DirectionToLight;
		float							AngularSizeOfLight;

		Math::VecF3						LightColor;
		float							GlowSize;

		Math::VecF3						SkyColor;
		float							GlowIntensity;

		Math::VecF3						HorizonColor;
		float							HorizonSize;

		Math::VecF3						GroundColor;
		float							GlowSharpness;

		Math::VecF3						DirectionUp;
		float							Padding1;
	};

	struct SkyConstants {
		Math::MatF44					MatClipToTranslatedWorld;

		ProceduralSkyShaderParameters	Params;
	};
}

#endif // SKY_CB_H