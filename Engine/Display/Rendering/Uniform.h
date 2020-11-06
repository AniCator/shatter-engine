// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <unordered_map>
#include <Engine/Utility/Math.h>
#include <ThirdParty/glm/glm.hpp>

struct FUniform
{
	enum
	{
		Component4 = 0,
		Component3,
		Component4x4
	} Type;

	Vector3D Uniform3;
	Vector4D Uniform4;
	Matrix4D Uniform4x4;

	FUniform( const Vector4D& Vector )
	{
		Type = Component4;
		Uniform4 = Vector;
	}

	FUniform( const Vector3D& Vector )
	{
		Type = Component3;
		Uniform3 = Vector;
	}

	FUniform( const Matrix4D& Matrix )
	{
		Type = Component4x4;
		Uniform4x4 = Matrix;
	}
};

typedef std::unordered_map<std::string, FUniform> UniformMap;
