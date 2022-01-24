// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <unordered_map>
#include <Engine/Utility/Math.h>
#include <ThirdParty/glm/glm.hpp>

struct Uniform
{
	enum
	{
		Component4 = 0,
		Component3,
		Component4x4,
		Unsigned,
		Signed,
		Signed4,
		Float,
		Undefined
	} Type;

	Vector3D Uniform3;
	Vector4D Uniform4;
	Matrix4D Uniform4x4;
	unsigned int UniformUnsigned;
	int UniformSigned;
	int32_t* UniformSigned4 = nullptr;
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
		UniformSigned = Value;
	}

	Uniform( int32_t* Value )
	{
		Type = Signed4;
		UniformSigned4 = Value;
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

		Uniform3 = B.Uniform3;
		Uniform4 = B.Uniform4;
		Uniform4x4 = B.Uniform4x4;
		UniformUnsigned = B.UniformUnsigned;
		UniformSigned = B.UniformSigned;
		UniformSigned4 = B.UniformSigned4;
		UniformFloat = B.UniformFloat;
		// LastProgram = B.LastProgram;
		// BufferLocation = B.BufferLocation;
		// Name = B.Name;

		return *this;
	}

	void Bind( const unsigned int& Program, const std::string& Location );
	void Bind( const unsigned int& Program );

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
		UniformSigned = Value;
	}

	void Set( int32_t* Value )
	{
		Type = Signed4;
		UniformSigned4 = Value;
	}

	void Set( const float& Value )
	{
		Type = Float;
		UniformFloat = Value;
	}
};

typedef std::unordered_map<std::string, Uniform> UniformMap;
