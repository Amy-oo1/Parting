#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(DirectX12Wrapper)

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<cassert>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX
#include<Windows.h>
#include<time.h>
#include<dxgi1_6.h>
#ifdef _DEBUG
#include<dxgidebug.h>
#endif // _DEBUG

#include<d3d12.h>
#include<d3dcompiler.h>
#include<wrl.h>
#include<pix.h>

#include "ThirdParty/Windows/d3dx12.h"

#endif // PARTING_MODULE_BUILD#pragma once


using Microsoft::WRL::ComPtr;

#define D3D12_CHECK(x) { HRESULT hr = (x); if (FAILED(hr)) assert(false); };

//constexpr HRESULT HRESULT_FROM_WIN32(DWORD dwError) noexcept { return HRESULT_FROM_WIN32(dwError); }

constexpr HRESULT HRusltSucccess{ S_OK };

[[nodiscard]] constexpr auto D3D12EncodeAnisotropicFilter(UINT reductionType) noexcept { return D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType); }

[[nodiscard]] constexpr auto D3D12EnocdeBasicFilter(D3D12_FILTER_TYPE MinFilter, D3D12_FILTER_TYPE MagFilter, D3D12_FILTER_TYPE MipFilter, UINT reductionType) noexcept { return D3D12_ENCODE_BASIC_FILTER(MinFilter, MagFilter, MipFilter, reductionType); }

#define D3D12_SUCCESS(x)  SUCCEEDED(x)