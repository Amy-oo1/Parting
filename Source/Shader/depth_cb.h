/*
* Copyright (c) 2014-2024, NVIDIA CORPORATION. All rights reserved.
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

#ifndef DEPTH_CB_H
#define DEPTH_CB_H

constexpr Uint32 DepthSpaceMaterial{ 0 };
constexpr Uint32 DepthBindingMaterialDiffuseTexture{ 0 };
constexpr Uint32 DepthBindingMaterialOpacityTexture{ 1 };
constexpr Uint32 DepthBindingMaterialConstants{ 0 };

constexpr Uint32 DepthSapceInput{ 1 };
constexpr Uint32 DepthBindingPushConstants{ 1 };
constexpr Uint32 DepthBindingInstanceBuffer{ 10 };
constexpr Uint32 DepthBindingVertexBuffer{ 11 };

constexpr Uint32 DepthSpaceView{ 2 };
constexpr Uint32 DepthBindingViewConstants{ 2 };
constexpr Uint32 DepthBindingMaterialSampler{ 0 };

struct DepthPassConstants {
	Math::MatF44 MatWorldToClip;
};

// 

struct DepthPushConstants {
	Uint32			StartInstanceLocation;
	Uint32			StartVertexLocation;
	Uint32			PositionOffset;
	Uint32			TexCoordOffset;
};

namespace Shader {
	constexpr Uint32 DepthSpaceMaterial{ 0 };
	constexpr Uint32 DepthBindingMaterialDiffuseTexture{ 0 };
	constexpr Uint32 DepthBindingMaterialOpacityTexture{ 1 };
	constexpr Uint32 DepthBindingMaterialConstants{ 0 };

	constexpr Uint32 DepthSapceInput{ 1 };
	constexpr Uint32 DepthBindingPushConstants{ 1 };
	constexpr Uint32 DepthBindingInstanceBuffer{ 10 };
	constexpr Uint32 DepthBindingVertexBuffer{ 11 };

	constexpr Uint32 DepthSpaceView{ 2 };
	constexpr Uint32 DepthBindingViewConstants{ 2 };
	constexpr Uint32 DepthBindingMaterialSampler{ 0 };


	using DepthPassConstants = DepthPassConstants;
	using DepthPushConstants = DepthPushConstants;
}


#endif // DEPTH_CB_H