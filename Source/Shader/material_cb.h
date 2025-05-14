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

static const int MaterialDomain_Opaque = 0;
static const int MaterialDomain_AlphaTested = 1;
static const int MaterialDomain_AlphaBlended = 2;
static const int MaterialDomain_Transmissive = 3;
static const int MaterialDomain_TransmissiveAlphaTested = 4;
static const int MaterialDomain_TransmissiveAlphaBlended = 5;

static const int MaterialFlags_UseSpecularGlossModel = 0x00000001;
static const int MaterialFlags_DoubleSided = 0x00000002;
static const int MaterialFlags_UseMetalRoughOrSpecularTexture = 0x00000004;
static const int MaterialFlags_UseBaseOrDiffuseTexture = 0x00000008;
static const int MaterialFlags_UseEmissiveTexture = 0x00000010;
static const int MaterialFlags_UseNormalTexture = 0x00000020;
static const int MaterialFlags_UseOcclusionTexture = 0x00000040;
static const int MaterialFlags_UseTransmissionTexture = 0x00000080;
static const int MaterialFlags_MetalnessInRedChannel = 0x00000100;
static const int MaterialFlags_UseOpacityTexture = 0x00000200;
static const int MaterialFlags_SubsurfaceScattering = 0x00000400;
static const int MaterialFlags_Hair = 0x00000800;

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
	Uint32			BaseOrDiffuseTextureIndex;

	Uint32			MetalRoughOrSpecularTextureIndex;
	Uint32			EmissiveTextureIndex;
	Uint32			NormalTextureIndex;
	Uint32			OcclusionTextureIndex;

	Uint32			TransmissionTextureIndex;
	Uint32			OpacityTextureIndex;
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
	float			HairIor;
	float			HairCuticleAngle;

	Math::VecF3		HairDiffuseReflectionTint;
	float			HairDiffuseReflectionWeight;
};

#endif // MATERIAL_CB_H
