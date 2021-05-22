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
		Unsigned
	} Type;

	Vector3D Uniform3;
	Vector4D Uniform4;
	Matrix4D Uniform4x4;
	unsigned int UniformUnsigned;

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

	void Bind( const unsigned int& Program, const std::string& Location ) const;
};

typedef std::unordered_map<std::string, Uniform> UniformMap;
