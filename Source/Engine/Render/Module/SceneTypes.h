#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT VectorMath;
PARTING_IMPORT Logger;


PARTING_SUBMODULE(Parting, SSAOPass)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Shader/material_cb.h"
#include "Shader/bindless.h"
#include "Shader/light_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	enum class TextureAlphaMode :Uint8 {
		UNKNOWN = 0,
		STRAIGHT = 1,
		PREMULTIPLIED = 2,
		OPAQUE = 3,
		CUSTOM = 4,
	};

	enum class MaterialDomain : Uint8 {
		Opaque,
		AlphaTested,
		AlphaBlended,
		Transmissive,
		TransmissiveAlphaTested,
		TransmissiveAlphaBlended,

		Count
	};

	template<RHI::APITagConcept APITag>
	struct LoadedTexture {
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;

		RHI::RefCountPtr<typename RHI::RHITypeTraits<APITag>::Imp_Texture> Texture;//TODO Fix
		TextureAlphaMode AlphaMode{ TextureAlphaMode::UNKNOWN };
		Uint32 OriginalBitsPerPixel{ 0 };
		String FilePath;
		String MimeType;
	};


	template<RHI::APITagConcept APITag>
	struct Material final {
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;

		String Name;
		String ModelFileName;		// where this material originated from, e.g. GLTF file name
		Int32 MaterialIndexInModel{ -1 };  // index of the material in the model file
		MaterialDomain Domain{ MaterialDomain::Opaque };
		SharedPtr<LoadedTexture<APITag>> BaseOrDiffuseTexture; // metal-rough: base color; spec-gloss: diffuse color; .a = opacity (both modes)
		SharedPtr<LoadedTexture<APITag>> MetalRoughOrSpecularTexture; // metal-rough: ORM map; spec-gloss: specular color, .a = glossiness
		SharedPtr<LoadedTexture<APITag>> NormalTexture;
		SharedPtr<LoadedTexture<APITag>> EmissiveTexture;
		SharedPtr<LoadedTexture<APITag>> OcclusionTexture;
		SharedPtr<LoadedTexture<APITag>> TransmissionTexture; // see KHR_materials_transmission; undefined on specular-gloss materials
		SharedPtr<LoadedTexture<APITag>> OpacityTexture; // for renderers that store opacity or alpha mask separately, overrides baseOrDiffuse.a
		RHI::RefCountPtr<Imp_Buffer> MaterialConstants;
		Math::VecF3 BaseOrDiffuseColor{ 1.f }; // metal-rough: base color, spec-gloss: diffuse color (if no texture present)
		Math::VecF3 SpecularColor{ 0.f }; // spec-gloss: specular color
		Math::VecF3 EmissiveColor{ 0.f };//TODO :maybe define a default value is 1.f
		float EmissiveIntensity{ 1.f }; // additional multiplier for emissiveColor
		float Metalness{ 0.f }; // metal-rough only
		float Roughness{ 0.f }; // both metal-rough and spec-gloss
		float Opacity{ 1.f }; // for transparent materials; multiplied by diffuse.a if present
		float AlphaCutoff{ 0.5f }; // for alpha tested materials
		float TransmissionFactor{ 0.f }; // see KHR_materials_transmission; undefined on specular-gloss materials
		float NormalTextureScale{ 1.f };
		float OcclusionStrength{ 1.f };
		Math::VecF2 NormalTextureTransformScale{ 1.f };

		// Toggle between two PBR models: metal-rough and specular-gloss.
		// See the comments on the other fields here.
		bool UseSpecularGlossModel{ false };

		// Subsurface Scattering
		bool EnableSubsurfaceScattering{ false };
		struct SubsurfaceParams final {
			Math::VecF3 TransmissionColor{ 0.5f };
			Math::VecF3 ScatteringColor{ 0.5f };
			float Scale{ 1.f };
			float Anisotropy{ 0.f };
		} Subsurface;

		// Hair
		bool EnableHair{ false };
		struct HairParams final {
			Math::VecF3 BaseColor{ 1.0f };
			float Melanin{ 0.5f };
			float MelaninRedness{ 0.5f };
			float LongitudinalRoughness{ 0.25f };
			float AzimuthalRoughness{ 0.6f };
			float DiffuseReflectionWeight{ 0.0f };
			Math::VecF3 DiffuseReflectionTint{ 0.0f };
			float IOR{ 1.55f };
			float CuticleAngle{ 3.0f };
		} Hair;

		// Toggles for the textures. Only effective if the corresponding texture is non-null.
		bool EnableBaseOrDiffuseTexture{ true };
		bool EnableMetalRoughOrSpecularTexture{ true };
		bool EnableNormalTexture{ true };
		bool EnableEmissiveTexture{ true };
		bool EnableOcclusionTexture{ true };
		bool EnableTransmissionTexture{ true };
		bool EnableOpacityTexture{ true };

		bool DoubleSided{ false };

		// Useful when metalness and roughness are packed into a 2-channel texture for BC5 encoding.
		bool MetalnessInRedChannel{ false };

		int MaterialID{ 0 };
		bool Dirty{ true }; // set this to true to make Scene update the material data


		void FillConstantBuffer(typename ::MaterialConstants& constants) const;
	};


	template<RHI::APITagConcept APITag>
	struct BufferGroup final {
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;

		RHI::RefCountPtr<Imp_Buffer> IndexBuffer;
		RHI::RefCountPtr<Imp_Buffer> VertexBuffer;
		RHI::RefCountPtr<Imp_Buffer> InstanceBuffer;
		Array<RHI::RHIBufferRange, Tounderlying(RHI::RHIVertexAttribute::COUNT)> VertexBufferRanges;
		Vector<RHI::RHIBufferRange> MorphTargetBufferRange;
		Vector<Uint32> IndexData;
		Vector<Math::VecF3> PositionData;
		Vector<Math::VecF2> Texcoord1Data;
		Vector<Math::VecF2> Texcoord2Data;
		Vector<Uint32> NormalData;
		Vector<Uint32> TangentData;
		Vector<Math::Vec<Uint16, 4>> JointData;
		Vector<Math::VecF4> WeightData;
		Vector<float> RadiusData;
		Vector<Math::VecF4> MorphTargetData;

		STDNODISCARD bool HasAttribute(RHI::RHIVertexAttribute attr) const { return this->VertexBufferRanges[Tounderlying(attr)].ByteSize != 0; }
		void Set_VertexBufferRange(RHI::RHIVertexAttribute attr, const RHI::RHIBufferRange& range) { this->VertexBufferRanges[Tounderlying(attr)] = range; }
		RHI::RHIBufferRange& Get_VertexBufferRange(RHI::RHIVertexAttribute attr) { return this->VertexBufferRanges[Tounderlying(attr)]; }//NOTE :return no const to w range//TODO 
		STDNODISCARD const RHI::RHIBufferRange& Get_VertexBufferRange(RHI::RHIVertexAttribute attr) const { return this->VertexBufferRanges[Tounderlying(attr)]; }
	};


	enum class MeshGeometryPrimitiveType : Uint8 {
		Triangles,
		Lines,
		LineStrip,

		COUNT
	};

	template<RHI::APITagConcept APITag>
	struct MeshGeometry final {
		SharedPtr<Material<APITag>> Material;
		Math::BoxF3 ObjectSpaceBounds;
		Uint32 IndexOffsetInMesh{ 0 };
		Uint32 VertexOffsetInMesh{ 0 };
		Uint32 NumIndices{ 0 };
		Uint32 NumVertices{ 0 };
		Int32 GlobalGeometryIndex{ 0 };

		MeshGeometryPrimitiveType Type{ MeshGeometryPrimitiveType::Triangles };
	};

	enum class MeshType : Uint8 {
		Triangles,
		CurvePolytubes,
		CurveDisjointOrthogonalTriangleStrips,
		CurveLinearSweptSpheres,

		COUNT
	};

	template<RHI::APITagConcept APITag>
	struct MeshInfo final {
		String Name;
		MeshType Type{ MeshType::Triangles };
		SharedPtr<BufferGroup<APITag>> Buffers;
		SharedPtr<MeshInfo<APITag>> SkinPrototype;
		Vector<SharedPtr<MeshGeometry<APITag>>> Geometries;
		Math::BoxF3 ObjectSpaceBounds;
		Uint32 IndexOffset{ 0 };
		Uint32 VertexOffset{ 0 };
		Uint32 TotalIndices{ 0 };
		Uint32 TotalVertices{ 0 };
		Int32 GlobalMeshIndex{ 0 };
		bool IsMorphTargetAnimationMesh{ false };
		bool IsSkinPrototype{ false };

		bool Is_Curve(void) const {
			return
				(this->Type == MeshType::CurvePolytubes) ||
				(this->Type == MeshType::CurveDisjointOrthogonalTriangleStrips) ||
				(this->Type == MeshType::CurveLinearSweptSpheres);
		}
	};

	template<RHI::APITagConcept APITag>
	struct LightProbe final {
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;

		String Name;
		RHI::RefCountPtr<Imp_Texture> DiffuseMap;
		RHI::RefCountPtr<Imp_Texture> SpecularMap;
		RHI::RefCountPtr<Imp_Texture> EnvironmentBRDF;
		Uint32 DiffuseArrayIndex{ 0 };
		Uint32 SpecularArrayIndex{ 0 };
		float DiffuseScale{ 1.f };
		float SpecularScale{ 1.f };
		bool Enabled{ true };
		Math::Frustum Bounds{ Math::Frustum::Infinite() };

		STDNODISCARD bool Is_Active(void) const;

		void FillLightProbeConstants(::LightProbeConstants& lightProbeConstants) const;
	};





	struct SceneLoadingStats final {
		Atomic<Uint32> ObjectsTotal;
		Atomic<Uint32> ObjectsLoaded;
	};

	HEADER_INLINE SceneLoadingStats g_LoadingStats;

	RHI::RHIVertexAttributeDesc BuildVertexAttributeDesc(RHI::RHIVertexAttributeDescBuilder& builder, RHI::RHIVertexAttribute attribute, const String& name, Uint32 bufferIndex) {
		builder.Set_Attribute(attribute).Set_Name(name).Set_BufferIndex(bufferIndex);

		switch (attribute) {
			using enum RHI::RHIVertexAttribute;;
		case Position:case PrevPosition:
			builder.Set_Format(RHI::RHIFormat::RGB32_FLOAT).Set_ElementStride(sizeof(Math::VecF3));
			break;

		case TexCoord1:case TexCoord2:
			builder.Set_Format(RHI::RHIFormat::RG32_FLOAT).Set_ElementStride(sizeof(Math::VecF2));
			break;

		case Normal:case Tangent:
			builder.Set_Format(RHI::RHIFormat::RGBA8_SNORM).Set_ElementStride(sizeof(Uint32));
			break;

		case Transform:
			builder.Set_Format(RHI::RHIFormat::RGBA32_FLOAT).Set_ElementStride(sizeof(InstanceData))
				.Set_ArrayCount(3)
				.Set_Offset(offsetof(InstanceData, Transform))
				.Set_IsInstanced(true);
			break;

		case PrevTransform:
			builder.Set_Format(RHI::RHIFormat::RGBA32_FLOAT).Set_ElementStride(sizeof(InstanceData))
				.Set_ArrayCount(3)
				.Set_Offset(offsetof(InstanceData, PrevTransform))
				.Set_IsInstanced(true);
			break;

		case COUNT: default:
			ASSERT(false);
			break;
		}

		return builder.Build();
	}

	template<RHI::APITagConcept APITag>
	inline void Material<APITag>::FillConstantBuffer(typename ::MaterialConstants& constants) const {
		constants.Flags = 0;

		if (this->UseSpecularGlossModel)
			constants.Flags |= MaterialFlags_UseSpecularGlossModel;

		if (nullptr != this->BaseOrDiffuseTexture && this->EnableBaseOrDiffuseTexture)
			constants.Flags |= MaterialFlags_UseBaseOrDiffuseTexture;

		if (nullptr != this->MetalRoughOrSpecularTexture && this->EnableMetalRoughOrSpecularTexture)
			constants.Flags |= MaterialFlags_UseMetalRoughOrSpecularTexture;

		if (nullptr != EmissiveTexture && this->EnableEmissiveTexture)
			constants.Flags |= MaterialFlags_UseEmissiveTexture;

		if (nullptr != this->NormalTexture && this->EnableNormalTexture)
			constants.Flags |= MaterialFlags_UseNormalTexture;

		if (nullptr != this->OcclusionTexture && this->EnableOcclusionTexture)
			constants.Flags |= MaterialFlags_UseOcclusionTexture;

		if (nullptr != this->TransmissionTexture && this->EnableTransmissionTexture)
			constants.Flags |= MaterialFlags_UseTransmissionTexture;

		if (nullptr != this->OpacityTexture && this->EnableOpacityTexture)
			constants.Flags |= MaterialFlags_UseOpacityTexture;

		if (this->DoubleSided)
			constants.Flags |= MaterialFlags_DoubleSided;

		if (this->MetalnessInRedChannel)
			constants.Flags |= MaterialFlags_MetalnessInRedChannel;

		// free parameters

		constants.Domain = static_cast<Int32>(this->Domain);
		constants.BaseOrDiffuseColor = this->BaseOrDiffuseColor;
		constants.SpecularColor = this->SpecularColor;
		constants.EmissiveColor = this->EmissiveColor * this->EmissiveIntensity;
		constants.Roughness = this->Roughness;
		constants.Metalness = this->Metalness;
		constants.NormalTextureScale = this->NormalTextureScale;
		constants.MaterialID = this->MaterialID;
		constants.OcclusionStrength = this->OcclusionStrength;
		constants.TransmissionFactor = this->TransmissionFactor;
		constants.NormalTextureTransformScale = this->NormalTextureTransformScale;

		switch (this->Domain) {
			using enum MaterialDomain;
		case AlphaBlended:case TransmissiveAlphaBlended:
			constants.Opacity = this->Opacity;
			break;

		case Opaque:case AlphaTested:case Transmissive:case TransmissiveAlphaTested:
			constants.Opacity = 1.f;
			break;

		default:break;
		}

		switch (this->Domain) {
			using enum MaterialDomain;
		case AlphaTested:case TransmissiveAlphaTested:
			constants.AlphaCutoff = this->AlphaCutoff;
			break;

		case AlphaBlended:case TransmissiveAlphaBlended:
			constants.AlphaCutoff = 0.f; // discard only if opacity == 0
			break;

		case Opaque:case Transmissive:
			constants.AlphaCutoff = -1.f; // never discard
			break;

		default:break;
		}

		if (this->EnableSubsurfaceScattering) {
			constants.Flags |= MaterialFlags_SubsurfaceScattering;

			constants.SSSTransmissionColor = this->Subsurface.TransmissionColor;
			constants.SSSScatteringColor = this->Subsurface.ScatteringColor;
			constants.SSSScale = this->Subsurface.Scale;
			constants.SSSAnisotropy = this->Subsurface.Anisotropy;
		}

		if (this->EnableHair) {
			constants.Flags |= MaterialFlags_Hair;

			constants.HairBaseColor = this->Hair.BaseColor;
			constants.HairMelanin = this->Hair.Melanin;
			constants.HairMelaninRedness = this->Hair.MelaninRedness;
			constants.HairLongitudinalRoughness = this->Hair.LongitudinalRoughness;
			constants.HairAzimuthalRoughness = this->Hair.AzimuthalRoughness;
			constants.HairIOR = this->Hair.IOR;
			constants.HairCuticleAngle = this->Hair.CuticleAngle;
			constants.HairDiffuseReflectionWeight = this->Hair.DiffuseReflectionWeight;
			constants.HairDiffuseReflectionTint = this->Hair.DiffuseReflectionTint;
		}
	}

	template<RHI::APITagConcept APITag>
	inline bool LightProbe<APITag>::Is_Active(void) const {
		if (!this->Enabled)
			return false;
		if (this->Bounds.Is_Empty())
			return false;
		if ((this->DiffuseScale == 0.f || nullptr == this->DiffuseMap) && (this->SpecularScale == 0.f || nullptr == this->SpecularMap))
			return false;

		return true;
	}

	template<RHI::APITagConcept APITag>
	inline void LightProbe<APITag>::FillLightProbeConstants(::LightProbeConstants& lightProbeConstants) const {
		lightProbeConstants.DiffuseArrayIndex = this->DiffuseArrayIndex;
		lightProbeConstants.SpecularArrayIndex = this->SpecularArrayIndex;
		lightProbeConstants.DiffuseScale = this->DiffuseScale;
		lightProbeConstants.SpecularScale = this->SpecularScale;
		lightProbeConstants.MipLevels = this->SpecularMap ? static_cast<float>(this->SpecularMap->Get_Desc().MipLevelCount) : 0.f;

		for (Uint32 nPlane = 0; nPlane < Tounderlying(Math::Frustum::PlaneType::COUNT); ++nPlane)
			lightProbeConstants.FrustumPlanes[nPlane] = Math::VecF4{ this->Bounds.m_Planes[nPlane].m_Normal, this->Bounds.m_Planes[nPlane].m_Distance };
	}

}