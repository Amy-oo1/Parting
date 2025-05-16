#pragma once

#include "Core/Algorithm/Module/Algorithm.h"

#include<limits>
#include<cmath>
#include<algorithm>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#include "Core/VectorMath/Module/VectorMath-Misc.h"
#include "Core/VectorMath/Module/VectorMath-Vec.h"
#include "Core/VectorMath/Module/VectorMath-Mat.h"
#include "Core/VectorMath/Module/VectorMath-Affine.h"
#include "Core/VectorMath/Module/VectorMath-Box.h"
#include "Core/VectorMath/Module/VectorMath-Frustum.h"

namespace Math {
	using VecF2 = Vec<float, 2>;
	using VecF3 = Vec<float, 3>;
	using VecF4 = Vec<float, 4>;

	using VecI2 = Vec<Int32, 2>;
	using VecI3 = Vec<Int32, 3>;
	using VecI4 = Vec<Int32, 4>;

	using VecU2 = Vec<Uint32, 2>;
	using VecU3 = Vec<Uint32, 3>;
	using VecU4 = Vec<Uint32, 4>;

	using MatF22 = Mat<float, 2, 2>;
	using MatF33 = Mat<float, 3, 3>;
	using MatF34 = Mat<float, 3, 4>;
	using MatF44 = Mat<float, 4, 4>;

	using AffineF2 = Affine<float, 2>;
	using AffineF3 = Affine<float, 3>;
	using AffineF4 = Affine<float, 4>;

	using BoxF2 = Box<float, 2>;
	using BoxF3 = Box<float, 3>;

	using BoxI2 = Box<Int32, 2>;
	using BoxI3 = Box<Int32, 3>;



	MatF44 OrthoProjD3DStyle(float left, float right, float bottom, float top, float zNear, float zFar) {
		float xScale{ 1.0f / (right - left) };
		float yScale{ 1.0f / (top - bottom) };
		float zScale{ 1.0f / (zFar - zNear) };
		return MatF44{
			2.0f * xScale,				0.f,						0.f,				0.f,
			0.f,						2.0f * yScale,				0.f,				0.f,
			0.f,						0.f,						zScale,				0.f,
			-(left + right) * xScale,	-(bottom + top) * yScale,	-zNear * zScale,	1.f
		};
	}


	// Fast shortcut for float3x4(transpose(affineToHomogenous(a)))
	// Useful for storing transformations in buffers and passing them to ray tracing APIs
	inline void AffineToColumnMajor(AffineF3 const& a, float m[12]) {
		m[0] = a.m_Linear.M00;
		m[1] = a.m_Linear.M10;
		m[2] = a.m_Linear.M20;
		m[3] = a.m_Translation.X;
		m[4] = a.m_Linear.M01;
		m[5] = a.m_Linear.M11;
		m[6] = a.m_Linear.M21;
		m[7] = a.m_Translation.Y;
		m[8] = a.m_Linear.M02;
		m[9] = a.m_Linear.M12;
		m[10] = a.m_Linear.M22;
		m[11] = a.m_Translation.Z;
	}




}