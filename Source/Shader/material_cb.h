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

#ifndef MATERIAL_CB_H
#define MATERIAL_CB_H
// NOTE: adjust LoadMaterialConstants(...) in bindless.h when changing this structure

struct MaterialConstants
{
	Math::VecF3		BaseOrDiffuseColor;
	Uint32			Flags;

	Math::VecF3		SpecularColor;
	Uint32			MaterialID;

	Math::VecF3		EmissiveColor;
	Uint32			Domain;

	float			Opacity;
	float			Roughness;
	float			Metalness;
	float			NormalTextureScale;

	float			OcclusionStrength;
	float			AlphaCutoff;
	float			TransmissionFactor;


	Int32			BaseOrDiffuseTextureIndex{ -1 };//TODO :Remove
	Int32			MetalRoughOrSpecularTextureIndex{ -1 };
	Int32			EmissiveTextureIndex{ -1 };
	Int32			NormalTextureIndex{ -1 };
	Int32			OcclusionTextureIndex{ -1 };
	Int32			TransmissionTextureIndex{ -1 };
	Int32			OpacityTextureIndex{ -1 };


	Math::VecF2		NormalTextureTransformScale;

	Math::VecU3		Padding1;
	float			SSSScale;

	Math::VecF3		SSSTransmissionColor;
	float			SSSAnisotropy;

	Math::VecF3		SSSScatteringColor;
	float			HairMelanin;

	Math::VecF3		HairBaseColor;
	float			HairMelaninRedness;

	float			HairLongitudinalRoughness;
	float			HairAzimuthalRoughness;
	float			HairIOR;
	float			HairCuticleAngle;

	Math::VecF3		HairDiffuseReflectionTint;
	float			HairDiffuseReflectionWeight;
};

namespace Shader {

	constexpr Int32 MaterialDomain_Opaque = 0;
	constexpr Int32 MaterialDomain_AlphaTested = 1;
	constexpr Int32 MaterialDomain_AlphaBlended = 2;
	constexpr Int32 MaterialDomain_Transmissive = 3;
	constexpr Int32 MaterialDomain_TransmissiveAlphaTested = 4;
	constexpr Int32 MaterialDomain_TransmissiveAlphaBlended = 5;

	constexpr Int32 MaterialFlags_UseSpecularGlossModel = 0x00000001;
	constexpr Int32 MaterialFlags_DoubleSided = 0x00000002;
	constexpr Int32 MaterialFlags_UseMetalRoughOrSpecularTexture = 0x00000004;
	constexpr Int32 MaterialFlags_UseBaseOrDiffuseTexture = 0x00000008;
	constexpr Int32 MaterialFlags_UseEmissiveTexture = 0x00000010;
	constexpr Int32 MaterialFlags_UseNormalTexture = 0x00000020;
	constexpr Int32 MaterialFlags_UseOcclusionTexture = 0x00000040;
	constexpr Int32 MaterialFlags_UseTransmissionTexture = 0x00000080;
	constexpr Int32 MaterialFlags_MetalnessInRedChannel = 0x00000100;
	constexpr Int32 MaterialFlags_UseOpacityTexture = 0x00000200;
	constexpr Int32 MaterialFlags_SubsurfaceScattering = 0x00000400;
	constexpr Int32 MaterialFlags_Hair = 0x00000800;

	using MaterialConstants = ::MaterialConstants;
}


#endif // MATERIAL_CB_H
