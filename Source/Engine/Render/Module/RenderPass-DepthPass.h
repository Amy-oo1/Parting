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

#include "Engine/Render/Module/RenderPass-Base.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/ShaderFactory.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {

	template<RHI::APITagConcept APITag>
	class DepthPass : public IGeometryPass<APITag> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;

	public:
		struct CreateParameters final {
			Int32 DepthBias{ 0 };
			float DepthBiasClamp{ 0.f };
			float SlopeScaledDepthBias{ 0.f };


			// Switches between loading vertex data through the Input Assembler (true) or buffer SRVs (false).
			// Using Buffer SRVs is often faster.
			bool UseInputAssembler{ false };
			bool TrackLiveness{ true };

			Uint32 numConstantBufferVersions{ 16 };
		};

	public:

		DepthPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonRenderPasses) :
			m_Device{ device },
			m_CommonRenderPasses{ MoveTemp(commonRenderPasses) } {
		}

		void DeferInit(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) {
			this->m_UseInputAssembler = params.UseInputAssembler;
			this->m_TrackLiveness = params.TrackLiveness;
		}

	private:
		auto CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;
		auto CreatePixelShadewr(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;
		auto CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout>;


	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		SharedPtr<CommonRenderPasses<APITag>> m_CommonRenderPasses;

		Int32 m_DepthBias{ 0 };
		float m_DepthBiasClamp{ 0.f };
		float m_SlopeScaledDepthBias{ 0.f };
		bool m_UseInputAssembler{ false };
		bool m_TrackLiveness{ true };

	};


	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		return shaderFactory.CreateShader(
			"Parting/Passes/depth_vs.hlsl",
			params.UseInputAssembler ? "input_assembler" : "buffer_loads",
			nullptr,
			RHI::RHIShaderType::Vertex
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreatePixelShadewr(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		return shaderFactory.CreateStaticShader("Parting/Passes/depth_ps.hlsl", "main", nullptr, RHI::RHIShaderType::Pixel);
	}

	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout> {
		/*if (params.UseInputAssembler) {
			return this->m_Device->CreateInputLayout(RHI::RHIVertexAttributeDescBuilder{}
			)
		}*/

		//TODO :builder has to add some func to create input layout
	}

}