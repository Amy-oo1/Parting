#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(RHI, InputLayout)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {

	enum class RHIVertexAttribute :Uint8 {//Joint ? maybe in render lay
		Position,
		PrevPosition,
		TexCoord1,
		TexCoord2,
		Normal,
		Tangent,
		Transform,
		PrevTransform,
		JointIndices,
		JointWeights,
		CurveRadius,

		COUNT
	};


	PARTING_EXPORT struct RHIVertexAttributeDesc final {
		RHIVertexAttribute Attribute{ RHIVertexAttribute::COUNT };
		String Name;
		RHIFormat Format{ RHIFormat::UNKNOWN };
		Uint32 ArrayCount{ 1 };
		Uint32 BufferIndex{ 0 };
		Uint32 Offset{ 0 };
		// note: for most APIs, all strides for a given bufferIndex must be identical
		Uint32 ElementStride{ 0 };
		bool IsInstanced{ false };
	};

	PARTING_EXPORT class RHIVertexAttributeDescBuilder final {
	public:
		constexpr RHIVertexAttributeDescBuilder& Reset(void) { this->m_Desc = RHIVertexAttributeDesc{}; return *this; }//NOTE :cpp20 constexpr string

		constexpr RHIVertexAttributeDescBuilder& Set_Attribute(RHIVertexAttribute attribute) { this->m_Desc.Attribute = attribute; return *this; }
		RHIVertexAttributeDescBuilder& Set_Name(const String& name) { this->m_Desc.Name = name; return *this; }
		constexpr RHIVertexAttributeDescBuilder& Set_Format(RHIFormat format) { this->m_Desc.Format = format; return *this; }
		constexpr RHIVertexAttributeDescBuilder& Set_ArrayCount(Uint32 count) { this->m_Desc.ArrayCount = count; return *this; }
		constexpr RHIVertexAttributeDescBuilder& Set_BufferIndex(Uint32 index) { this->m_Desc.BufferIndex = index; return *this; }
		constexpr RHIVertexAttributeDescBuilder& Set_Offset(Uint32 offset) { this->m_Desc.Offset = offset; return *this; }
		constexpr RHIVertexAttributeDescBuilder& Set_ElementStride(Uint32 stride) { this->m_Desc.ElementStride = stride; return *this; }
		constexpr RHIVertexAttributeDescBuilder& Set_IsInstanced(bool isInstanced) { this->m_Desc.IsInstanced = isInstanced; return *this; }

		STDNODISCARD constexpr const RHIVertexAttributeDesc& Build(void)const { return this->m_Desc; }
	private:
		RHIVertexAttributeDesc m_Desc{};
	};

	PARTING_EXPORT template<typename Derived>
		class RHIInputLayout :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHIInputLayout(void) = default;
			virtual ~RHIInputLayout(void) = default;

		public:
			STDNODISCARD Uint32 Get_AttributeCount(void)const { return this->Get_Derived()->Imp_Get_AttributeCount(); }
			STDNODISCARD const RHIVertexAttributeDesc* Get_AttributeDesc(Uint32 Index)const { return this->Get_Derived()->Imp_Get_AttributeDesc(Index); }

		private:
			STDNODISCARD constexpr Derived* Get_Derived(void) const { return static_cast<Derived*>(this); }
		private:
			Uint32 Imp_Get_AttributeCount(void)const { LOG_ERROR("No Imp"); return 0; }
			const RHIVertexAttributeDesc* Imp_Get_AttributeDesc(Uint32 Index)const { LOG_ERROR("No Imp"); return nullptr; }

	};

}