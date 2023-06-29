// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>
#include <math.h>
#include <algorithm>
#include <intrin.h>

class Vector2D
{
public:
	union {
		struct { float X, Y; };
		struct { float R, G; };
		struct { float S, T; };
	};

	Vector2D() = default;
	Vector2D( const float& X, const float& Y )
	{
		this->X = X;
		this->Y = Y;
	}

	Vector2D( const float& A )
	{
		this->X = A;
		this->Y = A;
	}

	float* Base()
	{
		return &X;
	}

	float& operator[]( size_t Index )
	{
		return ( &X )[Index];
	}

	const float& operator[]( size_t Index ) const
	{
		return ( &X )[Index];
	}

	bool operator<( const Vector2D& B ) const
	{
		return X < B.X && Y < B.Y;
	}

	Vector2D& operator=( const Vector2D& Vector )
	{
		this->X = Vector.X;
		this->Y = Vector.Y;
		return *this;
	}

	Vector2D operator+( const Vector2D& Vector ) const
	{
		return Vector2D(
			X + Vector.X,
			Y + Vector.Y
		);
	}

	Vector2D operator-( const Vector2D& Vector ) const
	{
		return Vector2D(
			X - Vector.X,
			Y - Vector.Y
		);
	}

	Vector2D operator*( const Vector2D& Vector ) const
	{
		return Vector2D(
			X * Vector.X,
			Y * Vector.Y
		);
	}

	Vector2D operator/( const Vector2D& Vector ) const
	{
		return Vector2D(
			X / Vector.X,
			Y / Vector.Y
		);
	}

	Vector2D operator+( const float& Scalar ) const
	{
		return Vector2D(
			X + Scalar,
			Y + Scalar
		);
	}

	Vector2D operator-( const float& Scalar ) const
	{
		return Vector2D(
			X - Scalar,
			Y - Scalar
		);
	}

	Vector2D operator*( const float& Scalar ) const
	{
		return Vector2D(
			X * Scalar,
			Y * Scalar
		);
	}

	Vector2D operator/( const float& Scalar ) const
	{
		return Vector2D(
			X / Scalar,
			Y / Scalar
		);
	}

	Vector2D operator+=( const Vector2D& Vector )
	{
		X += Vector.X;
		Y += Vector.Y;
		return *this;
	}

	Vector2D operator-=( const Vector2D& Vector )
	{
		X -= Vector.X;
		Y -= Vector.Y;
		return *this;
	}

	Vector2D operator*=( const Vector2D& Vector )
	{
		X *= Vector.X;
		Y *= Vector.Y;
		return *this;
	}

	Vector2D operator/=( const Vector2D& Vector )
	{
		X /= Vector.X;
		Y /= Vector.Y;
		return *this;
	}

	Vector2D operator+=( const float& Scalar )
	{
		X += Scalar;
		Y += Scalar;
		return *this;
	}

	Vector2D operator-=( const float& Scalar )
	{
		X -= Scalar;
		Y -= Scalar;
		return *this;
	}

	Vector2D operator*=( const float& Scalar )
	{
		X *= Scalar;
		Y *= Scalar;
		return *this;
	}

	Vector2D operator/=( const float& Scalar )
	{
		X /= Scalar;
		Y /= Scalar;
		return *this;
	}

	float Dot( const Vector2D& Vector ) const
	{
		return X * Vector.X + Y * Vector.Y;
	}

	float Length() const
	{
		return sqrt( Dot( *this ) );
	}

	float LengthSquared() const
	{
		return Dot( *this );
	}

	float Length( const Vector2D& Vector ) const
	{
		return sqrt( Vector.Dot( Vector ) );
	}

	float Distance( const Vector2D& Vector ) const
	{
		return Length( *this - Vector );
	}

	float DistanceSquared( const Vector2D& Vector ) const
	{
		const Vector2D Delta = *this - Vector;
		return Delta.Dot( Delta );
	}

	Vector2D Normalized() const
	{
		Vector2D Unit = *this;

		const float VectorLength = Length( Unit );
		if( VectorLength > 0.0f )
		{
			Unit /= VectorLength;
		}

		return Unit;
	}

	float Normalize()
	{
		const float VectorLength = Length( *this );
		if( VectorLength > 0.0f )
		{
			*this /= VectorLength;
		}

		return VectorLength;
	}
};

class Vector3D
{
public:
	union {
		struct { float X, Y, Z; };
		struct { float R, G, B; };
		struct { float S, T, P; };
		struct { float Pitch, Roll, Yaw; };
	};

	Vector3D()
	{
		X = 0.0f;
		Y = 0.0f;
		Z = 0.0f;
	}

	Vector3D( const float& X, const float& Y, const float& Z )
	{
		this->X = X;
		this->Y = Y;
		this->Z = Z;
	}

	Vector3D( const float& A )
	{
		this->X = A;
		this->Y = A;
		this->Z = A;
	}

	const float* Base() const
	{
		return &X;
	}

	float& operator[]( size_t Index )
	{
		return ( &X )[Index];
	}

	const float& operator[]( size_t Index ) const
	{
		return ( &X )[Index];
	}

	bool operator<( const Vector3D& B ) const
	{
		return X < B.X && Y < B.Y && Z < B.Z;
	}

	Vector3D& operator=( const Vector3D& Vector )
	{
		this->X = Vector.X;
		this->Y = Vector.Y;
		this->Z = Vector.Z;
		return *this;
	}

	Vector3D operator+( const Vector3D& Vector ) const
	{
		return Vector3D(
			X + Vector.X,
			Y + Vector.Y,
			Z + Vector.Z
		);
	}

	Vector3D operator-( const Vector3D& Vector ) const
	{
		return Vector3D(
			X - Vector.X,
			Y - Vector.Y,
			Z - Vector.Z
		);
	}

	Vector3D operator*( const Vector3D& Vector ) const
	{
		return Vector3D(
			X * Vector.X,
			Y * Vector.Y,
			Z * Vector.Z
		);
	}

	Vector3D operator/( const Vector3D& Vector ) const
	{
		return Vector3D(
			X / Vector.X,
			Y / Vector.Y,
			Z / Vector.Z
		);
	}

	Vector3D operator+( const float& Scalar ) const
	{
		return Vector3D(
			X + Scalar,
			Y + Scalar,
			Z + Scalar
		);
	}

	Vector3D operator-( const float& Scalar ) const
	{
		return Vector3D(
			X - Scalar,
			Y - Scalar,
			Z - Scalar
		);
	}

	Vector3D operator*( const float& Scalar ) const
	{
		return Vector3D(
			X * Scalar,
			Y * Scalar,
			Z * Scalar
		);
	}

	friend Vector3D operator*( const float& Scalar, const Vector3D& Vector )
	{
		return Vector * Scalar;
	}

	Vector3D operator/( const float& Scalar ) const
	{
		return Vector3D(
			X / Scalar,
			Y / Scalar,
			Z / Scalar
		);
	}

	friend Vector3D operator/( const float& Scalar, const Vector3D& Vector )
	{
		return Vector3D(
			Scalar / Vector.X,
			Scalar / Vector.Y,
			Scalar / Vector.Z
		);
	}

	Vector3D operator+=( const Vector3D& Vector )
	{
		X += Vector.X;
		Y += Vector.Y;
		Z += Vector.Z;
		return *this;
	}

	Vector3D operator-=( const Vector3D& Vector )
	{
		X -= Vector.X;
		Y -= Vector.Y;
		Z -= Vector.Z;
		return *this;
	}

	Vector3D operator*=( const Vector3D& Vector )
	{
		X *= Vector.X;
		Y *= Vector.Y;
		Z *= Vector.Z;
		return *this;
	}

	Vector3D operator/=( const Vector3D& Vector )
	{
		X /= Vector.X;
		Y /= Vector.Y;
		Z /= Vector.Z;
		return *this;
	}

	Vector3D operator+=( const float& Scalar )
	{
		X += Scalar;
		Y += Scalar;
		Z += Scalar;
		return *this;
	}

	Vector3D operator-=( const float& Scalar )
	{
		X -= Scalar;
		Y -= Scalar;
		Z -= Scalar;
		return *this;
	}

	Vector3D operator*=( const float& Scalar )
	{
		X *= Scalar;
		Y *= Scalar;
		Z *= Scalar;
		return *this;
	}

	Vector3D operator/=( const float& Scalar )
	{
		X /= Scalar;
		Y /= Scalar;
		Z /= Scalar;
		return *this;
	}

	friend Vector3D operator-( const Vector3D& Vector )
	{
		Vector3D Result;
		Result.X = -Vector.X;
		Result.Y = -Vector.Y;
		Result.Z = -Vector.Z;
		return Result;
	}

	float Dot( const Vector3D& Vector ) const
	{
		return X * Vector.X + Y * Vector.Y + Z * Vector.Z;
	}

	Vector3D Cross( const Vector3D& Vector ) const
	{
		return Vector3D(
			Y * Vector.Z - Vector.Y * Z,
			Z * Vector.X - Vector.Z * X,
			X * Vector.Y - Vector.X * Y
		);
	}

	float Length() const
	{
		return sqrt( Dot( *this ) );
	}

	float LengthSquared() const
	{
		return Dot( *this );
	}

	float Length( const Vector3D& Vector ) const
	{
		return sqrt( Vector.Dot( Vector ) );
	}

	float Distance( const Vector3D& Vector ) const
	{
		return Length( *this - Vector );
	}

	float DistanceSquared( const Vector3D& Vector ) const
	{
		const Vector3D Delta = *this - Vector;
		return Delta.Dot( Delta );
	}

	Vector3D Normalized() const
	{
		Vector3D Unit = *this;

		const float VectorLength = Length( Unit );
		if( VectorLength > 0.0f )
		{
			Unit /= VectorLength;
		}

		return Unit;
	}

	float Normalize()
	{
		const float VectorLength = Length( *this );
		if( VectorLength > 0.0f )
		{
			*this /= VectorLength;
		}

		return VectorLength;
	}

	bool IsNaN() const
	{
		return isnan( X )
			|| isnan( Y )
			|| isnan( Z );
	}

	bool IsInfinite() const
	{
		return isinf( X )
			|| isinf( Y )
			|| isinf( Z );
	}

	bool IsValid() const
	{
		return !IsNaN() && !IsInfinite();
	}

	static const Vector3D Zero;
	static const Vector3D One;
};

class Vector4D
{
public:
	union {
		struct { float X, Y, Z, W; };
		struct { float R, G, B, A; };
		struct { float S, T, P, Q; };
	};

	Vector4D() = default;
	Vector4D( float X, float Y, float Z, float W )
	{
		this->X = X;
		this->Y = Y;
		this->Z = Z;
		this->W = W;
	}

	Vector4D( const Vector3D& Vector, float W )
	{
		this->X = Vector.X;
		this->Y = Vector.Y;
		this->Z = Vector.Z;
		this->W = W;
	}

	const float* Base() const
	{
		return &X;
	}

	float& operator[]( size_t Index )
	{
		return ( &X )[Index];
	}

	const float& operator[]( size_t Index ) const
	{
		return ( &X )[Index];
	}

	bool operator<( const Vector4D& B ) const
	{
		return X < B.X && Y < B.Y && Z < B.Z && W < B.W;
	}

	Vector4D& operator=( const Vector4D& Vector )
	{
		this->X = Vector.X;
		this->Y = Vector.Y;
		this->Z = Vector.Z;
		this->W = Vector.W;
		return *this;
	}

	Vector4D operator+( const Vector4D& Vector ) const
	{
		return Vector4D(
			X + Vector.X,
			Y + Vector.Y,
			Z + Vector.Z,
			W + Vector.W
		);
	}

	Vector4D operator-( const Vector4D& Vector ) const
	{
		return Vector4D(
			X - Vector.X,
			Y - Vector.Y,
			Z - Vector.Z,
			W - Vector.W
		);
	}

	Vector4D operator*( const Vector4D& Vector ) const
	{
		return Vector4D(
			X * Vector.X,
			Y * Vector.Y,
			Z * Vector.Z,
			W * Vector.W
		);
	}

	Vector4D operator/( const Vector4D& Vector ) const
	{
		return Vector4D(
			X / Vector.X,
			Y / Vector.Y,
			Z / Vector.Z,
			W / Vector.W
		);
	}

	Vector4D operator+( const float& Scalar ) const
	{
		return Vector4D(
			X + Scalar,
			Y + Scalar,
			Z + Scalar,
			W + Scalar
		);
	}

	Vector4D operator-( const float& Scalar ) const
	{
		return Vector4D(
			X - Scalar,
			Y - Scalar,
			Z - Scalar,
			W - Scalar
		);
	}

	Vector4D operator*( const float& Scalar ) const
	{
		return Vector4D(
			X * Scalar,
			Y * Scalar,
			Z * Scalar,
			W * Scalar
		);
	}

	Vector4D operator/( const float& Scalar ) const
	{
		return Vector4D(
			X / Scalar,
			Y / Scalar,
			Z / Scalar,
			W / Scalar
		);
	}

	Vector4D operator+=( const Vector4D& Vector )
	{
		X += Vector.X;
		Y += Vector.Y;
		Z += Vector.Z;
		W += Vector.W;
		return *this;
	}

	Vector4D operator-=( const Vector4D& Vector )
	{
		X -= Vector.X;
		Y -= Vector.Y;
		Z -= Vector.Z;
		W -= Vector.W;
		return *this;
	}

	Vector4D operator*=( const Vector4D& Vector )
	{
		X *= Vector.X;
		Y *= Vector.Y;
		Z *= Vector.Z;
		W *= Vector.W;
		return *this;
	}

	Vector4D operator/=( const Vector4D& Vector )
	{
		X /= Vector.X;
		Y /= Vector.Y;
		Z /= Vector.Z;
		W /= Vector.W;
		return *this;
	}

	Vector4D operator+=( const float& Scalar )
	{
		X += Scalar;
		Y += Scalar;
		Z += Scalar;
		W += Scalar;
		return *this;
	}

	Vector4D operator-=( const float& Scalar )
	{
		X -= Scalar;
		Y -= Scalar;
		Z -= Scalar;
		W -= Scalar;
		return *this;
	}

	Vector4D operator*=( const float& Scalar )
	{
		X *= Scalar;
		Y *= Scalar;
		Z *= Scalar;
		W *= Scalar;
		return *this;
	}

	Vector4D operator/=( const float& Scalar )
	{
		X /= Scalar;
		Y /= Scalar;
		Z /= Scalar;
		W /= Scalar;
		return *this;
	}

	float Dot( const Vector4D& Vector ) const
	{
		return X * Vector.X + Y * Vector.Y + Z * Vector.Z + W * Vector.W;
	}

	float Length() const
	{
		return sqrt( Dot( *this ) );
	}

	float Length( const Vector4D& Vector ) const
	{
		return sqrt( Vector.Dot( Vector ) );
	}

	float Distance( const Vector4D& Vector ) const
	{
		return Length( *this - Vector );
	}

	Vector4D Normalized() const
	{
		Vector4D Unit = *this;

		const float VectorLength = Length( Unit );
		if( VectorLength > 0.0f )
		{
			Unit /= VectorLength;
		}

		return Unit;
	}

	float Normalize()
	{
		const float LengthBiased = 1.f / ( Length() + FLT_EPSILON );
		*this *= LengthBiased;

		return LengthBiased;
	}

	float Normalize( const Vector4D& Vector, const float& Length )
	{
		const float LengthBiased = 1.f / ( Length + FLT_EPSILON );
		*this *= LengthBiased;

		return LengthBiased;
	}
};

namespace Intrinsic
{
	class Vector4D
	{
	public:
		Vector4D() = default;
		Vector4D( float X, float Y, float Z, float W )
		{
			StoredVector = _mm_setr_ps( X, Y, Z, W );
		}

		Vector4D( const ::Vector4D& Vector )
		{
			StoredVector = _mm_setr_ps( Vector.X, Vector.Y, Vector.Z, Vector.W );
		}

		__m128 Base() const
		{
			return StoredVector;
		}

		void operator=( const ::Vector4D& Vector )
		{
			StoredVector = _mm_setr_ps( Vector.X, Vector.Y, Vector.Z, Vector.W );
		}

		::Vector4D ToVector4D() const
		{
			::Vector4D Vector;
			_mm_storeu_ps( &Vector.X, StoredVector );
			return Vector;
		}

		::Vector3D ToVector3D() const
		{
			::Vector4D Vector4 = ToVector4D();
			::Vector3D Vector = ::Vector3D( Vector4.X, Vector4.Y, Vector4.Z );
			return Vector;
		}

		float Dot( const __m128& Vector ) const
		{
			return _mm_cvtss_f32( _mm_dp_ps( StoredVector, Vector, 0x71 ) );
		}

		float Length() const
		{
			return Length( StoredVector );
		}

		float LengthSquared() const
		{
			return Dot( StoredVector );
		}

		float Distance( const __m128& Vector ) const
		{
			return Length( Subtract( StoredVector, Vector ) );
		}

		__m128 operator+=( const __m128& Vector )
		{
			StoredVector = Add( StoredVector, Vector );
			return StoredVector;
		}

		__m128 operator-=( const __m128& Vector )
		{
			StoredVector = Subtract( StoredVector, Vector );
			return StoredVector;
		}

		__m128 operator*=( const __m128& Vector )
		{
			StoredVector = Multiply( StoredVector, Vector );
			return StoredVector;
		}

		static __m128 Add( const __m128& A, const __m128& B )
		{
			return _mm_add_ps( A, B );
		}

		static __m128 Subtract( const __m128& A, const __m128& B )
		{
			return _mm_sub_ps( A, B );
		}

		static __m128 Multiply( const __m128& A, const __m128& B )
		{
			return _mm_mul_ps( A, B );
		}

		static float Length( const __m128& Vector )
		{
			return _mm_cvtss_f32( _mm_sqrt_ss( _mm_dp_ps( Vector, Vector, 0x71 ) ) );
		}

	private:
		__m128 StoredVector;
	};
}
