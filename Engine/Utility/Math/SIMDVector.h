// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>

/*
namespace SIMDMath
{
	typedef __m128 vec3;
	typedef __m128 vec4;

	inline vec3 ToVec3( glm::vec3 vec )
	{
		return _mm_setr_ps( vec.x, vec.y, vec.z, 0.0f );
	}

	inline vec4 ToVec4( glm::vec4 vec )
	{
		return _mm_setr_ps( vec.x, vec.y, vec.z, vec.w );
	}

	inline vec3 Subtract( vec3 p, vec3 v )
	{
		return _mm_sub_ps( p, v );
	}

	inline float Length( vec3 v )
	{
		return _mm_cvtss_f32( _mm_sqrt_ss( _mm_dp_ps( v, v, 0x71 ) ) );
	}

	inline float Distance( vec3 p, vec3 v )
	{
		return Length( Subtract( p, v ) );
	}

	inline vec3 Dot( vec3 v, vec3 w )
	{
		return _mm_dp_ps( v, w, 0x71 );
	}
}*/

namespace Math
{
	class SIMDVector4D
	{
	public:
		SIMDVector4D()
		{
			StoredVector = _mm_setr_ps( 0.0f, 0.0f, 0.0f, 0.0f );
		};

		SIMDVector4D( const glm::vec3& Vector )
		{
			AssignVector( Vector );
		}

		SIMDVector4D( const glm::vec4& Vector )
		{
			AssignVector( Vector );
		}

		inline void AssignVector( const glm::vec3& Vector )
		{
			StoredVector = _mm_setr_ps( Vector.x, Vector.y, Vector.z, 0.0f );
		}

		inline void AssignVector( const glm::vec4& Vector )
		{
			StoredVector = _mm_setr_ps( Vector.x, Vector.y, Vector.z, Vector.w );
		}

		inline void operator=( const glm::vec3& Vector )
		{
			AssignVector( Vector );
		};

		inline void operator=( const glm::vec4& Vector )
		{
			AssignVector( Vector );
		};

		inline glm::vec4 ToVec4() const
		{
			float RawVector[4];
			_mm_storeu_ps( RawVector, StoredVector );
			return glm::vec4( RawVector[0], RawVector[1], RawVector[2], RawVector[3] );
		}

		inline float Length()
		{
			return Length( StoredVector );
		}

		inline __m128 Dot( const __m128& Vector )
		{
			return _mm_dp_ps( StoredVector, Vector, 0x71 );
		}

		inline float Distance( const __m128& Vector )
		{
			return Length( Subtract( StoredVector, Vector ) );
		}

		inline __m128 operator+=( const __m128& Vector )
		{
			StoredVector = Add( StoredVector, Vector );
			return StoredVector;
		};

		inline __m128 operator-=( const __m128& Vector )
		{
			StoredVector = Subtract( StoredVector, Vector );
			return StoredVector;
		};

		inline __m128 operator*=( const __m128& Vector )
		{
			StoredVector = Multiply( StoredVector, Vector );
			return StoredVector;
		};

		// Static functions
		static inline __m128 Add( const __m128& A, const __m128& B )
		{
			return _mm_add_ps( A, B );
		}

		static inline __m128 Subtract( const __m128& A, const __m128& B )
		{
			return _mm_sub_ps( A, B );
		}

		static inline __m128 Multiply( const __m128& A, const __m128& B )
		{
			return _mm_mul_ps( A, B );
		}

		static inline float Length( const __m128& Vector )
		{
			return _mm_cvtss_f32( _mm_sqrt_ss( _mm_dp_ps( Vector, Vector, 0x71 ) ) );
		}
	private:
		__m128 StoredVector;
	};
}