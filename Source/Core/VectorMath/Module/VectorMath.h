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
}