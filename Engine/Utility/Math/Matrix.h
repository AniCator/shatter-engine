// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math/Vector.h>

class Matrix3D
{
public:
	Matrix3D() = default;

	void Identity()
	{
		Columns[X] = { 1.0f, 0.0f ,0.0f };
		Columns[Y] = { 0.0f, 1.0f ,0.0f };
		Columns[Z] = { 0.0f, 0.0f ,1.0f };
	}

	Vector2D Rotate( const Vector2D& Vector ) const
	{
		Vector2D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.X;
		Result.Y = Columns[X].Y * Vector.Y + Columns[Y].Y * Vector.Y;
		return Result;
	}

	Vector2D Transform( const Vector2D& Vector ) const
	{
		Vector2D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.X + Columns[Z].X;
		Result.Y = Columns[X].Y * Vector.Y + Columns[Y].Y * Vector.Y + Columns[Z].Y;
		return Result;
	}

	Vector3D Transform( const Vector3D& Vector ) const
	{
		Vector3D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.X + Columns[Z].X * Vector.X;
		Result.Y = Columns[X].Y * Vector.Y + Columns[Y].Y * Vector.Y + Columns[Z].Y * Vector.Y;
		Result.Z = Columns[X].Z * Vector.Z + Columns[Y].Z * Vector.Z + Columns[Z].Z * Vector.Z;
		return Result;
	}

	Matrix3D Transpose() const
	{
		Matrix3D Transposed;
		Transposed[0][0] = Columns[0][0];
		Transposed[0][1] = Columns[1][0];
		Transposed[0][2] = Columns[2][0];

		Transposed[1][0] = Columns[0][1];
		Transposed[1][1] = Columns[1][1];
		Transposed[1][2] = Columns[2][1];

		Transposed[2][0] = Columns[0][2];
		Transposed[2][1] = Columns[1][2];
		Transposed[2][2] = Columns[2][2];

		return Transposed;
	}

	Matrix3D operator*( const Matrix3D& B ) const
	{
		Matrix3D Result;
		Result[X] = Columns[X] * B[X][X] + Columns[Y] * B[X][Y] + Columns[Z] * B[X][Z];
		Result[Y] = Columns[X] * B[Y][X] + Columns[Y] * B[Y][Y] + Columns[Z] * B[Y][Z];
		Result[Z] = Columns[X] * B[Z][X] + Columns[Y] * B[Z][Y] + Columns[Z] * B[Z][Z];
		return Result;
	}

	const Vector3D& operator[]( uint32_t Column ) const
	{
		return Columns[Column];
	}

	Vector3D& operator[]( uint32_t Column )
	{
		return Columns[Column];
	}

	// Takes in a normalized axis and the angle as radians.
	static Matrix3D FromAxisAngle( const Vector3D& Axis, const float& Angle )
	{
		// Axis angle to matrix. (compact edition)
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );
		const auto InvertedCosine = 1.0f - Cosine;

		Matrix3D RotationMatrix;
		RotationMatrix[X] = Vector3D( InvertedCosine * Axis.X * Axis.X + Cosine, InvertedCosine * Axis.X * Axis.Y - Axis.Z * Sine, InvertedCosine * Axis.X * Axis.Z + Axis.Y * Sine );
		RotationMatrix[Y] = Vector3D( InvertedCosine * Axis.X * Axis.Y + Axis.Z * Sine, InvertedCosine * Axis.Y * Axis.Y + Cosine, InvertedCosine * Axis.Y * Axis.Z - Axis.X * Sine );
		RotationMatrix[Z] = Vector3D( InvertedCosine * Axis.X * Axis.Z - Axis.Y * Sine, InvertedCosine * Axis.Y * Axis.Z + Axis.X * Sine, InvertedCosine * Axis.Z * Axis.Z + Cosine );

		return RotationMatrix;
	}

	// Rotation around the X-axis (angle is in radians, clockwise).
	static Matrix3D RotationX( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix3D RotationMatrix;
		RotationMatrix[X] = Vector3D( 1.0f, 0.0f, 0.0f );
		RotationMatrix[Y] = Vector3D( 0.0f, Cosine, Sine );
		RotationMatrix[Z] = Vector3D( 0.0f, -Sine, Cosine );

		return RotationMatrix;
	}

	// Rotation around the Y-axis (angle is in radians, clockwise).
	static Matrix3D RotationY( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix3D RotationMatrix;
		RotationMatrix[X] = Vector3D( Cosine, 0.0f, -Sine );
		RotationMatrix[Y] = Vector3D( 0.0f, 1.0f, 0.0f );
		RotationMatrix[Z] = Vector3D( Sine, 0.0f, Cosine );

		return RotationMatrix;
	}

	// Rotation around the Z-axis (angle is in radians, clockwise).
	static Matrix3D RotationZ( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto NegativeSine = -Sine;
		const auto Cosine = cosf( Angle );

		Matrix3D RotationMatrix;
		RotationMatrix[X] = Vector3D( Cosine, Sine, NegativeSine );
		RotationMatrix[Y] = Vector3D( NegativeSine, Cosine, 0.0f );
		RotationMatrix[Z] = Vector3D( 0.0f, 0.0f, 1.0f );

		return RotationMatrix;
	}

	// Rotation around the X-axis (angle is in radians, counter-clockwise).
	static Matrix3D RotationXCCW( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix3D RotationMatrix;
		RotationMatrix[X] = Vector3D( 1.0f, 0.0f, 0.0f );
		RotationMatrix[Y] = Vector3D( 0.0f, Cosine, -Sine );
		RotationMatrix[Z] = Vector3D( 0.0f, Sine, Cosine );

		return RotationMatrix;
	}

	// Rotation around the Y-axis (angle is in radians, counter-clockwise).
	static Matrix3D RotationYCCW( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix3D RotationMatrix;
		RotationMatrix[X] = Vector3D( Cosine, 0.0f, Sine );
		RotationMatrix[Y] = Vector3D( 0.0f, 1.0f, 0.0f );
		RotationMatrix[Z] = Vector3D( -Sine, 0.0f, Cosine );

		return RotationMatrix;
	}

	// Rotation around the Z-axis (angle is in radians, counter-clockwise).
	static Matrix3D RotationZCCW( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix3D RotationMatrix;
		RotationMatrix[X] = Vector3D( Cosine, -Sine, Sine );
		RotationMatrix[Y] = Vector3D( Sine, Cosine, 0.0f );
		RotationMatrix[Z] = Vector3D( 0.0f, 0.0f, 1.0f );

		return RotationMatrix;
	}

	// Returns a non-uniform scale matrix.
	static Matrix3D Scale( const Vector3D& Size )
	{
		Matrix3D ScaleMatrix;
		ScaleMatrix[X] = Vector3D( Size.X, 0.0f, 0.0f );
		ScaleMatrix[Y] = Vector3D( 0.0f, Size.Y, 0.0f );
		ScaleMatrix[Z] = Vector3D( 0.0f, 0.0f, Size.Z );

		return ScaleMatrix;
	}

	Vector3D Columns[3]
	{
		{ 1.0f, 0.0f ,0.0f },
		{ 0.0f, 1.0f ,0.0f },
		{ 0.0f, 0.0f ,1.0f }
	};

private:
	constexpr static uint32_t X = 0;
	constexpr static uint32_t Y = 1;
	constexpr static uint32_t Z = 2;
};

class Matrix4D
{
public:
	Matrix4D() = default;
	Matrix4D( const Vector3D& Offset )
	{
		Translate( Offset );
	}

	void Identity()
	{
		Columns[X] = { 1.0f, 0.0f ,0.0f, 0.0f };
		Columns[Y] = { 0.0f, 1.0f ,0.0f, 0.0f };
		Columns[Z] = { 0.0f, 0.0f ,1.0f, 0.0f };
		Columns[W] = { 0.0f, 0.0f ,0.0f, 1.0f };
	}

	void Translate( const Vector3D& Offset )
	{
		Columns[W] = Columns[X] * Offset.X + Columns[Y] * Offset.Y + Columns[Z] * Offset.Z + Columns[W];
	}

	// TODO: Come up with a better name that doesn't conflict with the static functions of Matrix4D.
	/*void Scale( const Vector3D& Factor )
	{
		Columns[X][X] *= Factor.X;
		Columns[Y][Y] *= Factor.Y;
		Columns[Z][Z] *= Factor.Z;
	}

	void Scale( const Vector4D& Factor )
	{
		Columns[X][X] *= Factor.X;
		Columns[Y][Y] *= Factor.Y;
		Columns[Z][Z] *= Factor.Z;
		Columns[W][W] *= Factor.W;
	}*/

	Vector3D Rotate( const Vector3D& Vector ) const
	{
		Vector3D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.Y + Columns[Z].X * Vector.Z;
		Result.Y = Columns[X].Y * Vector.X + Columns[Y].Y * Vector.Y + Columns[Z].Y * Vector.Z;
		Result.Z = Columns[X].Z * Vector.X + Columns[Y].Z * Vector.Y + Columns[Z].Z * Vector.Z;
		return Result;
	}

	Vector3D Transform( const Vector3D& Vector ) const
	{
		Vector3D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.Y + Columns[Z].X * Vector.Z + Columns[W].X;
		Result.Y = Columns[X].Y * Vector.X + Columns[Y].Y * Vector.Y + Columns[Z].Y * Vector.Z + Columns[W].Y;
		Result.Z = Columns[X].Z * Vector.X + Columns[Y].Z * Vector.Y + Columns[Z].Z * Vector.Z + Columns[W].Z;
		return Result;
	}

	Vector4D Transform( const Vector4D& Vector ) const
	{
		Vector4D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.Y + Columns[Z].X * Vector.Z + Columns[W].X * Vector.W;
		Result.Y = Columns[X].Y * Vector.X + Columns[Y].Y * Vector.Y + Columns[Z].Y * Vector.Z + Columns[W].Y * Vector.W;
		Result.Z = Columns[X].Z * Vector.X + Columns[Y].Z * Vector.Y + Columns[Z].Z * Vector.Z + Columns[W].Z * Vector.W;
		Result.W = Columns[X].W * Vector.X + Columns[Y].W * Vector.Y + Columns[Z].W * Vector.Z + Columns[W].W * Vector.W;
		return Result;
	}

	Matrix4D Transpose() const
	{
		Matrix4D Transposed;
		Transposed[0][0] = Columns[0][0];
		Transposed[0][1] = Columns[1][0];
		Transposed[0][2] = Columns[2][0];
		Transposed[0][3] = Columns[3][0];

		Transposed[1][0] = Columns[0][1];
		Transposed[1][1] = Columns[1][1];
		Transposed[1][2] = Columns[2][1];
		Transposed[1][3] = Columns[3][1];

		Transposed[2][0] = Columns[0][2];
		Transposed[2][1] = Columns[1][2];
		Transposed[2][2] = Columns[2][2];
		Transposed[2][3] = Columns[3][2];

		Transposed[3][0] = Columns[0][3];
		Transposed[3][1] = Columns[1][3];
		Transposed[3][2] = Columns[2][3];
		Transposed[3][3] = Columns[3][3];

		return Transposed;
	}

	Matrix3D To3D() const
	{
		Matrix3D Matrix;
		Matrix[X] = { Columns[X].X, Columns[X].Y ,Columns[X].Z };
		Matrix[Y] = { Columns[Y].X, Columns[Y].Y ,Columns[Y].Z };
		Matrix[Z] = { Columns[Z].X, Columns[Z].Y ,Columns[Z].Z };
		return Matrix;
	}

	Matrix4D operator*( const Matrix4D& B ) const
	{
		Matrix4D Result;
		Result[X] = Columns[X] * B[X][X] + Columns[Y] * B[X][Y] + Columns[Z] * B[X][Z] + Columns[W] * B[X][W];
		Result[Y] = Columns[X] * B[Y][X] + Columns[Y] * B[Y][Y] + Columns[Z] * B[Y][Z] + Columns[W] * B[Y][W];
		Result[Z] = Columns[X] * B[Z][X] + Columns[Y] * B[Z][Y] + Columns[Z] * B[Z][Z] + Columns[W] * B[Z][W];
		Result[W] = Columns[X] * B[W][X] + Columns[Y] * B[W][Y] + Columns[Z] * B[W][Z] + Columns[W] * B[W][W];
		return Result;
	}

	const Vector4D& operator[](uint32_t Column) const
	{
		return Columns[Column];
	}

	Vector4D& operator[]( uint32_t Column )
	{
		return Columns[Column];
	}

	Matrix4D& operator=( const Matrix3D& B )
	{
		Columns[X][X] = B[X][X];
		Columns[X][Y] = B[X][Y];
		Columns[X][Z] = B[X][Z];
		Columns[X][W] = 0.0f;

		Columns[Y][X] = B[X][X];
		Columns[Y][Y] = B[X][Y];
		Columns[Y][Z] = B[X][Z];
		Columns[Y][W] = 0.0f;

		Columns[Z][X] = B[X][X];
		Columns[Z][Y] = B[X][Y];
		Columns[Z][Z] = B[X][Z];
		Columns[Z][W] = 0.0f;

		Columns[W][X] = 0.0f;
		Columns[W][Y] = 0.0f;
		Columns[W][Z] = 0.0f;
		Columns[W][W] = 1.0f;
		
		return *this;
	}

	// Takes in a normalized axis and the angle as radians.
	static Matrix4D FromAxisAngle( const Vector3D& Axis, const float& Angle )
	{
		// Axis angle to matrix. (compact edition)
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );
		const auto InvertedCosine = 1.0f - Cosine;

		Matrix4D RotationMatrix;
		RotationMatrix[X] = Vector4D( InvertedCosine * Axis.X * Axis.X + Cosine, InvertedCosine * Axis.X * Axis.Y - Axis.Z * Sine, InvertedCosine * Axis.X * Axis.Z + Axis.Y * Sine, 0.0f );
		RotationMatrix[Y] = Vector4D( InvertedCosine * Axis.X * Axis.Y + Axis.Z * Sine, InvertedCosine * Axis.Y * Axis.Y + Cosine, InvertedCosine * Axis.Y * Axis.Z - Axis.X * Sine, 0.0f );
		RotationMatrix[Z] = Vector4D( InvertedCosine * Axis.X * Axis.Z - Axis.Y * Sine, InvertedCosine * Axis.Y * Axis.Z + Axis.X * Sine, InvertedCosine * Axis.Z * Axis.Z + Cosine, 0.0f );

		return RotationMatrix;
	}

	// Rotation around the X-axis (angle is in radians, clockwise).
	static Matrix4D RotationX( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix4D RotationMatrix;
		RotationMatrix[X] = Vector4D( 1.0f, 0.0f, 0.0f, 0.0f );
		RotationMatrix[Y] = Vector4D( 0.0f, Cosine, Sine, 0.0f );
		RotationMatrix[Z] = Vector4D( 0.0f, -Sine, Cosine, 0.0f );
		
		return RotationMatrix;
	}

	// Rotation around the Y-axis (angle is in radians, clockwise).
	static Matrix4D RotationY( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix4D RotationMatrix;
		RotationMatrix[X] = Vector4D( Cosine, 0.0f, -Sine, 0.0f );
		RotationMatrix[Y] = Vector4D( 0.0f, 1.0f, 0.0f, 0.0f );
		RotationMatrix[Z] = Vector4D( Sine, 0.0f, Cosine, 0.0f );

		return RotationMatrix;
	}

	// Rotation around the Z-axis (angle is in radians, clockwise).
	static Matrix4D RotationZ( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto NegativeSine = -Sine;
		const auto Cosine = cosf( Angle );

		Matrix4D RotationMatrix;
		RotationMatrix[X] = Vector4D( Cosine, Sine, NegativeSine, 0.0f );
		RotationMatrix[Y] = Vector4D( NegativeSine, Cosine, 0.0f, 0.0f );
		RotationMatrix[Z] = Vector4D( 0.0f, 0.0f, 1.0f, 0.0f );

		return RotationMatrix;
	}

	// Rotation around the X-axis (angle is in radians, counter-clockwise).
	static Matrix4D RotationXCCW( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix4D RotationMatrix;
		RotationMatrix[X] = Vector4D( 1.0f, 0.0f, 0.0f, 0.0f );
		RotationMatrix[Y] = Vector4D( 0.0f, Cosine, -Sine, 0.0f );
		RotationMatrix[Z] = Vector4D( 0.0f, Sine, Cosine, 0.0f );

		return RotationMatrix;
	}

	// Rotation around the Y-axis (angle is in radians, counter-clockwise).
	static Matrix4D RotationYCCW( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix4D RotationMatrix;
		RotationMatrix[X] = Vector4D( Cosine, 0.0f, Sine, 0.0f );
		RotationMatrix[Y] = Vector4D( 0.0f, 1.0f, 0.0f, 0.0f );
		RotationMatrix[Z] = Vector4D( -Sine, 0.0f, Cosine, 0.0f );

		return RotationMatrix;
	}

	// Rotation around the Z-axis (angle is in radians, counter-clockwise).
	static Matrix4D RotationZCCW( const float& Angle )
	{
		const auto Sine = sinf( Angle );
		const auto Cosine = cosf( Angle );

		Matrix4D RotationMatrix;
		RotationMatrix[X] = Vector4D( Cosine, -Sine, Sine, 0.0f );
		RotationMatrix[Y] = Vector4D( Sine, Cosine, 0.0f, 0.0f );
		RotationMatrix[Z] = Vector4D( 0.0f, 0.0f, 1.0f, 0.0f );

		return RotationMatrix;
	}

	// Returns a non-uniform scale matrix.
	static Matrix4D Scale( const Vector3D& Size )
	{
		Matrix4D ScaleMatrix;
		ScaleMatrix[X] = Vector4D( Size.X, 0.0f, 0.0f, 0.0f );
		ScaleMatrix[Y] = Vector4D( 0.0f, Size.Y, 0.0f, 0.0f );
		ScaleMatrix[Z] = Vector4D( 0.0f, 0.0f, Size.Z, 0.0f );

		return ScaleMatrix;
	}

	// Returns an offset matrix that can be used to translate matrices.
	static Matrix4D Translation( const Vector3D& Offset )
	{
		Matrix4D TranslationMatrix;
		TranslationMatrix[X] = Vector4D( 1.0f, 0.0f, 0.0f, 0.0f );
		TranslationMatrix[Y] = Vector4D( 0.0f, 1.0f, 0.0f, 0.0f );
		TranslationMatrix[Z] = Vector4D( 0.0f, 0.0f, 1.0f, 0.0f );
		TranslationMatrix[W] = Vector4D( Offset.X, Offset.Y, Offset.Z, 1.0f );

		return TranslationMatrix;
	}

	Vector4D Columns[4]
	{
		{ 1.0f, 0.0f ,0.0f, 0.0f },
		{ 0.0f, 1.0f ,0.0f, 0.0f },
		{ 0.0f, 0.0f ,1.0f, 0.0f },
		{ 0.0f, 0.0f ,0.0f, 1.0f }
	};

private:
	constexpr static uint32_t X = 0;
	constexpr static uint32_t Y = 1;
	constexpr static uint32_t Z = 2;
	constexpr static uint32_t W = 3;
};
