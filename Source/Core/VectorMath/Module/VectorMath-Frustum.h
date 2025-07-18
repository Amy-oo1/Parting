#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(VectorMath, Vec)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

//#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include<cmath>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#include "Core/VectorMath/Module/VectorMath-Misc.h"
#include "Core/VectorMath/Module/VectorMath-Vec.h"
#include "Core/VectorMath/Module/VectorMath-Mat.h"
#include "Core/VectorMath/Module/VectorMath-Affine.h"
#include "Core/VectorMath/Module/VectorMath-Quaternion.h"
#include "Core/VectorMath/Module/VectorMath-Box.h"

#endif // PARTING_MODULE_BUILD


namespace Math {

	// a plane equation, so that any point (v) for which (dot(normal, v) == distance) lies on the plane
	struct Plane final {
		Vec<float, 3> m_Normal{ Vec<float, 3> ::Zero() };
		float m_Distance{ 0 };

		constexpr Plane(void) = default;
		constexpr Plane(const Plane&) = default;
		constexpr Plane(const Vec<float, 3>& N, float d) : m_Normal{ N }, m_Distance{ d } {}
		constexpr Plane(float x, float y, float z, float d) : m_Normal{ x, y, z }, m_Distance{ d } {}

		Plane Normalize(void) const {
			const auto lengthSq{ Dot(this->m_Normal, this->m_Normal) };
			const auto scale{ lengthSq > 0.f ? (1.0f / Sqrt(lengthSq)) : 0 };

			return Plane{ this->m_Normal * scale ,this->m_Distance * scale };
		}

		constexpr bool Is_Empty(void) const { return All(this->m_Normal == 0.f); }//TODO ; float must in right ... so nedd a  operator(float ,vec)....
	};

	// six planes, normals pointing outside of the volume
	struct Frustum final {
		enum class PlaneType :Uint8 {
			Near = 0,
			Far,
			Left,
			Right,
			Top,
			Bottom,

			COUNT
		};

		enum class Corners :Uint8 {
			None = 0b000u,

			Left = 0b000u,
			Right = 0b001u,
			Bottom = 0b000u,
			Top = 0b010u,
			Near = 0b000u,
			Far = 0b100u
		};
		FRIEDND_ENUM_CLASS_OPERATORS(Corners)


			Array<Plane, Tounderlying(PlaneType::COUNT)> m_Planes;

		Frustum(void) = default;
		Frustum(const Frustum&) = default;

		Frustum(const Mat<float, 4, 4>& viewProjMatrix, bool isReverseProjection);

		bool IntersectsWith(const Vec<float, 3>& point) const;
		bool IntersectsWith(const Box<float, 3>& Box) const;

		static constexpr Uint32 NumCorners{ 8 };

		Vec<float, 3> Get_Corner(Corners index) const;

		Frustum Normalize(void) const {
			Frustum Re;
			for (Uint32 Index = 0; const auto & plane:this->m_Planes)
				Re.m_Planes[Index++] = plane.Normalize();
			return Re;
		}

		Frustum Grow(float distance) const {
			Frustum Re;
			for (Uint32 Index = 0; const auto & plane : this->m_Planes) {
				Re.m_Planes[Index] = plane.Normalize();
				Re.m_Planes[Index++].m_Distance += distance;
			}
			return Re;
		}

		bool Is_Empty(void) const;		 // returns true if the frustum trivially rejects all points; does *not* analyze cases when plane equations are mutually exclusive
		bool Is_Open(void) const;		 // returns true if the frustum has at least one plane that trivially accepts all points
		bool Is_Infinite(void) const;	   // returns true if the frustum trivially accepts all points

		//TODO : Func should be less, be like Get_Plane(PlaneType::Near) or Get_Plane(PlaneType::Far) etc
		STDNODISCARD Plane& Get_Pline(PlaneType type) { return this->m_Planes[Tounderlying(type)]; }
		STDNODISCARD const Plane& Get_Pline(PlaneType type) const { return this->m_Planes[Tounderlying(type)]; }

		void Set_Plane(PlaneType type, const Plane& plane) { this->m_Planes[Tounderlying(type)] = plane; }

		//NOTE :not use it
		//Plane& Get_NearPlane(void) { return this->m_Planes[Tounderlying(PlaneType::Near)]; }
		//Plane& Get_FarPlane(void) { return this->m_Planes[Tounderlying(PlaneType::Far)]; }
		//Plane& Get_LeftPlane(void) { return this->m_Planes[Tounderlying(PlaneType::Left)]; }
		//Plane& Get_RightPlane(void) { return this->m_Planes[Tounderlying(PlaneType::Right)]; }
		//Plane& Get_TopPlane(void) { return this->m_Planes[Tounderlying(PlaneType::Top)]; }
		//Plane& Get_BottomPlane(void) { return this->m_Planes[Tounderlying(PlaneType::Bottom)]; }

		//void Set_NearPlane(const Plane& plane) { this->m_Planes[Tounderlying(PlaneType::Near)] = plane; }
		//void Set_FarPlane(const Plane& plane) { this->m_Planes[Tounderlying(PlaneType::Far)] = plane; }
		//void Set_LeftPlane(const Plane& plane) { this->m_Planes[Tounderlying(PlaneType::Left)] = plane; }
		//void Set_RightPlane(const Plane& plane) { this->m_Planes[Tounderlying(PlaneType::Right)] = plane; }
		//void Set_TopPlane(const Plane& plane) { this->m_Planes[Tounderlying(PlaneType::Top)] = plane; }
		//void Set_BottomPlane(const Plane& plane) { this->m_Planes[Tounderlying(PlaneType::Bottom)] = plane; }

		//const Plane& Get_NearPlane(void) const { return this->m_Planes[Tounderlying(PlaneType::Near)]; }
		//const Plane& Get_FarPlane(void) const { return this->m_Planes[Tounderlying(PlaneType::Far)]; }
		//const Plane& Get_LeftPlane(void) const { return this->m_Planes[Tounderlying(PlaneType::Left)]; }
		//const Plane& Get_RightPlane(void) const { return this->m_Planes[Tounderlying(PlaneType::Right)]; }
		//const Plane& Get_TopPlane(void) const { return this->m_Planes[Tounderlying(PlaneType::Top)]; }
		//const Plane& Get_BottomPlane(void) const { return this->m_Planes[Tounderlying(PlaneType::Bottom)]; }

		STDNODISCARD static Frustum Empty(void);    // a frustum that doesn't intersect with any points
		STDNODISCARD static Frustum Infinite(void); // a frustum that intersects with all points

		STDNODISCARD static Frustum FromBox(const Box<float, 3>& b);
	};


	Math::Frustum::Frustum(const Mat<float, 4, 4>& m, bool isReverseProjection) {
		this->m_Planes[Tounderlying(PlaneType::Near)] = Plane{ -m[0].Z, -m[1].Z, -m[2].Z, m[3].Z };
		this->m_Planes[Tounderlying(PlaneType::Far)] = Plane{ -m[0].W + m[0].Z, -m[1].W + m[1].Z, -m[2].W + m[2].Z, m[3].W - m[3].Z };

		if (isReverseProjection)
			Swap(this->m_Planes[Tounderlying(PlaneType::Near)], this->m_Planes[Tounderlying(PlaneType::Far)]);

		this->m_Planes[Tounderlying(PlaneType::Left)] = Plane{ -m[0].W - m[0].X, -m[1].W - m[1].X, -m[2].W - m[2].X, m[3].W + m[3].X };
		this->m_Planes[Tounderlying(PlaneType::Right)] = Plane{ -m[0].W + m[0].X, -m[1].W + m[1].X, -m[2].W + m[2].X, m[3].W - m[3].X };

		this->m_Planes[Tounderlying(PlaneType::Top)] = Plane{ -m[0].W + m[0].Y, -m[1].W + m[1].Y, -m[2].W + m[2].Y, m[3].W - m[3].Y };
		this->m_Planes[Tounderlying(PlaneType::Bottom)] = Plane{ -m[0].W - m[0].Y, -m[1].W - m[1].Y, -m[2].W - m[2].Y, m[3].W + m[3].Y };

		*this = this->Normalize();
	}

	inline bool Frustum::IntersectsWith(const Vec<float, 3>& point) const {
		for (const auto& plane : this->m_Planes)
			if (Dot(plane.m_Normal, point) > plane.m_Distance)
				return false;

		return true;
	}

	inline bool Frustum::IntersectsWith(const Box<float, 3>& Box) const {
		for (const auto& plane : this->m_Planes) {
			float x = plane.m_Normal.X > 0 ? Box.m_Mins.X : Box.m_Maxs.X;
			float y = plane.m_Normal.Y > 0 ? Box.m_Mins.Y : Box.m_Maxs.Y;
			float z = plane.m_Normal.Z > 0 ? Box.m_Mins.Z : Box.m_Maxs.Z;

			float distance{
				plane.m_Normal.X * x +
				plane.m_Normal.Y * y +
				plane.m_Normal.Z * z -
				plane.m_Distance
			};

			if (distance > 0.f)
				return false;
		}

		return true;
	}


	inline Vec<float, 3> Frustum::Get_Corner(Corners index) const {
		const auto& a{ Corners::None != (index & static_cast<Corners>(0b001u)) ? this->m_Planes[Tounderlying(PlaneType::Right)] : this->m_Planes[Tounderlying(PlaneType::Left)] };
		const auto& b{ Corners::None != (index & static_cast<Corners>(0b010u)) ? this->m_Planes[Tounderlying(PlaneType::Top)] : this->m_Planes[Tounderlying(PlaneType::Bottom)] };
		const auto& c{ Corners::None != (index & static_cast<Corners>(0b100u)) ? this->m_Planes[Tounderlying(PlaneType::Far)] : this->m_Planes[Tounderlying(PlaneType::Near)] };

		Mat<float, 3, 3> m{ a.m_Normal, b.m_Normal, c.m_Normal };
		Vec<float, 3> d{ a.m_Distance, b.m_Distance, c.m_Distance };
		return Inverse(m) * d;
	}

	inline bool Frustum::Is_Empty(void) const {
		for (const auto& plane : this->m_Planes)
			if (All(plane.m_Normal == 0.f) && plane.m_Distance < 0.f)
				return false;

		return true;
	}

	inline bool Frustum::Is_Open(void) const {
		if (this->Is_Empty())
			return false;

		for (const auto& plane : this->m_Planes)
			if (All(plane.m_Normal == 0.f) && plane.m_Distance >= 0.f)
				return true;

		return false;
	}

	inline bool Frustum::Is_Infinite(void) const {
		for (const auto& plane : this->m_Planes)
			if (!All(plane.m_Normal == 0.f) && plane.m_Distance >= 0.f)
				return false;

		return true;
	}

	inline Frustum Frustum::Empty(void) {
		Frustum Re;
		// (dot(normal, v) - distance) positive for any v => any point is outside
		for (auto& plne : Re.m_Planes)
			plne = Plane{ Vec<float, 3>::Zero(), -1.f };

		return Re;
	}

	inline Frustum Frustum::Infinite(void) {
		Frustum Re;
		// (dot(normal, v) - distance) negative for any v => any point is inside
		for (auto& plne : Re.m_Planes)
			plne = Plane{ Vec<float, 3>::Zero(), 1.f };

		return Re;
	}

	inline Frustum Frustum::FromBox(const Box<float, 3>& b) {
		Frustum f;

		f.m_Planes[Tounderlying(PlaneType::Left)] = Plane{ Vec<float,3>{-1.f, 0.f, 0.f}, -b.m_Mins.X };
		f.m_Planes[Tounderlying(PlaneType::Right)] = Plane{ Vec<float,3>{1.f, 0.f, 0.f}, b.m_Maxs.X };
		f.m_Planes[Tounderlying(PlaneType::Bottom)] = Plane{ Vec<float,3>{0.f, -1.f, 0.f}, -b.m_Mins.Y };
		f.m_Planes[Tounderlying(PlaneType::Top)] = Plane{ Vec<float,3>{0.f, 1.f, 0.f}, b.m_Maxs.Y };
		f.m_Planes[Tounderlying(PlaneType::Near)] = Plane{ Vec<float,3>{0.f, 0.f, -1.f}, -b.m_Mins.Z };
		f.m_Planes[Tounderlying(PlaneType::Far)] = Plane{ Vec<float,3>{0.f, 0.f, 1.f}, b.m_Maxs.Z };

		return f;
	}

}