#ifndef BINDLESS_H_
#define BINDLESS_H_

#include "material_cb.h"

struct GeometryData {
	Uint32 NumIndices;
	Uint32 NumVertices;
	Int32 IndexBufferIndex{ -1 };//TODO :Removce
	Uint32 IndexOffset;

	Int32 VertexBufferIndex{ -1 };//TODO :Removce
	Uint32 PositionOffset;
	Uint32 PrevPositionOffset;
	Uint32 TexCoord1Offset;

	Uint32 TexCoord2Offset;
	Uint32 NormalOffset;
	Uint32 TangentOffset;
	Uint32 CurveRadiusOffset;

	Uint32 MaterialIndex;
	//Math::VecU3 pad;
	Uint32 pad0;
	Uint32 pad1;
	Uint32 pad2;
};

struct InstanceData {
	Uint32 Flags;
	Uint32 FirstGeometryInstanceIndex; // index into global list of geometry instances. 
	// foreach (Instance)
	//     foreach(Geo) index++
	Uint32 FirstGeometryIndex;         // index into global list of geometries. 
	// foreach(Mesh)
	//     foreach(Geo) index++
	Uint32 NumGeometries;

	Math::MatF34 Transform;
	Math::MatF34 PrevTransform;

	/*bool IsCurveDOTS() { return (flags & InstanceFlags_CurveDisjointOrthogonalTriangleStrips) != 0; }*/
};

namespace Shader {
	constexpr Uint32 InstanceFlags_CurveDisjointOrthogonalTriangleStrips = 0x00000001u;
	
	using GeometryData = ::GeometryData;
	using InstanceData = ::InstanceData;
}

#ifndef __cplusplus

static const uint c_SizeOfTriangleIndices = 12;
static const uint c_SizeOfPosition = 12;
static const uint c_SizeOfTexcoord = 8;
static const uint c_SizeOfNormal = 4;
static const uint c_SizeOfJointIndices = 8;
static const uint c_SizeOfJointWeights = 16;
static const uint c_SizeOfCurveRadius = 4;

// Define the sizes of these structures because FXC doesn't support sizeof(x)
static const uint c_SizeOfGeometryData = 4 * 16;
static const uint c_SizeOfInstanceData = 7 * 16;
static const uint c_SizeOfMaterialConstants = 13 * 16;

GeometryData LoadGeometryData(ByteAddressBuffer buffer, uint offset)
{
	uint4 a = buffer.Load4(offset + 16 * 0);
	uint4 b = buffer.Load4(offset + 16 * 1);
	uint4 c = buffer.Load4(offset + 16 * 2);
	uint4 d = buffer.Load4(offset + 16 * 3);

	GeometryData ret;
	ret.numIndices = a.x;
	ret.numVertices = a.y;
	ret.indexBufferIndex = int(a.z);
	ret.indexOffset = a.w;
	ret.vertexBufferIndex = int(b.x);
	ret.positionOffset = b.y;
	ret.prevPositionOffset = b.z;
	ret.texCoord1Offset = b.w;
	ret.texCoord2Offset = c.x;
	ret.normalOffset = c.y;
	ret.tangentOffset = c.z;
	ret.curveRadiusOffset = c.w;
	ret.materialIndex = d.x;
	ret.pad0 = d.y;
	ret.pad1 = d.z;
	ret.pad2 = d.w;
	return ret;
}

InstanceData LoadInstanceData(ByteAddressBuffer buffer, uint offset)
{
	uint4 a = buffer.Load4(offset + 16 * 0);
	uint4 b = buffer.Load4(offset + 16 * 1);
	uint4 c = buffer.Load4(offset + 16 * 2);
	uint4 d = buffer.Load4(offset + 16 * 3);
	uint4 e = buffer.Load4(offset + 16 * 4);
	uint4 f = buffer.Load4(offset + 16 * 5);
	uint4 g = buffer.Load4(offset + 16 * 6);

	InstanceData ret;
	ret.flags = a.x;
	ret.firstGeometryInstanceIndex = a.y;
	ret.firstGeometryIndex = a.z;
	ret.numGeometries = a.w;
	ret.transform = float3x4(asfloat(b), asfloat(c), asfloat(d));
	ret.prevTransform = float3x4(asfloat(e), asfloat(f), asfloat(g));
	return ret;
}

MaterialConstants LoadMaterialConstants(ByteAddressBuffer buffer, uint offset)
{
	uint4 a = buffer.Load4(offset + 16 * 0);
	uint4 b = buffer.Load4(offset + 16 * 1);
	uint4 c = buffer.Load4(offset + 16 * 2);
	uint4 d = buffer.Load4(offset + 16 * 3);
	uint4 e = buffer.Load4(offset + 16 * 4);
	uint4 f = buffer.Load4(offset + 16 * 5);
	uint4 g = buffer.Load4(offset + 16 * 6);
	uint4 h = buffer.Load4(offset + 16 * 7);
	uint4 i = buffer.Load4(offset + 16 * 8);
	uint4 j = buffer.Load4(offset + 16 * 9);
	uint4 k = buffer.Load4(offset + 16 * 10);
	uint4 l = buffer.Load4(offset + 16 * 11);
	uint4 m = buffer.Load4(offset + 16 * 12);

	MaterialConstants ret;
	ret.baseOrDiffuseColor = asfloat(a.xyz);
	ret.flags = int(a.w);
	ret.specularColor = asfloat(b.xyz);
	ret.materialID = int(b.w);
	ret.emissiveColor = asfloat(c.xyz);
	ret.domain = int(c.w);
	ret.opacity = asfloat(d.x);
	ret.roughness = asfloat(d.y);
	ret.metalness = asfloat(d.z);
	ret.normalTextureScale = asfloat(d.w);
	ret.occlusionStrength = asfloat(e.x);
	ret.alphaCutoff = asfloat(e.y);
	ret.transmissionFactor = asfloat(e.z);
	ret.baseOrDiffuseTextureIndex = int(e.w);
	ret.metalRoughOrSpecularTextureIndex = int(f.x);
	ret.emissiveTextureIndex = int(f.y);
	ret.normalTextureIndex = int(f.z);
	ret.occlusionTextureIndex = int(f.w);
	ret.transmissionTextureIndex = int(g.x);
	ret.opacityTextureIndex = int(g.y);
	ret.normalTextureTransformScale = asfloat(g.zw);
	ret.padding1 = h.xyz;
	ret.sssScale = int(h.w);
	ret.sssTransmissionColor = asfloat(i.xyz);
	ret.sssAnisotropy = asfloat(i.w);
	ret.sssScatteringColor = asfloat(j.xyz);
	ret.hairMelanin = asfloat(j.w);
	ret.hairBaseColor = asfloat(k.xyz);
	ret.hairMelaninRedness = asfloat(k.w);
	ret.hairLongitudinalRoughness = asfloat(l.x);
	ret.hairAzimuthalRoughness = asfloat(l.y);
	ret.hairIor = asfloat(l.z);
	ret.hairCuticleAngle = asfloat(l.w);
	ret.hairDiffuseReflectionTint = asfloat(m.xyz);
	ret.hairDiffuseReflectionWeight = asfloat(m.w);
	return ret;
}

#endif

#endif // BINDLESS_H_