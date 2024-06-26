// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <unordered_map>
#include <Engine/Utility/Math.h>

struct Uniform
{
	enum
	{
		Component4 = 0,
		Component3,
		Component2,
		Component4x4,
		Unsigned,
		Signed,
		Signed4,
		Float,
		Undefined
	} Type;

	Vector2D Uniform2;
	Vector3D Uniform3;
	Vector4D Uniform4;
	Matrix4D Uniform4x4;
	unsigned int UniformUnsigned;
	int32_t UniformSigned4[4];
	float UniformFloat;
	unsigned int LastProgram = 0;
	int BufferLocation = -1;
	std::string Name;

	Uniform()
	{
		Type = Undefined;
	}

	Uniform( const std::string& Name )
	{
		Type = Undefined;
		this->Name = Name;
	}

	Uniform( const Vector4D& Vector )
	{
		Type = Component4;
		Uniform4 = Vector;
	}

	Uniform( const Vector3D& Vector )
	{
		Type = Component3;
		Uniform3 = Vector;
	}

	Uniform( const Vector2D& Vector )
	{
		Type = Component2;
		Uniform2 = Vector;
	}

	Uniform( const Matrix4D& Matrix )
	{
		Type = Component4x4;
		Uniform4x4 = Matrix;
	}

	Uniform( const unsigned int& Value )
	{
		Type = Unsigned;
		UniformUnsigned = Value;
	}

	Uniform( const int& Value )
	{
		Type = Signed;
		UniformSigned4[0] = Value;
	}

	Uniform( const int32_t* Value )
	{
		Type = Signed4;
		UniformSigned4[0] = Value[0];
		UniformSigned4[1] = Value[1];
		UniformSigned4[2] = Value[2];
		UniformSigned4[3] = Value[3];
	}

	Uniform( const float& Value )
	{
		Type = Float;
		UniformFloat = Value;
	}

	Uniform& operator=( const Uniform& B )
	{
		if( this == &B )
			return *this;

		Uniform2 = B.Uniform2;
		Uniform3 = B.Uniform3;
		Uniform4 = B.Uniform4;
		Uniform4x4 = B.Uniform4x4;
		UniformUnsigned = B.UniformUnsigned;
		UniformSigned4[0] = B.UniformSigned4[0];
		UniformSigned4[1] = B.UniformSigned4[1];
		UniformSigned4[2] = B.UniformSigned4[2];
		UniformSigned4[3] = B.UniformSigned4[3];
		UniformFloat = B.UniformFloat;

		LastProgram = 0;
		BufferLocation = -1;
		Type = B.Type;

		return *this;
	}

	void Bind( const unsigned int& Program, const std::string& Location );
	void Bind( const unsigned int& Program );
	void Reset();

	void Set( const Vector4D& Vector )
	{
		Type = Component4;
		Uniform4 = Vector;
	}

	void Set( const Vector3D& Vector )
	{
		Type = Component3;
		Uniform3 = Vector;
	}

	void Set( const Vector2D& Vector )
	{
		Type = Component2;
		Uniform2 = Vector;
	}

	void Set( const Matrix4D& Matrix )
	{
		Type = Component4x4;
		Uniform4x4 = Matrix;
	}

	void Set( const unsigned int& Value )
	{
		Type = Unsigned;
		UniformUnsigned = Value;
	}

	void Set( const int& Value )
	{
		Type = Signed;
		UniformSigned4[0] = Value;
	}

	void Set( const int32_t* Value )
	{
		Type = Signed4;

		UniformSigned4[0] = Value[0];
		UniformSigned4[1] = Value[1];
		UniformSigned4[2] = Value[2];
		UniformSigned4[3] = Value[3];
	}

	void Set( const float& Value )
	{
		Type = Float;
		UniformFloat = Value;
	}
};

typedef std::unordered_map<std::string, Uniform> UniformMap;
