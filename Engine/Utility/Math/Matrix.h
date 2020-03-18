// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math/Vector.h>

class Matrix3D
{
public:
	Matrix3D()
	{
		Identity();
	}

	Matrix3D( const Vector2D& Translation )
	{
		Identity();
		Translate( Translation );
	}

	Matrix3D( const Vector3D& Translation )
	{
		Identity();
		Translate( Translation );
	}

	inline void Identity()
	{
		Columns[X] = { 1.0f, 0.0f ,0.0f };
		Columns[Y] = { 0.0f, 1.0f ,0.0f };
		Columns[Z] = { 0.0f, 0.0f ,1.0f };
	}

	inline void Translate( const Vector2D& Translation )
	{
		Columns[Z] = Columns[X] * Translation.X + Columns[Y] * Translation.Y + Columns[Z];
	}

	inline void Translate( const Vector3D& Translation )
	{
		Columns[Z] = Columns[X] * Translation.X + Columns[Y] * Translation.Y + Columns[Z];
	}

	inline Vector2D Rotate( const Vector2D& Vector )
	{
		Vector2D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.X;
		Result.Y = Columns[X].Y * Vector.Y + Columns[Y].Y * Vector.Y;
		return Result;
	}

	inline Vector2D Transform( const Vector2D& Vector )
	{
		Vector2D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.X + Columns[Z].X;
		Result.Y = Columns[X].Y * Vector.Y + Columns[Y].Y * Vector.Y + Columns[Z].Y;
		return Result;
	}

	inline Vector3D Transform( const Vector3D& Vector )
	{
		Vector3D Result;
		Result.X = Columns[X].X * Vector.X + Columns[Y].X * Vector.X + Columns[Z].X * Vector.X;
		Result.Y = Columns[X].Y * Vector.Y + Columns[Y].Y * Vector.Y + Columns[Z].Y * Vector.Y;
		Result.Z = Columns[X].Z * Vector.Z + Columns[Y].Z * Vector.Z + Columns[Z].Z * Vector.Z;
		return Result;
	}

	inline const Vector3D& operator[]( uint32_t Column ) const
	{
		return Columns[Column];
	}

	inline Vector3D& operator[]( uint32_t Column )
	{
		return Columns[Column];
	}

	Vector3D Columns[3];

private:
	const static uint32_t X = 0;
	const static uint32_t Y = 1;
	const static uint32_t Z = 2;
};

class Matrix4D
{
public:
	Matrix4D()
	{
		Identity();
	}

	Matrix4D( const Vector3D& Offset )
	{
		Identity();
		Translate( Offset );
	}

	inline void Identity()
	{
		Columns[X] = { 1.0f, 0.0f ,0.0f, 0.0f };
		Columns[Y] = { 0.0f, 1.0f ,0.0f, 0.0f };
		Columns[Z] = { 0.0f, 0.0f ,1.0f, 0.0f };
		Columns[W] = { 0.0f, 0.0f ,0.0f, 1.0f };
	}

	inline void Translate( const Vector3D& Offset )
	{
		Columns[W] = Columns[X] * Offset.X + Columns[Y] * Offset.Y + Columns[Z] * Offset.Z + Columns[W];
	}

	inline void Scale( const Vector3D& Factor )
	{
		Columns[X][X] *= Factor.X;
		Columns[Y][Y] *= Factor.Y;
		Columns[Z][Z] *= Factor.Z;
	}

	inline void Scale( const Vector4D& Factor )
	{
		Columns[X][X] *= Factor.X;
		Columns[Y][Y] *= Factor.Y;
		Columns[Z][Z] *= Factor.Z;
		Columns[W][W] *= Factor.W;
	}

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

	inline Matrix3D To3D() const
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

	inline const Vector4D& operator[](uint32_t Column) const
	{
		return Columns[Column];
	}

	inline Vector4D& operator[]( uint32_t Column )
	{
		return Columns[Column];
	}

	Vector4D Columns[4];

private:
	const static uint32_t X = 0;
	const static uint32_t Y = 1;
	const static uint32_t Z = 2;
	const static uint32_t W = 3;
};
